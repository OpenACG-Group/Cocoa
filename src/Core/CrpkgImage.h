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

#ifndef COCOA_CORE_CRPKGIMAGE_H
#define COCOA_CORE_CRPKGIMAGE_H

#include <string>
#include <memory>
#include <optional>
#include <functional>

extern "C" {
#include "squash.h"
}

#include "Core/Project.h"
#include "Core/Filesystem.h"
#include "Core/EnumClassBitfield.h"

namespace cocoa {

class CrpkgFile;
class CrpkgDirectory;
class Data;

/**
 * Crpkg (Cocoa Resource Package) is a packed and compressed virtual filesystem
 * based on Linux's SquashFS. CrpkgImage is a userspace platform-independent
 * implementation of SquashFS driver.
 */
class CrpkgImage : public std::enable_shared_from_this<CrpkgImage>
{
public:
    struct CrpkgImagePrivate;

    explicit CrpkgImage(CrpkgImagePrivate *data);
    CrpkgImage(const CrpkgImage&) = delete;
    CrpkgImage& operator=(const CrpkgImage&) = delete;
    ~CrpkgImage();

    static std::shared_ptr<CrpkgImage> MakeFromData(const std::shared_ptr<Data>& data);

    std::shared_ptr<CrpkgFile> openFile(const std::string& path);
    std::shared_ptr<CrpkgDirectory> openDir(const std::string& path);
    std::optional<std::string> readlink(const std::string& path);

private:
    CrpkgImagePrivate       *fData;
};

class CrpkgFile
{
public:
    CrpkgFile(int32_t vfd, std::shared_ptr<CrpkgImage> image);
    ~CrpkgFile();

    g_nodiscard inline std::shared_ptr<CrpkgImage> getImage() const {
        return fImage;
    }

    ssize_t read(void *buffer, ssize_t size) const;
    off_t seek(vfs::SeekWhence whence, off_t offset);
    g_nodiscard std::optional<vfs::Stat> stat() const;

private:
    int32_t                     fFile;
    std::shared_ptr<CrpkgImage> fImage;
};

class CrpkgDirectory : public std::enable_shared_from_this<CrpkgDirectory>
{
public:
    class ScopedSeekRewind
    {
    public:
        explicit ScopedSeekRewind(std::shared_ptr<CrpkgDirectory> dir,
                                  bool rewindImmediately = false);
        ScopedSeekRewind(const ScopedSeekRewind&) = delete;
        ~ScopedSeekRewind();

        ScopedSeekRewind& operator=(const ScopedSeekRewind&) = delete;

    private:
        std::shared_ptr<CrpkgDirectory> fDir;
    };

    friend class ScopedSeekRewind;

    enum class NameFilterMode : uint8_t
    {
        /* fifo (named-pipe), socket, block and char files are treated as regular files */
        kRegular    = 1 << 0,
        kLinked     = 1 << 1,
        kDirectory  = 1 << 2,
        kUnknown    = 1 << 3,
        kAny        = kRegular | kLinked | kDirectory
    };

    CrpkgDirectory(SQUASH_DIR *dirp, std::shared_ptr<CrpkgImage> image);
    ~CrpkgDirectory();

    g_nodiscard inline std::shared_ptr<CrpkgImage> getImage() const {
        return fImage;
    }

    g_nodiscard bool contains(const std::string& name,
                               Bitfield<NameFilterMode> filter = {NameFilterMode::kAny});

    using ElementForeachFunc = std::function<void(const std::string&, NameFilterMode)>;
    void foreachEntry(const ElementForeachFunc& func);

private:
    SQUASH_DIR                     *fDirp;
    std::shared_ptr<CrpkgImage>     fImage;
};

} // namespace cocoa
#endif //COCOA_CORE_CRPKGIMAGE_H
