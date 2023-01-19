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

#include "fmt/format.h"

#include "Core/Data.h"
#include "Gallium/binder/ThrowExcept.h"
#include "Gallium/binder/Class.h"
#include "Gallium/bindings/utau/Exports.h"
GALLIUM_BINDINGS_UTAU_NS_BEGIN

namespace {

template<typename T>
std::optional<T> extract_object_owned_property(v8::Isolate *isolate, v8::Local<v8::Object> obj,
                                               const std::string& prop_name,
                                               const std::function<bool(v8::Local<v8::Value>)>& type_checker)
{
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::String> key_name = binder::to_v8(isolate, prop_name);
    bool has_prop = obj->HasOwnProperty(context, key_name).FromMaybe(false);
    if (!has_prop)
        return {};

    v8::Local<v8::Value> v = obj->Get(context, key_name).ToLocalChecked();
    if (!type_checker(v))
        g_throw(TypeError, fmt::format("Invalid type of object property `{}`", prop_name));

    return binder::from_v8<T>(isolate, v);
}

} // namespace anonymous

v8::Local<v8::Value> AVStreamDecoderWrap::MakeFromFile(const std::string& path,
                                                       v8::Local<v8::Value> options)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    if (!options->IsObject())
        g_throw(TypeError, "Argument `options` must be an object");

    auto obj = v8::Local<v8::Object>::Cast(options);
    CHECK(!obj.IsEmpty());

    auto opt_disable_audio = extract_object_owned_property<bool>(
            isolate, obj, "disableAudio", [](v8::Local<v8::Value> v) {return v->IsBoolean();});

    auto opt_disable_video = extract_object_owned_property<bool>(
            isolate, obj, "disableVideo", [](v8::Local<v8::Value> v) {return v->IsBoolean();});

    auto opt_use_hw_decoding = extract_object_owned_property<bool>(
            isolate, obj, "useHWDecoding", [](v8::Local<v8::Value> v) {return v->IsBoolean();});

    auto opt_audio_codec_name = extract_object_owned_property<std::string>(
            isolate, obj, "audioCodecName", [](v8::Local<v8::Value> v) {return v->IsString();});

    auto opt_video_codec_name = extract_object_owned_property<std::string>(
            isolate, obj, "videoCodecName", [](v8::Local<v8::Value> v) {return v->IsString();});

    utau::AVStreamDecoder::Options opts{};
    if (opt_disable_audio)
        opts.disable_audio = *opt_disable_audio;
    if (opt_disable_video)
        opts.disable_video = *opt_disable_video;
    if (opt_use_hw_decoding)
        opts.use_hw_decode = *opt_use_hw_decoding;
    if (opt_audio_codec_name)
        opts.audio_codec_name = *opt_audio_codec_name;
    if (opt_video_codec_name)
        opts.video_codec_name = *opt_video_codec_name;

    std::shared_ptr<Data> data = Data::MakeFromFile(path, {vfs::OpenFlags::kReadonly});
    if (!data)
        g_throw(Error, "Failed to open media file");

    std::unique_ptr<utau::AVStreamDecoder> decoder =
            utau::AVStreamDecoder::MakeFromData(data, opts);
    if (!decoder)
        g_throw(Error, "Failed to create decoder for media file");

    return binder::Class<AVStreamDecoderWrap>::create_object(
            isolate, std::move(decoder));
}

bool AVStreamDecoderWrap::hasAudioStream()
{
    return decoder_->HasAudioStream();
}

bool AVStreamDecoderWrap::hasVideoStream()
{
    return decoder_->HasVideoStream();
}

v8::Local<v8::Value> AVStreamDecoderWrap::getStreamInfo(int32_t selector)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    using Selector = utau::AVStreamDecoder::StreamSelector;
    if (selector < 0 || selector > Selector::kLast_StreamType)
        g_throw(RangeError, "Invalid enumeration value for `selector`");

    std::optional<utau::AVStreamDecoder::StreamInfo> info =
            decoder_->GetStreamInfo(static_cast<Selector>(selector));
    if (!info)
        g_throw(Error, "Failed to query stream information");

    std::unordered_map<std::string_view, v8::Local<v8::Value>> map{
        { "timeBase", MakeRational(isolate, info->time_base.num, info->time_base.denom) },
        { "duration", binder::to_v8(isolate, info->duration) }
    };

    if (selector == Selector::kAudio_StreamType)
    {
        map["sampleFormat"] = binder::to_v8(isolate, static_cast<int32_t>(info->sample_fmt));
        map["channelMode"] = binder::to_v8(isolate, static_cast<int32_t>(info->channel_mode));
        map["sampleRate"] = binder::to_v8(isolate, static_cast<int32_t>(info->sample_rate));
    }
    else
    {
        map["pixelFormat"] = binder::to_v8(isolate, static_cast<int32_t>(info->pixel_fmt));
        map["width"] = binder::to_v8(isolate, info->width);
        map["height"] = binder::to_v8(isolate, info->height);
        map["SAR"] = MakeRational(isolate, info->sar.num, info->sar.denom);
    }

    return binder::to_v8(isolate, map);
}

v8::Local<v8::Value> AVStreamDecoderWrap::decodeNextFrame()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    utau::AVStreamDecoder::AVGenericDecoded decoded = decoder_->DecodeNextFrame();
    std::unordered_map<std::string_view, v8::Local<v8::Value>> map;

    map["type"] = binder::to_v8(isolate, static_cast<int32_t>(decoded.type));

    if (decoded.type == utau::AVStreamDecoder::AVGenericDecoded::kAudio)
    {
        map["audioBuffer"] = binder::Class<AudioBufferWrap>::create_object(
                isolate, std::move(decoded.audio));
    }
    else if (decoded.type == utau::AVStreamDecoder::AVGenericDecoded::kVideo)
    {
        map["videoBuffer"] = binder::Class<VideoBufferWrap>::create_object(
                isolate, std::move(decoded.video));
    }

    return binder::to_v8(isolate, map);
}

void AVStreamDecoderWrap::seekStreamTo(int32_t selector, int64_t ts)
{
    using Selector = utau::AVStreamDecoder::StreamSelector;
    if (selector < 0 || selector > Selector::kLast_StreamType)
        g_throw(RangeError, "Invalid enumeration value for `selector`");

    if (!decoder_->SeekStreamTo(static_cast<Selector>(selector), ts))
        g_throw(Error, "Failed to seek stream to specified position");
}

void AVStreamDecoderWrap::flushDecoderBuffers(int32_t selector)
{
    using Selector = utau::AVStreamDecoder::StreamSelector;
    if (selector < 0 || selector > Selector::kLast_StreamType)
        g_throw(RangeError, "Invalid enumeration value for `selector`");

    if (!decoder_->FlushDecoderBuffers(static_cast<Selector>(selector)))
        g_throw(Error, "Failed to flush decoder buffers");
}

GALLIUM_BINDINGS_UTAU_NS_END
