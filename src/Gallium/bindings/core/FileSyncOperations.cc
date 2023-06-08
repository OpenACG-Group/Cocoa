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

#include "uv.h"

#include "fmt/format.h"

#include "Core/Errors.h"
#include "Gallium/Gallium.h"
#include "Gallium/bindings/core/Exports.h"
GALLIUM_BINDINGS_NS_BEGIN

v8::Local<v8::Value> FileWrap::ReadFileSync(const std::string& path)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    uv_fs_t req;
    int fd = uv_fs_open(nullptr, &req, path.c_str(), O_RDONLY, 0, nullptr);
    uv_fs_req_cleanup(&req);
    if (fd < 0)
    {
        g_throw(Error, fmt::format("Failed to open file {}: {}",
                                   path, uv_strerror(fd)));
    }

    ScopeExitAutoInvoker closer([fd]() {
        uv_fs_t req;
        uv_fs_close(nullptr, &req, fd, nullptr);
        uv_fs_req_cleanup(&req);
    });

    int err = uv_fs_fstat(nullptr, &req, fd, nullptr);
    size_t file_size = req.statbuf.st_size;
    uv_fs_req_cleanup(&req);

    if (err < 0)
    {
        g_throw(Error, fmt::format("Failed to stat {}: {}",
                                   path, uv_strerror(err)));
    }

    v8::Local<v8::Value> buffer = Buffer::MakeFromSize(file_size);
    Buffer *wrapper = binder::UnwrapObject<Buffer>(isolate, buffer);
    CHECK(wrapper);

    uv_buf_t buf = uv_buf_init((char*)wrapper->addressU8(), wrapper->length());
    err = uv_fs_read(nullptr, &req, fd, &buf, 1, 0, nullptr);
    uv_fs_req_cleanup(&req);

    if (err < 0)
    {
        g_throw(Error, fmt::format("Failed to read file {}: {}",
                                   path, uv_strerror(err)));
    }

    return buffer;
}

v8::Local<v8::Value> FileWrap::WriteFileSync(const std::string& path,
                                             v8::Local<v8::Value> buffer)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    Buffer *wrapper = binder::UnwrapObject<Buffer>(isolate, buffer);
    if (!wrapper)
        g_throw(TypeError, "Argument `buffer` must be an instance of core:Buffer");

    uv_fs_t req;
    int fd = uv_fs_open(nullptr,
                        &req,
                        path.c_str(),
                        O_WRONLY | O_CREAT | O_TRUNC,
                        S_IWUSR | S_IRUSR,
                        nullptr);
    uv_fs_req_cleanup(&req);

    if (fd < 0)
    {
        g_throw(Error, fmt::format("Failed to open file {}: {}",
                                   path, uv_strerror(fd)));
    }

    ScopeExitAutoInvoker closer([fd]() {
        uv_fs_t req;
        uv_fs_close(nullptr, &req, fd, nullptr);
        uv_fs_req_cleanup(&req);
    });

    uv_buf_t buf = uv_buf_init((char*)wrapper->addressU8(), wrapper->length());
    int err = uv_fs_write(nullptr, &req, fd, &buf, 1, 0, nullptr);
    uv_fs_req_cleanup(&req);

    if (err < 0)
    {
        g_throw(Error, fmt::format("Failed to write file {}: {}",
                                   path, uv_strerror(err)));
    }

    return v8::True(isolate);
}

GALLIUM_BINDINGS_NS_END
