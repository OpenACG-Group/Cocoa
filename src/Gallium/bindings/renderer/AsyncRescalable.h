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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_ASYNCRESCALABLE_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_ASYNCRESCALABLE_H

#include "include/v8.h"
#include "include/core/SkImage.h"

#include "Gallium/bindings/renderer/Types.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

struct AsyncRescaleContext
{
    static std::tuple<AsyncRescaleContext*, v8::Local<v8::Promise>>
    Create(v8::Isolate *isolate, uint32_t plane_count);

    static void Delete(AsyncRescaleContext *ctx);

    static void Callback(void *ctx, std::unique_ptr<const SkImage::AsyncReadResult> result);

    struct Plane
    {
        SkISize size;
        int bytes_per_pixel;
    };

    void PostProcess(std::unique_ptr<const SkImage::AsyncReadResult> result);

    v8::Isolate *isolate;
    v8::Global<v8::Promise::Resolver> resolver;

    uint32_t plane_count;
    Plane planes[];
};

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_ASYNCRESCALABLE_H
