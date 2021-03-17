#include <unistd.h>
#include <iostream>
#include <thread>
#include <cstring>

#include "zstd.h"

#include "Core/Exception.h"
#include "crpkg/crpkg.h"
#include "crpkg/Compression.h"
CRPKG_BEGIN_NS

std::shared_ptr<CompressContext> CompressContext::Make()
{
    return std::make_shared<CompressContext>();
}

CompressContext::CompressContext()
    : fCCtx(nullptr),
      fInputChunkSize(0),
      fOutputChunkSize(0)
{
    fCCtx = ZSTD_createCCtx();
    if (fCCtx == nullptr)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Failed to create ZSTD context for compression")
                .make<RuntimeException>();
    }
    fInputChunkSize = ZSTD_CStreamInSize();
    fOutputChunkSize = ZSTD_CStreamOutSize();

    ZSTD_CCtx_setParameter(fCCtx, ZSTD_c_compressionLevel, 10);
    ZSTD_CCtx_setParameter(fCCtx, ZSTD_c_checksumFlag, 0);
    ZSTD_CCtx_setParameter(fCCtx, ZSTD_c_nbWorkers, static_cast<int>(std::thread::hardware_concurrency()));
}

CompressContext::~CompressContext()
{
    if (fCCtx != nullptr)
        ZSTD_freeCCtx(fCCtx);
}

CompressContext::PromiseResult CompressContext::compress(std::ifstream stream)
{
    return PromiseResult(false, shared_from_this(), std::move(stream));
}

// -------------------------------------------------------------------------------

CompressContext::PromiseResult::PromiseResult()
    : fContext(nullptr),
      fInputChunkBuffer(nullptr),
      fOutputChunkBuffer(nullptr)
{
}

CompressContext::PromiseResult::PromiseResult(bool memoryCache,
                                              std::shared_ptr<CompressContext> ctx, std::ifstream stream)
    : fStream(std::move(stream)),
      fContext(std::move(ctx)),
      fInputChunkBuffer(nullptr),
      fOutputChunkBuffer(nullptr),
      fIsMemoryCached(memoryCache)
{
    fInputChunkBuffer = static_cast<uint8_t*>(std::malloc(fContext->inputChunkSize()));
    if (fInputChunkBuffer == nullptr)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("crpkg: Failed to compress data, out of memory")
                .make<RuntimeException>();
    }

    fOutputChunkBuffer = static_cast<uint8_t*>(std::malloc(fContext->outputChunkSize()));
    if (fOutputChunkBuffer == nullptr)
    {
        std::free(fInputChunkBuffer);
        throw RuntimeException::Builder(__FUNCTION__)
                .append("crpkg: Failed to compress data, out of memory")
                .make<RuntimeException>();
    }
}

CompressContext::PromiseResult::PromiseResult(PromiseResult&& rhs) noexcept
    : fStream(std::move(rhs.fStream)),
      fContext(std::move(rhs.fContext)),
      fInputChunkBuffer(rhs.fInputChunkBuffer),
      fOutputChunkBuffer(rhs.fOutputChunkBuffer),
      fIsCached(rhs.fIsCached),
      fIsMemoryCached(rhs.fIsMemoryCached),
      fMemoryCacheBuffer(std::move(rhs.fMemoryCacheBuffer)),
      fMemoryCacheBufferPos(rhs.fMemoryCacheBufferPos),
      fDiskCacheFile(std::move(rhs.fDiskCacheFile)),
      fDiskCacheStream(std::move(rhs.fDiskCacheStream))
{
    rhs.fInputChunkBuffer = nullptr;
    rhs.fOutputChunkBuffer = nullptr;
    rhs.fIsCached = false;
    rhs.fMemoryCacheBufferPos = 0;
}

CompressContext::PromiseResult & CompressContext::PromiseResult::operator=(PromiseResult&& rhs) noexcept
{
    fStream = std::move(rhs.fStream);
    fContext = std::move(rhs.fContext);
    fInputChunkBuffer = rhs.fInputChunkBuffer;
    fOutputChunkBuffer = rhs.fOutputChunkBuffer;
    fIsCached = rhs.fIsCached;
    fIsMemoryCached = rhs.fIsMemoryCached;
    fMemoryCacheBuffer = std::move(rhs.fMemoryCacheBuffer);
    fMemoryCacheBufferPos = rhs.fMemoryCacheBufferPos;
    fDiskCacheFile = std::move(rhs.fDiskCacheFile);
    fDiskCacheStream = std::move(rhs.fDiskCacheStream);
    rhs.fInputChunkBuffer = nullptr;
    rhs.fOutputChunkBuffer = nullptr;
    rhs.fIsCached = false;
    rhs.fMemoryCacheBufferPos = 0;

    return *this;
}

CompressContext::PromiseResult::~PromiseResult()
{
    if (fInputChunkBuffer)
        std::free(fInputChunkBuffer);
    if (fOutputChunkBuffer)
        std::free(fOutputChunkBuffer);
    freeCaches();
}

void CompressContext::PromiseResult::prepareDiskCache()
{
    std::ostringstream oss;
    oss << this << '@' << getpid() << ".crpkg.cache";
    fDiskCacheFile = oss.str();

    fDiskCacheStream.open(fDiskCacheFile, std::ios::binary | std::ios::trunc | std::ios::in | std::ios::out);
    if (!fDiskCacheStream.is_open())
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("crpkg: Failed to create disk cache")
                .make<RuntimeException>();
    }

    fIsCached = true;
}

void CompressContext::PromiseResult::prepareMemoryCache()
{
    fStream.seekg(0, std::ios::seekdir::_S_end);
    size_t size = ZSTD_compressBound(fStream.tellg());
    fStream.seekg(0);

    fMemoryCacheBuffer.reset(static_cast<uint8_t*>(std::malloc(size)));
    fIsCached = true;
}

void CompressContext::PromiseResult::dropDiskCache()
{
    if (!fIsCached || fIsMemoryCached)
        return;

    fDiskCacheStream.close();
    remove(fDiskCacheFile.c_str());
    fIsCached = false;
}

void CompressContext::PromiseResult::dropMemoryCache()
{
    if (!fIsCached || !fIsMemoryCached)
        return;

    fMemoryCacheBuffer.reset(nullptr);
    fIsCached = false;
}

void CompressContext::PromiseResult::diskCacheWriter(const void *data, size_t size)
{
    fDiskCacheStream.write(static_cast<const char*>(data), size);
}

void CompressContext::PromiseResult::memoryCacheWriter(const void *data, size_t size)
{
    std::memcpy(fMemoryCacheBuffer.get() + fMemoryCacheBufferPos, data, size);
    fMemoryCacheBufferPos += size;
}

void CompressContext::PromiseResult::compressToCache()
{
    if (!fIsCached)
        return;

    if (fIsMemoryCached)
    {
        this->compress([this](const void *data, size_t size) -> void {
            this->memoryCacheWriter(data, size);
        });
    }
    else
    {
        fDiskCacheStream.seekp(0);
        this->compress([this](const void *data, size_t size) -> void {
            this->diskCacheWriter(data, size);
        });
        fDiskCacheStream.close();
        fDiskCacheStream.open(fDiskCacheFile, std::ios::binary | std::ios::trunc | std::ios::in | std::ios::out);
    }
}

void CompressContext::PromiseResult::compress(const std::function<void(const void *, size_t)>& writer)
{
    while (true)
    {
        size_t actualRead = fStream.readsome(reinterpret_cast<char *>(fInputChunkBuffer),
                                             fContext->inputChunkSize());
        bool lastChunk = (actualRead < fContext->inputChunkSize());
        const ZSTD_EndDirective mode = lastChunk ? ZSTD_e_end : ZSTD_e_continue;

        bool finished;
        ZSTD_inBuffer inBuffer{fInputChunkBuffer, actualRead, 0};
        do
        {
            ZSTD_outBuffer outBuffer{fOutputChunkBuffer, fContext->outputChunkSize(), 0};
            const size_t remaining = ZSTD_compressStream2(fContext->context(),
                                                          &outBuffer,
                                                          &inBuffer,
                                                          mode);
            writer(outBuffer.dst, outBuffer.pos);

            finished = lastChunk ? !remaining : (inBuffer.pos == inBuffer.size);
        } while (!finished);

        if (lastChunk)
            break;
    }
}

void CompressContext::PromiseResult::write(const std::function<void(const void *, size_t)>& writer)
{
    if (!fIsCached)
    {
        this->compress(writer);
        return;
    }

    if (fIsMemoryCached)
    {
        writer(fMemoryCacheBuffer.get(), fMemoryCacheBufferPos);
    }
    else
    {
        std::unique_ptr<uint8_t> buffer(new uint8_t[CRPKG_COMPRESS_READ_ONCE]);
        size_t actualRead = 0;
        fDiskCacheStream.seekg(0);
        while ((actualRead = fDiskCacheStream.readsome(reinterpret_cast<char*>(buffer.get()),
                                                       CRPKG_COMPRESS_READ_ONCE)) > 0)
        {
            writer(buffer.get(), actualRead);
        }
        fDiskCacheStream.seekg(0);
    }
}

void CompressContext::PromiseResult::freeCaches()
{
    if (!fIsCached)
        return;

    if (fIsMemoryCached)
        dropMemoryCache();
    else
        dropDiskCache();
}

size_t CompressContext::PromiseResult::compressedSize()
{
    if (fIsCached)
    {
        if (fIsMemoryCached)
            return fMemoryCacheBufferPos;
        else
            return fDiskCacheStream.tellp();
    }

    if (fIsMemoryCached)
        prepareMemoryCache();
    else
        prepareDiskCache();
    compressToCache();

    return compressedSize();
}

CRPKG_END_NS
