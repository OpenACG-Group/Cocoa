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

#include "Gallium/bindings/utau/Exports.h"
#include "Utau/AudioBuffer.h"
GALLIUM_BINDINGS_UTAU_NS_BEGIN

int32_t AudioBufferWrap::getSampleFormat()
{
    return static_cast<int32_t>(buffer_->GetInfo().GetSampleFormat());
}

int32_t AudioBufferWrap::getChannelMode()
{
    return static_cast<int32_t>(buffer_->GetInfo().GetChannelMode());
}

int32_t AudioBufferWrap::getSampleRate()
{
    return buffer_->GetInfo().GetSampleRate();
}

int32_t AudioBufferWrap::getSamplesCount()
{
    return buffer_->GetInfo().GetSamplesCount();
}

GALLIUM_BINDINGS_UTAU_NS_END
