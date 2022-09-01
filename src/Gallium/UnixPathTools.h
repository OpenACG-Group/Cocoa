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

#ifndef COCOA_GALLIUM_UNIXPATHTOOLS_H
#define COCOA_GALLIUM_UNIXPATHTOOLS_H

#include "Gallium/Gallium.h"
GALLIUM_NS_BEGIN

namespace unixpath {

/**
 * @note @a path must be a canonical string of path,
 *       which means given by @a vfs::Realpath.
 */
std::string SolveShortestPathRepresentation(const std::string& path);

}

GALLIUM_NS_END
#endif //COCOA_GALLIUM_UNIXPATHTOOLS_H
