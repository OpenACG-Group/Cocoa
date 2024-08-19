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

#ifndef COCOA_GALLIUM_BINDINGS_FS_FILEOPS_H
#define COCOA_GALLIUM_BINDINGS_FS_FILEOPS_H

#include "Gallium/ffi/ReturnValue.h"
#include "Gallium/ffi/ArrayBuffer.h"
#include "Gallium/bindings/fs/Types.h"
GALLIUM_BINDINGS_FS_NS_BEGIN

//! TSDecl: @interface ReadFileResult
//! TSDecl: @property buffer: @mem(u8)
//! TSDecl: @property readSize: u64
//! TSDecl: @end

//! TSDecl: @function ReadFile(path: string, offset: i64, dst: @union(null, @mem(u8))): ReadFileResult
ffi::RetLocal<v8::Value> ReadFile(const std::string& path, int64_t offset,
                                  ffi::Opt<ffi::Mem<uint8_t>> dst);

//! TSDecl: @function WriteFile(path: string, content: @mem(u8), mode: u32): void
ffi::Ret<void> WriteFile(const std::string& path, const ffi::Mem<uint8_t>& content, uint32_t mode);

//! TSDecl: @function realpath(path: string): string
ffi::Ret<std::string> realpath(const std::string& path);

GALLIUM_BINDINGS_FS_NS_END
#endif //COCOA_GALLIUM_BINDINGS_FS_FILEOPS_H
