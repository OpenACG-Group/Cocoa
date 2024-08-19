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

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "fmt/format.h"

#include "Core/Exception.h"
#include "Gallium/bindings/fs/FileOps.h"
#include "Gallium/bindings/fs/SysError.h"
GALLIUM_BINDINGS_FS_NS_BEGIN

ffi::RetLocal<v8::Value> ReadFile(const std::string& path, int64_t offset,
                                  ffi::Opt<ffi::Mem<uint8_t>> dst)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0)
        return SysFail(fmt::format("failed to open `{}`", path));

    ScopeExitAutoInvoker close_on_leave([fd]() {
        close(fd);
    });

    struct stat statbuf{};
    if (fstat(fd, &statbuf) < 0)
        return SysFail(fmt::format("failed to stat file `{}`", path));

    if (offset >= statbuf.st_size)
        return ffi::Fail(ffi::kErr, "invalid offset for reading file");
    if (lseek(fd, offset, SEEK_SET) < 0)
        return SysFail(fmt::format("failed to lseek file `{}`", path));

    size_t read_size = statbuf.st_size - offset;
    v8::Local<v8::Uint8Array> dst_buffer;
    uint8_t *dst_address;
    if (dst)
    {
        read_size = std::min(read_size, dst->ByteSize());
        dst_buffer = dst->TypedArray();
        dst_address = dst->Address();
    }
    else
    {
        auto ab = v8::ArrayBuffer::New(isolate, read_size);
        dst_buffer = v8::Uint8Array::New(ab, 0, read_size);
        dst_address = static_cast<uint8_t*>(ab->Data());
    }

    ssize_t actual_read_size = read(fd, dst_address, read_size);
    if (actual_read_size < 0)
        return SysFail(fmt::format("failed to read file `{}`", path));

    return ffi::Cast<ffi::ObjectLiteralMap>::ToChecked(isolate, {
        { "buffer", dst_buffer },
        { "readSize", ffi::Cast<ssize_t>::ToChecked(isolate, actual_read_size) }
    });
}

ffi::Ret<void> WriteFile(const std::string& path, const ffi::Mem<uint8_t>& content, uint32_t mode)
{
    int fd = open(path.c_str(), O_RDWR | O_CREAT, mode);
    if (fd < 0)
        return SysFail(fmt::format("failed to open file `{}`", path));
    ScopeExitAutoInvoker close_fd([fd] {
        close(fd);
    });

    if (ftruncate(fd, 0) < 0)
        return SysFail(fmt::format("failed to truncate file `{}`", path));

    if (content.ByteSize() == 0)
        return {};

    ssize_t result = write(fd, content.Address(), content.ByteSize());
    if (result < 0)
        return SysFail(fmt::format("failed to write file `{}`", path));

    if (result != content.ByteSize())
        return ffi::Fail(ffi::kErr, "failed to write all bytes into file");

    return {};
}

ffi::Ret<std::string> realpath(const std::string& path)
{
    char *resolved = ::realpath(path.c_str(), nullptr);
    if (!resolved)
        return SysFail(fmt::format("failed to resolve path `{}`", path));

    std::string result(resolved);
    free(resolved);
    return result;
}

GALLIUM_BINDINGS_FS_NS_END
