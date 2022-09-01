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

#ifndef COCOA_GLAMOR_WAYLAND_SCREENCAST_SCREENCAST_H
#define COCOA_GLAMOR_WAYLAND_SCREENCAST_SCREENCAST_H

#include "Core/Project.h"

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#define SCREENCAST_COMPAT_SIGNATURE "3b7a936a-9fdb-465e-97b7-db618c70c060"

#define SCREENCAST_NAMESPACE_BEGIN  namespace cocoa::wlcast {
#define SCREENCAST_NAMESPACE_END    }

SCREENCAST_NAMESPACE_BEGIN

enum class TextureFormat
{
    kUnknown,

    kBGRA,          /* [31:0] A:R:G:B 8:8:8:8 little endian */
    kRGBA,          /* [31:0] A:B:G:R 8:8:8:8 little endian */
    kBGRX,          /* [31:0] x:R:G:B 8:8:8:8 little endian */
    kRGBX           /* [31:0] x:B:G:R 8:8:8:8 little endian */
};

/**
 * As the communication with the host process (Cocoa process) via coproc
 * is complicated and needs some initialization, we transfer some parameters
 * by the commandline.
 */
struct ParametersFromHost
{
    // Commandline flag --compatibility-signature=sig
    std::string compatibility_signature;

    // Commandline flag --host-accept-dmabuf
    bool host_accept_dmabuf;

    // Commandline flag --drm-support-dmabuf-implicit-modifiers
    bool support_dmabuf_implicit_modifiers;

    // Commandline flag --drm-formats=format1:mod1,mod2!format2:mod1 (modifiers are in hex without 0x prefix)
    // Modifiers are numbers of DRM definition, formats are literal strings
    using ModifiersList = std::vector<uint64_t>;
    std::unordered_map<TextureFormat, ModifiersList> drm_formats;

    // Commandline flag --raster-formats=format1,format2
    std::vector<TextureFormat> raster_formats;

    // Commandline flag --fps-fraction=<numerator>:<denominator>
    uint32_t fps_num;
    uint32_t fps_den;
};

extern ParametersFromHost *g_host_params;

SCREENCAST_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_SCREENCAST_SCREENCAST_H
