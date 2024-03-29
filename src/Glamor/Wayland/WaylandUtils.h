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

#ifndef COCOA_GLAMOR_WAYLAND_WAYLANDUTILS_H
#define COCOA_GLAMOR_WAYLAND_WAYLANDUTILS_H

#include <wayland-client-protocol.h>

#include "include/core/SkColor.h"
#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

SkColorType WlShmFormatToSkColorType(wl_shm_format type);
wl_shm_format SkColorTypeToWlShmFormat(SkColorType type);

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_WAYLANDUTILS_H
