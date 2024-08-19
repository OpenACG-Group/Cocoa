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

/**
 * Utau is a generic multimedia framework based on ffmpeg and pipewire which provides
 * the basic functions like video/audio decoding and playback.
 * Advanced functions like hardware acceleration are experimental.
 */

#ifndef COCOA_UTAU_UTAU_H
#define COCOA_UTAU_UTAU_H

#include <cstdint>
#include <memory>
#include <chrono>

#include "Core/Project.h"
#include "Core/UniquePersistent.h"

#include "Utau/ffwrappers/samplefmt.h"
#include "Utau/ffwrappers/pixfmt.h"

#define UTAU_NAMESPACE_BEGIN    namespace cocoa::utau {
#define UTAU_NAMESPACE_END      }

UTAU_NAMESPACE_BEGIN

enum class ScaleResampler : uint32_t
{
    kNearest,
    kBilinear,
    kBicubic
};

struct ContextOptions
{
    std::string     hwdevice_drm_device_path;
};

class GlobalContext : public UniquePersistent<GlobalContext>
{
public:
    explicit GlobalContext(const ContextOptions& options);
    ~GlobalContext();

    g_nodiscard const ContextOptions& GetOptions() const {
        return options_;
    }

private:
    ContextOptions options_;
};

void InitializePlatform(const ContextOptions& options);
void DisposePlatform();

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_UTAU_H
