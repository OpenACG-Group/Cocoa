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

#include "Gallium/bindings/utau/Exports.h"
#include "Gallium/binder/ThrowExcept.h"
#include "Utau/Utau.h"
#include "Utau/AVStreamDecoder.h"
#include "Utau/AudioSinkStream.h"
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
        { "CH_MODE_STEREO",     I(ChM::kStereo) },

        { "STREAM_SELECTOR_VIDEO", I(utau::AVStreamDecoder::kVideo_StreamType) },
        { "STREAM_SELECTOR_AUDIO", I(utau::AVStreamDecoder::kAudio_StreamType) },

        { "DECODE_BUFFER_AUDIO", I(utau::AVStreamDecoder::AVGenericDecoded::kAudio) },
        { "DECODE_BUFFER_VIDEO", I(utau::AVStreamDecoder::AVGenericDecoded::kVideo) },
        { "DECODE_BUFFER_EOF",   I(utau::AVStreamDecoder::AVGenericDecoded::kEOF)   },
        { "DECODE_BUFFER_NULL",  I(utau::AVStreamDecoder::AVGenericDecoded::kNull)  },

        { "MEDIA_TYPE_VIDEO",    I(utau::MediaType::kVideo) },
        { "MEDIA_TYPE_AUDIO",    I(utau::MediaType::kAudio) },

        { "AUDIO_SINKSTREAM_BUFFER_CONSUMED", I(utau::AudioSinkStream::BufferState::kConsumed) },
        { "AUDIO_SINKSTREAM_BUFFER_CANCELLED", I(utau::AudioSinkStream::BufferState::kCancelled) }
    };

#undef I

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    instance->Set(context, binder::to_v8(isolate, "Constants"),
                  binder::to_v8(isolate, constants)).Check();
}

v8::Local<v8::Object> MakeRational(v8::Isolate *i, int32_t num, int32_t denom)
{
    CHECK(i && "Invalid isolate");
    return binder::to_v8(i, std::unordered_map<std::string_view, v8::Local<v8::Value>>{
        { "num", binder::to_v8(i, num) },
        { "denom", binder::to_v8(i, denom) }
    });
}

utau::Ratio ExtractRational(v8::Isolate *i, v8::Local<v8::Value> v)
{
    CHECK(i && "Invalid isolate");

    utau::Ratio r;

    if (!v->IsObject())
        g_throw(TypeError, "Type `Rational` must be an object");

    auto obj = v8::Local<v8::Object>::Cast(v);
    CHECK(!obj.IsEmpty());

    auto maybe = obj->Get(i->GetCurrentContext(), binder::to_v8(i, "num"));
    if (maybe.IsEmpty())
        g_throw(TypeError, "Missing `num` property for `Rational` type");
    r.num = binder::from_v8<int32_t>(i, maybe.ToLocalChecked());

    maybe = obj->Get(i->GetCurrentContext(), binder::to_v8(i, "denom"));
    if (maybe.IsEmpty())
        g_throw(TypeError, "Missing `denom` property for `Rational` type");
    r.denom = binder::from_v8<int32_t>(i, maybe.ToLocalChecked());

    return r;
}

uint64_t getCurrentTimestampMs()
{
    return utau::GlobalContext::Ref().GetCurrentTimestampMs();
}

GALLIUM_BINDINGS_UTAU_NS_END
