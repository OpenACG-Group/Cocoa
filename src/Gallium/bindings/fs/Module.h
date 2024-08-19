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

#ifndef COCOA_GALLIUM_BINDINGS_FS_MODULE_H
#define COCOA_GALLIUM_BINDINGS_FS_MODULE_H

#include "Gallium/ffi/Module.h"
#include "Gallium/bindings/fs/Types.h"
GALLIUM_BINDINGS_NS_BEGIN

class FsModule : public ffi::NativeModule
{
public:
    FsModule();
    ~FsModule() override = default;

    ffi::LocalExports OnBuildExports(v8::Isolate *isolate) override;
};

GALLIUM_BINDINGS_NS_END
#endif //COCOA_GALLIUM_BINDINGS_FS_MODULE_H
