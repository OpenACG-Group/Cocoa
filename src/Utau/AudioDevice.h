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

#ifndef COCOA_UTAU_AUDIODEVICE_H
#define COCOA_UTAU_AUDIODEVICE_H

#include "uv.h"

#include "Utau/Utau.h"
UTAU_NAMESPACE_BEGIN

class AudioSinkStream;

class AudioDevice
{
public:
    enum Backend
    {
        kPipeWire_Backend
    };

    static std::shared_ptr<AudioDevice> MakePipeWire(uv_loop_t *loop);

    explicit AudioDevice(Backend backend) : backend_(backend) {}
    virtual ~AudioDevice() = default;

    g_nodiscard g_inline Backend GetBackend() const {
        return backend_;
    }

    virtual std::unique_ptr<AudioSinkStream> CreateSinkStream(const std::string& name) = 0;

private:
    Backend backend_;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_AUDIODEVICE_H
