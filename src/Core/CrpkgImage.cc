/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

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

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Core.Crpkg)

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

std::shared_ptr<CrpkgDirectory> CrpkgImage::openDir(const std::string& path)
{
    SQUASH_DIR *dirp = squash_opendir(fData->squash, path.c_str());
    if (!dirp)
        return nullptr;
    return std::make_shared<CrpkgDirectory>(dirp, shared_from_this());
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

CrpkgDirectory::ScopedSeekRewind::ScopedSeekRewind(std::shared_ptr<CrpkgDirectory> dir,
                                                   bool rewindImmediately)
    : fDir(std::move(dir))
{
    CHECK(fDir != nullptr);
    if (rewindImmediately)
        squash_rewinddir(dir->fDirp);
}

CrpkgDirectory::ScopedSeekRewind::~ScopedSeekRewind()
{
    squash_rewinddir(fDir->fDirp);
}

CrpkgDirectory::CrpkgDirectory(SQUASH_DIR *dirp, std::shared_ptr<CrpkgImage> image)
    : fDirp(dirp)
    , fImage(std::move(image))
{
}

CrpkgDirectory::~CrpkgDirectory()
{
    squash_closedir(fDirp);
}

namespace {

CrpkgDirectory::NameFilterMode dirent_type_to_filter_mode(uint8_t type)
{
    CrpkgDirectory::NameFilterMode filter;
    switch (type)
    {
    case DT_BLK:
    case DT_CHR:
    case DT_FIFO:
    case DT_REG:
    case DT_SOCK:
    case DT_UNKNOWN:
        filter = CrpkgDirectory::NameFilterMode::kRegular;
        break;
    case DT_LNK:
        filter = CrpkgDirectory::NameFilterMode::kLinked;
        break;
    case DT_DIR:
        filter = CrpkgDirectory::NameFilterMode::kDirectory;
        break;
    default:
        filter = CrpkgDirectory::NameFilterMode::kUnknown;
        break;
    }
    return filter;
}

} // namespace anonymous

bool CrpkgDirectory::contains(const std::string& name, Bitfield<NameFilterMode> filterMask)
{
    ScopedSeekRewind scope(shared_from_this(), true);

    dirent *d = squash_readdir(fDirp);
    while (d)
    {
        NameFilterMode filter = dirent_type_to_filter_mode(d->d_type);
        if (d->d_name == name && (filterMask & filter))
            return true;
        d = squash_readdir(fDirp);
    }
    return false;
}

void CrpkgDirectory::foreachEntry(const ElementForeachFunc& func)
{
    ScopedSeekRewind scope(shared_from_this(), true);

    dirent *d = squash_readdir(fDirp);
    while (d)
    {
        NameFilterMode filter = dirent_type_to_filter_mode(d->d_type);
        func(d->d_name, filter);
        d = squash_readdir(fDirp);
    }
}

} // namespace cocoa
