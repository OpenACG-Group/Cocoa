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

#include <vector>

#include "fmt/format.h"

#include "Gallium/bindings/utau/Exports.h"
#include "Gallium/binder/Class.h"
#include "Gallium/binder/Convert.h"
GALLIUM_BINDINGS_UTAU_NS_BEGIN

using DAG = utau::AVFilterDAG;

namespace {

template<typename T>
T extract_object_owned_property(v8::Isolate *isolate, v8::Local<v8::Object> obj,
                                const std::string& prop_name,
                                const std::function<bool(v8::Local<v8::Value>)>& type_checker)
{
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::String> key_name = binder::to_v8(isolate, prop_name);
    bool has_prop = obj->HasOwnProperty(context, key_name).FromMaybe(false);
    if (!has_prop)
        g_throw(TypeError, fmt::format("Missing required property `{}`", prop_name));

    v8::Local<v8::Value> v = obj->Get(context, key_name).ToLocalChecked();
    if (!type_checker(v))
        g_throw(TypeError, fmt::format("Invalid type of object property `{}`", prop_name));

    return binder::from_v8<T>(isolate, v);
}

template<typename T>
T extract_params_obj(g_maybe_unused v8::Isolate *isolate,
                     g_maybe_unused v8::Local<v8::Object> obj)
{
    MARK_UNREACHABLE();
}

AVBufferRef *extract_possible_hw_frame_ctx_from_inparams(v8::Local<v8::Object> obj)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    auto maybe_buf = obj->Get(context, binder::to_v8(isolate, "hwFrameContextFrom"));

    if (maybe_buf.IsEmpty())
        return nullptr;
    auto buf = maybe_buf.ToLocalChecked();

    if (buf->IsNullOrUndefined())
        return nullptr;

    auto *wrapper = binder::UnwrapObject<VideoBufferWrap>(isolate, buf);
    if (!wrapper)
        g_throw(TypeError, "Property `hwFrameContextFrom` must be an instance of `VideoBuffer`");

    return wrapper->GetBuffer()->CastUnderlyingPointer<AVFrame>()->hw_frames_ctx;
}

template<>
DAG::InBufferParameters extract_params_obj(v8::Isolate *isolate,
                                           v8::Local<v8::Object> obj)
{
    DAG::InBufferParameters params{};
    params.name = extract_object_owned_property<std::string>(
            isolate, obj, "name", [](v8::Local<v8::Value> v) {return v->IsString();});

    auto number_type_checker = [](v8::Local<v8::Value> v) {
        return v->IsNumber();
    };

    int media_type_v = extract_object_owned_property<int>(isolate, obj, "mediaType", number_type_checker);
    if (media_type_v < 0 || media_type_v > static_cast<int>(utau::MediaType::kLast))
        g_throw(RangeError, "Invalid enumeration value for `mediaType` property in `inparams`");

    params.media_type = static_cast<utau::MediaType>(media_type_v);

    if (params.media_type == utau::MediaType::kAudio)
    {
        auto ev = extract_object_owned_property<int32_t>(
                isolate, obj, "sampleFormat", number_type_checker);
        if (ev < 0 || ev > static_cast<int32_t>(utau::SampleFormat::kLast))
            g_throw(RangeError, "Invalid enumeration value for a sample format");
        params.sample_fmt = static_cast<utau::SampleFormat>(ev);

        ev = extract_object_owned_property<int32_t>(
                isolate, obj, "channelMode", number_type_checker);
        if (ev < 0 || ev > static_cast<int32_t>(utau::AudioChannelMode::kLast))
            g_throw(RangeError, "Invalid enumeration value for a channel mode");
        params.channel_mode = static_cast<utau::AudioChannelMode>(ev);

        params.sample_rate = extract_object_owned_property<int32_t>(
                isolate, obj, "sampleRate", number_type_checker);
        if (params.sample_rate <= 0)
            g_throw(RangeError, "Invalid value for a sample rate");
    }
    else if (params.media_type == utau::MediaType::kVideo)
    {
        auto v = extract_object_owned_property<int32_t>(
                isolate, obj, "pixelFormat", number_type_checker);
        if (v < 0 || v > AVPixelFormat::AV_PIX_FMT_NB)
            g_throw(RangeError, "Invalid enumeration value for pixel format");
        params.pixel_fmt = static_cast<AVPixelFormat>(v);

        params.hw_frame_ctx = extract_possible_hw_frame_ctx_from_inparams(obj);

        params.width = extract_object_owned_property<int32_t>(
                isolate, obj, "width", number_type_checker);
        params.height = extract_object_owned_property<int32_t>(
                isolate, obj, "height", number_type_checker);

        if (!obj->HasOwnProperty(isolate->GetCurrentContext(),
                                 binder::to_v8(isolate, "timeBase")).FromMaybe(false))
        {
            g_throw(TypeError, "Missing `timeBase` property in `inparams`");
        }
        params.time_base = ExtractRational(isolate, obj->Get(
                isolate->GetCurrentContext(), binder::to_v8(isolate, "timeBase")).ToLocalChecked());

        if (!obj->HasOwnProperty(isolate->GetCurrentContext(),
                                 binder::to_v8(isolate, "SAR")).FromMaybe(false))
        {
            g_throw(TypeError, "Missing `SAR` property in `inparams`");
        }
        params.sar = ExtractRational(isolate, obj->Get(
                isolate->GetCurrentContext(), binder::to_v8(isolate, "SAR")).ToLocalChecked());
    }
    else
    {
        MARK_UNREACHABLE();
    }

    return params;
}

template<>
DAG::OutBufferParameters extract_params_obj(v8::Isolate *isolate,
                                            v8::Local<v8::Object> obj)
{
    DAG::OutBufferParameters params{};

    params.name = extract_object_owned_property<std::string>(
            isolate, obj, "name", [](v8::Local<v8::Value> v) {return v->IsString();});

    int media_type_v = extract_object_owned_property<int>(
            isolate, obj, "mediaType", [](v8::Local<v8::Value> v) {return v->IsNumber();});
    if (media_type_v < 0 || media_type_v > static_cast<int>(utau::MediaType::kLast))
        g_throw(RangeError, "Invalid enumeration value for `mediaType` property in `outparams`");

    params.media_type = static_cast<utau::MediaType>(media_type_v);

    return params;
}

template<typename T>
std::vector<T> extract_params_array(v8::Isolate *isolate,
                                    v8::Local<v8::Value> params)
{
    if (!params->IsArray())
        g_throw(TypeError, "inparams and outparams must be arrays");

    auto array = v8::Local<v8::Array>::Cast(params);
    CHECK(!array.IsEmpty());

    std::vector<T> result;

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    for (int i = 0; i < array->Length(); i++)
    {
        v8::Local<v8::Value> v = array->Get(context, i).FromMaybe(v8::Local<v8::Value>());
        CHECK(!v.IsEmpty());

        if (!v->IsObject())
            g_throw(TypeError, "Members in `inparams` or `outparams` are not objects");

        result.emplace_back(extract_params_obj<T>(
                isolate, v8::Local<v8::Object>::Cast(v)));
    }

    return result;
}

v8::Local<v8::Value> wrap_named_inout_buffers(v8::Isolate *isolate,
                                              const std::vector<DAG::NamedInOutBuffer>& buffers)
{
    std::vector<v8::Local<v8::Value>> result;

    for (const auto& buffer : buffers)
    {
        std::unordered_map<std::string_view, v8::Local<v8::Value>> map{
            {"name",   binder::to_v8(isolate, buffer.name)}
        };
        if (buffer.media_type == utau::MediaType::kAudio)
        {
            map["audioBuffer"] = binder::NewObject<AudioBufferWrap>(
                    isolate, buffer.audio_buffer);
        }
        else if (buffer.media_type == utau::MediaType::kVideo)
        {
            map["videoBuffer"] = binder::NewObject<VideoBufferWrap>(
                    isolate, buffer.video_buffer);
        }
        else
        {
            MARK_UNREACHABLE();
        }
        result.emplace_back(binder::to_v8(isolate, map));
    }

    return binder::to_v8(isolate, result);
}

std::vector<DAG::NamedInOutBuffer> extract_named_inout_buffers(v8::Isolate *isolate,
                                                               v8::Local<v8::Value> wrapped)
{
    std::vector<DAG::NamedInOutBuffer> result;

    if (!wrapped->IsArray())
        g_throw(TypeError, "`inbuffers` must be an array");

    auto array = v8::Local<v8::Array>::Cast(wrapped);

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    for (int32_t i = 0; i < array->Length(); i++)
    {
        auto v = array->Get(context, i).ToLocalChecked();
        if (!v->IsObject())
            g_throw(TypeError, "Elements of `inbuffers` must be objects");

        auto buffer_obj = v8::Local<v8::Object>::Cast(v);

        // Extract name and media_type
        DAG::NamedInOutBuffer cur{};
        cur.name = extract_object_owned_property<std::string>(
                isolate, buffer_obj, "name", [](v8::Local<v8::Value> v) {return v->IsString();});

        int media_type_v = extract_object_owned_property<int>(
                isolate, buffer_obj, "mediaType", [](v8::Local<v8::Value> v) {return v->IsNumber();});
        if (media_type_v < 0 || media_type_v > static_cast<int>(utau::MediaType::kLast))
            g_throw(RangeError, "Invalid enumeration value for `mediaType` property in `outparams`");

        cur.media_type = static_cast<utau::MediaType>(media_type_v);

        // Extract buffers
        if (cur.media_type == utau::MediaType::kAudio)
        {
            auto maybe_buf = buffer_obj->Get(context, binder::to_v8(isolate, "audioBuffer"));
            if (maybe_buf.IsEmpty())
                g_throw(TypeError, fmt::format("Missing `audioBuffer` property in input `{}`", cur.name));

            auto *wrapper = binder::UnwrapObject<AudioBufferWrap>(isolate, maybe_buf.ToLocalChecked());
            if (!wrapper)
                g_throw(TypeError, "Property `audioBuffer` must be an instance of `AudioBuffer`");

            cur.audio_buffer = wrapper->GetBuffer();
        }
        else if (cur.media_type == utau::MediaType::kVideo)
        {
            auto maybe_buf = buffer_obj->Get(context, binder::to_v8(isolate, "videoBuffer"));
            if (maybe_buf.IsEmpty())
                g_throw(TypeError, fmt::format("Missing `videoBuffer` property in input `{}`", cur.name));

            auto *wrapper = binder::UnwrapObject<VideoBufferWrap>(isolate, maybe_buf.ToLocalChecked());
            if (!wrapper)
                g_throw(TypeError, "Property `videoBuffer` must be an instance of `VideoBuffer`");

            cur.video_buffer = wrapper->GetBuffer();
        }
        else
        {
            MARK_UNREACHABLE();
        }

        result.push_back(cur);
    }

    return result;
}

} // namespace anonymous

v8::Local<v8::Value> AVFilterDAGWrap::MakeFromDSL(const std::string& dsl,
                                                  v8::Local<v8::Value> inparams,
                                                  v8::Local<v8::Value> outparams)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    std::vector<DAG::InBufferParameters> inparams_v =
            extract_params_array<DAG::InBufferParameters>(isolate, inparams);
    std::vector<DAG::OutBufferParameters> outparams_v =
            extract_params_array<DAG::OutBufferParameters>(isolate, outparams);

    std::unique_ptr<DAG> filter = DAG::MakeFromDSL(dsl, inparams_v, outparams_v);
    if (!filter)
        g_throw(Error, "Failed to create filters DAG");

    return binder::NewObject<AVFilterDAGWrap>(
            isolate, std::move(filter));
}

v8::Local<v8::Value> AVFilterDAGWrap::filter(v8::Local<v8::Value> inbuffers)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    std::vector<DAG::NamedInOutBuffer> in_buffers =
            extract_named_inout_buffers(isolate, inbuffers);

    return wrap_named_inout_buffers(isolate, DAG_->Filter(in_buffers));
}

GALLIUM_BINDINGS_UTAU_NS_END
