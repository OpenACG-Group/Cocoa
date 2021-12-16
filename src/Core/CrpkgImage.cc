#include <atomic>
#include <utility>

#ifdef __linux__
#include <sys/stat.h>
#endif /* __linux__ */

extern "C" {
#include "squash.h"
}

#include "Core/Exception.h"
#include "Core/Project.h"
#include "Core/CrpkgImage.h"
#include "Core/Journal.h"
#include "Core/Filesystem.h"
#include "Core/Errors.h"
#include "Core/Data.h"

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Core)

namespace cocoa {

namespace {

std::atomic<bool> gInitializedSquash(false);

} // namespace anonymous

struct CrpkgImage::CrpkgImagePrivate
{
    size_t       imageSize;
    sqfs        *squash;
    std::shared_ptr<Data> data;
};

std::shared_ptr<CrpkgImage> CrpkgImage::MakeFromData(const std::shared_ptr<Data>& data)
{
    if (!gInitializedSquash)
    {
        squash_start();
        gInitializedSquash = true;
    }

    if (!data->hasAccessibleBuffer())
        throw RuntimeException(__func__, "Data doesn't have an accessible buffer");

    auto *pData = new CrpkgImagePrivate;
    pData->data = data;
    pData->squash = reinterpret_cast<struct sqfs*>(std::malloc(sizeof(struct sqfs)));
    pData->imageSize = data->size();

    auto bufptr = reinterpret_cast<const uint8_t*>(data->getAccessibleBuffer());
    CHECK(bufptr);

    sqfs_err ret = sqfs_init(pData->squash, const_cast<uint8_t*>(bufptr), 0);
    switch (ret)
    {
    case SQFS_OK:
        break;
    case SQFS_BADFORMAT:
        QLOG(LOG_ERROR, "Not a valid SquashFS image");
        break;
    case SQFS_BADVERSION:
    {
        int major, minor;
        sqfs_version(pData->squash, &major, &minor);
        QLOG(LOG_ERROR, "SquashFS version {}.{} detected, which is not supported", major, minor);
        break;
    }
    case SQFS_BADCOMP:
        QLOG(LOG_ERROR, "Unknown compression algorithm");
        break;
    default:
        QLOG(LOG_ERROR, "Couldn't load crpkg image");
        break;
    }
    if (ret != SQFS_OK)
        return nullptr;

    return std::make_shared<CrpkgImage>(pData);
}

CrpkgImage::CrpkgImage(CrpkgImagePrivate *data)
    : fData(data)
{
}

CrpkgImage::~CrpkgImage()
{
    if (fData->squash)
    {
        sqfs_destroy(fData->squash);
        std::free(fData->squash);
    }
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
        return {};
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

std::optional<vfs::Stat> CrpkgFile::stat() const
{
#ifdef __linux__
    struct stat stbuf;  // NOLINT
    if (squash_fstat(fFile, &stbuf) < 0)
        return {};

    Bitfield<vfs::Mode> mode;
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
