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

#include "fmt/format.h"

#include "Core/EventLoop.h"
#include "Gallium/bindings/core/Exports.h"
GALLIUM_BINDINGS_NS_BEGIN

TTYStreamWrap::~TTYStreamWrap()
{
    this->close();
}

void TTYStreamWrap::close()
{
    if (closed_)
        return;

    StreamWrap::Dispose();
    uv_close(reinterpret_cast<uv_handle_t*>(handle_),
             [](uv_handle_t *ptr) { std::free(ptr); });

    handle_ = nullptr;
    closed_ = true;
}

v8::Local<v8::Value> TTYStreamWrap::OpenStdin()
{
    return OpenFromFd(0);
}

v8::Local<v8::Value> TTYStreamWrap::OpenStdout()
{
    return OpenFromFd(1);
}

v8::Local<v8::Value> TTYStreamWrap::OpenStderr()
{
    return OpenFromFd(2);
}

v8::Local<v8::Value> TTYStreamWrap::OpenFromFd(int fd)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *tty = reinterpret_cast<uv_tty_t*>(malloc(sizeof(uv_tty_t)));
    CHECK(tty);

    uv_loop_t *loop = EventLoop::Ref().handle();
    int ret = uv_tty_init(loop, tty, fd, /* ignored */ 0);
    if (ret < 0)
        g_throw(Error, fmt::format("Failed to open TTY: {}", uv_strerror(ret)));

    return binder::NewObject<TTYStreamWrap>(isolate, tty);
}

GALLIUM_BINDINGS_NS_END
