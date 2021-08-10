#include <atomic>

#ifdef __linux__
#include <sys/stat.h>
#endif /* __linux__ */

extern "C" {
#include "squash.h"
}

#include "Core/Project.h"
#include "Core/CrpkgImage.h"
#include "Core/Journal.h"
#include "Core/Filesystem.h"

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Core)

namespace cocoa {

namespace {

std::atomic<bool> gInitializedSquash(false);

} // namespace anonymous

struct CrpkgImage::CrpkgImagePrivate
{
    uint8_t     *imageMapped;
    size_t       imageSize;
    sqfs        *squash;
};

std::shared_ptr<CrpkgImage> CrpkgImage::Make(const std::string& file)
{
    if (!gInitializedSquash)
    {
        squash_start();
        gInitializedSquash = true;
    }

    if (vfs::Access(file, {vfs::AccessMode::kReadable}) == vfs::AccessResult::kFailed)
        return nullptr;

    int fd = vfs::Open(file, {vfs::OpenFlags::kReadonly});
    if (fd < 0)
    {
        LOGF(LOG_ERROR, "Failed in loading crpkg image {}: {}", file, strerror(errno))
        return nullptr;
    }

    auto *pData = new CrpkgImagePrivate;
    pData->squash = reinterpret_cast<struct sqfs*>(std::malloc(sizeof(struct sqfs)));
    pData->imageSize = vfs::FileSize(fd);
    /**
     * The image file may be huge, which is expensive to read it into a allocated buffer.
     * Mapping it directly is very fast and will not allocate from heap.
     * However, that maybe makes performance problem while we're reading data.
     */
    pData->imageMapped = reinterpret_cast<uint8_t*>(vfs::MemMap(fd,
                                                                nullptr,
                                                                {vfs::MapProtection::kRead},
                                                                {vfs::MapFlags::kPrivate},
                                                                pData->imageSize,
                                                                0));

    if (pData->imageMapped == nullptr)
    {
        LOGF(LOG_ERROR, "Failed in mapping crpkg image {}: {}", file, strerror(errno));
        return nullptr;
    }

    sqfs_err ret = sqfs_init(pData->squash, pData->imageMapped, 0);
    switch (ret)
    {
    case SQFS_OK:
        break;
    case SQFS_BADFORMAT:
        LOGF(LOG_ERROR, "{} is not a valid crpkg image", file)
        break;
    case SQFS_BADVERSION:
    {
        int major, minor;
        sqfs_version(pData->squash, &major, &minor);
        LOGF(LOG_ERROR, "crpkg (SquashFS) version {}.{} detected in {}, which is not supported",
             major, minor, file);
        break;
    }
    case SQFS_BADCOMP:
        LOGF(LOG_ERROR, "{} uses unknown compression algorithm", file)
        break;
    default:
        LOGF(LOG_ERROR, "Couldn't load crpkg image {}", file)
        break;
    }
    if (ret != SQFS_OK)
        return nullptr;

    return std::make_shared<CrpkgImage>(file, pData);
}

CrpkgImage::CrpkgImage(std::string imageFile, CrpkgImagePrivate *data)
    : fImageFile(imageFile),
      fData(data)
{
}

CrpkgImage::~CrpkgImage()
{
    if (fData->squash)
    {
        sqfs_destroy(fData->squash);
        std::free(fData->squash);
    }
    if (fData->imageMapped)
        vfs::MemUnmap(fData->imageMapped, fData->imageSize);
    delete fData;
}

std::shared_ptr<CrpkgFile> CrpkgImage::openFile(const std::string& path)
{
    int32_t vfd = squash_open(fData->squash, path.c_str());
    if (vfd < 0)
        return nullptr;
    return std::make_shared<CrpkgFile>(vfd, shared_from_this());
}

namespace {
thread_local char gTlsPathBuffer[PATH_MAX];
} // namespace anonymous
std::optional<std::string> CrpkgImage::readlink(const std::string& path)
{
    if (squash_readlink(fData->squash, path.c_str(), gTlsPathBuffer, PATH_MAX) < 0)
        return std::optional<std::string>();
    return std::make_optional<std::string>(gTlsPathBuffer);
}

CrpkgFile::CrpkgFile(int32_t vfd, std::shared_ptr<CrpkgImage> image)
    : fFile(vfd),
      fImage(std::move(image))
{
}

CrpkgFile::~CrpkgFile()
{
    squash_close(fFile);
}

ssize_t CrpkgFile::read(void *buffer, ssize_t size) const
{
    return squash_read(fFile, buffer, size);
}

std::optional<vfs::Stat> CrpkgFile::stat()
{
#ifdef __linux__
    struct stat stbuf;  // NOLINT
    if (squash_fstat(fFile, &stbuf) < 0)
        return std::optional<vfs::Stat>();

    vfs::Bitfield<vfs::Mode> mode;
    mode |= (stbuf.st_mode & S_IRUSR) ? vfs::Mode::kUsrR : vfs::Mode::kNone;
    mode |= (stbuf.st_mode & S_IWUSR) ? vfs::Mode::kUsrW : vfs::Mode::kNone;
    mode |= (stbuf.st_mode & S_IXUSR) ? vfs::Mode::kUsrX : vfs::Mode::kNone;
    mode |= (stbuf.st_mode & S_IRGRP) ? vfs::Mode::kGrpR : vfs::Mode::kNone;
    mode |= (stbuf.st_mode & S_IWGRP) ? vfs::Mode::kGrpW : vfs::Mode::kNone;
    mode |= (stbuf.st_mode & S_IXGRP) ? vfs::Mode::kGrpX : vfs::Mode::kNone;
    mode |= (stbuf.st_mode & S_IROTH) ? vfs::Mode::kOthR : vfs::Mode::kNone;
    mode |= (stbuf.st_mode & S_IWOTH) ? vfs::Mode::kOthW : vfs::Mode::kNone;
    mode |= (stbuf.st_mode & S_IXOTH) ? vfs::Mode::kOthX : vfs::Mode::kNone;
    mode |= S_ISDIR(stbuf.st_mode) ? vfs::Mode::kDir : vfs::Mode::kNone;
    mode |= S_ISREG(stbuf.st_mode) ? vfs::Mode::kRegular : vfs::Mode::kNone;
    mode |= S_ISLNK(stbuf.st_mode) ? vfs::Mode::kLink : vfs::Mode::kNone;
    mode |= S_ISCHR(stbuf.st_mode) ? vfs::Mode::kChar : vfs::Mode::kNone;
    mode |= S_ISBLK(stbuf.st_mode) ? vfs::Mode::kBlock : vfs::Mode::kNone;
    mode |= S_ISFIFO(stbuf.st_mode) ? vfs::Mode::kFifo : vfs::Mode::kNone;
    mode |= S_ISSOCK(stbuf.st_mode) ? vfs::Mode::kSocket : vfs::Mode::kNone;

    vfs::Stat result{};
    result.linkCount = stbuf.st_nlink;
    result.size = stbuf.st_size;
    result.uid = stbuf.st_uid;
    result.gid = stbuf.st_gid;
    result.atime = stbuf.st_atim;
    result.mtime = stbuf.st_mtim;
    result.ctime = stbuf.st_ctim;
    result.mode = mode;

    return result;
#else
    return std::optional<vfs::Stat>();
#endif /* __linux__ */
}

off_t CrpkgFile::seek(vfs::SeekWhence whence, off_t offset)
{
    int iWhence;
    switch (whence)
    {
    case vfs::SeekWhence::kSet:
        iWhence = SQUASH_SEEK_SET;
        break;
    case vfs::SeekWhence::kCurrent:
        iWhence = SQUASH_SEEK_CUR;
        break;
    case vfs::SeekWhence::kEnd:
        iWhence = SQUASH_SEEK_END;
        break;
    }
    return squash_lseek(fFile, offset, iWhence);
}

} // namespace cocoa
