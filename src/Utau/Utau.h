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

#include "Core/Project.h"
#include "Core/UniquePersistent.h"
#include "Utau/ffwrappers/samplefmt.h"

#define UTAU_NAMESPACE_BEGIN    namespace cocoa::utau {
#define UTAU_NAMESPACE_END      }

UTAU_NAMESPACE_BEGIN

enum class AudioChannelMode
{
    kUnknown,

    kMono,
    kStereo,

    kLast = kStereo
};

enum class SampleFormat : uint32_t
{
    kUnknown = 0,

    // interleaved formats
    kU8,
    kS16,
    kS32,
    kF32,
    kF64,

    // planar formats
    kU8P,
    kS16P,
    kS32P,
    kF32P,
    kF64P,

    kLast = kF64P
};


struct Ratio
{
    Ratio() : num(0), denom(1) {}
    Ratio(int32_t _num, int32_t _denom) : num(_num), denom(_denom) {}

    int32_t num;
    int32_t denom;
};

AVSampleFormat SampleFormatToLibavFormat(SampleFormat format);
SampleFormat LibavFormatToSampleFormat(AVSampleFormat format);

int GetPerSampleSize(SampleFormat format);
bool SampleFormatIsPlanar(SampleFormat format);

void InitializePlatform();

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_UTAU_H
