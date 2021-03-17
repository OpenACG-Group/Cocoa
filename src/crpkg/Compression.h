#ifndef COCOA_COMPRESSION_H
#define COCOA_COMPRESSION_H

#include <vector>
#include <string>
#include <functional>
#include <fstream>
#include <memory>

#include "zstd.h"

#include "crpkg/crpkg.h"
CRPKG_BEGIN_NS

#define CRPKG_COMPRESS_READ_ONCE    4096
class CompressContext : public std::enable_shared_from_this<CompressContext>
{
public:
    class PromiseResult
    {
    public:
        PromiseResult();
        PromiseResult(bool memoryCache, std::shared_ptr<CompressContext> ctx, std::ifstream stream);
        PromiseResult(const PromiseResult&) = delete;
        PromiseResult(PromiseResult&& rhs) noexcept;
        ~PromiseResult();

        PromiseResult& operator=(const PromiseResult&) = delete;
        PromiseResult& operator=(PromiseResult&& rhs) noexcept;

        void write(const std::function<void(const void *, size_t)>& writer);
        size_t compressedSize();
        void freeCaches();

    private:
        void prepareDiskCache();
        void prepareMemoryCache();
        void dropDiskCache();
        void dropMemoryCache();
        void diskCacheWriter(const void *data, size_t size);
        void memoryCacheWriter(const void *data, size_t size);
        void compressToCache();
        void compress(const std::function<void(const void*, size_t)>& writer);

        std::ifstream       fStream;
        std::shared_ptr<CompressContext>
                            fContext;
        uint8_t            *fInputChunkBuffer;
        uint8_t            *fOutputChunkBuffer;

        bool                fIsCached = false;
        bool                fIsMemoryCached = false;
        std::unique_ptr<uint8_t>
                            fMemoryCacheBuffer = nullptr;
        size_t              fMemoryCacheBufferPos = 0;
        std::string         fDiskCacheFile;
        std::fstream        fDiskCacheStream;
    };

    static std::shared_ptr<CompressContext> Make();

    CompressContext();
    ~CompressContext();

    PromiseResult compress(std::ifstream stream);

    inline ZSTD_CCtx *context() const
    {
        return fCCtx;
    }

    inline size_t inputChunkSize() const
    {
        return fInputChunkSize;
    }

    inline size_t outputChunkSize() const
    {
        return fOutputChunkSize;
    }

private:
    ZSTD_CCtx           *fCCtx;
    size_t               fInputChunkSize;
    size_t               fOutputChunkSize;
};

CRPKG_END_NS
#endif //COCOA_COMPRESSION_H
