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

#ifndef COCOA_GALLIUM_BINDINGS_UTAU_EXPORTS_H
#define COCOA_GALLIUM_BINDINGS_UTAU_EXPORTS_H

#define GALLIUM_BINDINGS_UTAU_NS_BEGIN   namespace cocoa::gallium::bindings::utau_wrap {
#define GALLIUM_BINDINGS_UTAU_NS_END     }

#include "Gallium/Gallium.h"
#include "Gallium/binder/Convert.h"

#include "Utau/AudioBuffer.h"
GALLIUM_BINDINGS_UTAU_NS_BEGIN

void Test();

void SetInstanceProperties(v8::Local<v8::Object> instance);

//! TSEnumDecl: Enum<SampleFormat> := Constants.SAMPLE_FORMAT_*
//! TSEnumDecl: Enum<ChannelMode> := Constants.CH_MODE_*

//! TSDecl: class AudioBuffer
class AudioBufferWrap
{
public:
    explicit AudioBufferWrap(std::shared_ptr<utau::AudioBuffer> buffer)
        : buffer_(std::move(buffer)) {}
    ~AudioBufferWrap() = default;

    //! TSDecl: readonly sampleFormat: Enum<SampleFormat>
    int32_t getSampleFormat();

    //! TSDecl: readonly channelMode: Enum<ChannelMode>
    int32_t getChannelMode();

    //! TSDecl: readonly sampleRate: number
    int32_t getSampleRate();

private:
    std::shared_ptr<utau::AudioBuffer>    buffer_;
};

GALLIUM_BINDINGS_UTAU_NS_END
#endif //COCOA_GALLIUM_BINDINGS_UTAU_EXPORTS_H
