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

#include "Core/Project.h"

#define UTAU_NAMESPACE_BEGIN    namespace cocoa::utau {
#define UTAU_NAMESPACE_END      }

UTAU_NAMESPACE_BEGIN

#define E(bits)         \
    kU##bits##_LE,      \
    kU##bits##_BE,      \
    kS##bits##_LE,      \
    kS##bits##_BE

enum class SampleFormat
{
    kUnknown,

    // interleaved formats
    kS8,
    kU8,
    E(16),
    E(24_32),
    E(32),
    E(24),
    E(20),
    E(18),
    kF32_LE,
    kF32_BE,

    kULAW,
    kALAW,

    // planar formats
    kU8P,
    kS16P,
    kS24_32P,
    kS32P,
    kS24P,
    kF32P,
    kF64P,
    kS8P
};

#undef E

uint32_t SampleFormatToPipewireFormat(SampleFormat format);
SampleFormat PipewireFormatToSampleFormat(uint32_t format);
uint32_t SampleFormatToLibavFormat(SampleFormat format);
SampleFormat LibavFormatToSampleFormat(uint32_t format);

/**
 * Specify a certain purpose that the media is used in.
 */
enum class MediaRole
{
    kMovie,
    kMusic,
    kCamera,
    kCapture,
    kScreen,
    kCommunication,
    kGame,
    kNotification,
    kDSP,
    kProduction,
    kAccessibility,
    kTest
};

const char *MediaRoleToString(MediaRole role);

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_UTAU_H
