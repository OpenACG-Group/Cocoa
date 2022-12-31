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

#include <unordered_map>

#include "Core/EventLoop.h"
#include "Gallium/bindings/utau/Exports.h"
#include "Utau/Utau.h"
#include "Utau/AudioSink.h"
#include "Utau/AudioResampler.h"
#include "Utau/AVStreamDecoder.h"
#include "Utau/AudioFilterDAG.h"
#include "fmt/core.h"
GALLIUM_BINDINGS_UTAU_NS_BEGIN

void SetInstanceProperties(v8::Local<v8::Object> instance)
{
#define I(x) static_cast<int32_t>(x)

    using FMT = utau::SampleFormat;
    using ChM = utau::AudioChannelMode;

    std::unordered_map<std::string, int32_t> constants{
        { "SAMPLE_FORMAT_UNKNOWN", I(FMT::kUnknown) },
        { "SAMPLE_FORMAT_U8",   I(FMT::kU8)     },
        { "SAMPLE_FORMAT_S16",  I(FMT::kS16)    },
        { "SAMPLE_FORMAT_S32",  I(FMT::kS32)    },
        { "SAMPLE_FORMAT_F32",  I(FMT::kF32)    },
        { "SAMPLE_FORMAT_F64",  I(FMT::kF64)    },
        { "SAMPLE_FORMAT_U8P",  I(FMT::kU8P)    },
        { "SAMPLE_FORMAT_S16P", I(FMT::kS16P)   },
        { "SAMPLE_FORMAT_S32P", I(FMT::kS32P)   },
        { "SAMPLE_FORMAT_F32P", I(FMT::kF32P)   },
        { "SAMPLE_FORMAT_F64P", I(FMT::kF64P)   },

        { "CH_MODE_MONO",       I(ChM::kMono)   },
        { "CH_MODE_STEREO",     I(ChM::kStereo) }
    };

#undef I

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    instance->Set(context, binder::to_v8(isolate, "Constants"),
                  binder::to_v8(isolate, constants)).Check();
}

GALLIUM_BINDINGS_UTAU_NS_END
