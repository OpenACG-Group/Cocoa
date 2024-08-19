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

#include <algorithm>
#include <ranges>

#include "include/codec/SkCodec.h"

#include "Gallium/bindings/multimedia/ffwrappers/libavutil.h"
#include "Core/Exception.h"
#include "Gallium/bindings/renderer/Color.h"
#include "Gallium/bindings/multimedia/HWFramesContext.h"
#include "Gallium/bindings/multimedia/Frame.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

namespace {

ffi::Ret<void> fill_frame_from_spec(AVFrame *frame, const ffi::IFace<FrameSpecification>& spec)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (spec->pixel_format && spec->sample_format)
        return ffi::Fail(ffi::kTypeErr, "never set `pixelFormat` and `sampleFormat` simultaneously");

    if (spec->hw_frames_ctx)
        return ffi::Fail(ffi::kErr, "never set `hwFramesCtx` from the frame specification");

    if (spec->sar)
    {
        auto res = AVRationalAdapter::Cast(isolate, *spec->sar);
        if (res.HasError())
        {
            return ffi::Fail(ffi::kErr, fmt::format(
                    "invalid property `SAR`: {}", res.GetError().message));
        }
        frame->sample_aspect_ratio = *res.Extract();
    }

    ScopeExitAutoInvoker ch_layout_free;
    if (spec->channel_layout)
    {
        AChannelLayout *ptr = ffi::JSObject::Unwrap<AChannelLayout>(
                isolate, *spec->channel_layout);
        if (!ptr)
        {
            return ffi::Fail(ffi::kTypeErr,
                "invalid property `channelLayout`: not an instance of `AChannelLayout`");
        }
        ptr->CopyLayoutTo(frame->ch_layout);

        ch_layout_free.Reset([frame] {
            av_channel_layout_uninit(&frame->ch_layout);
        });
    }
    if (spec->pixel_format)
        frame->format = static_cast<int>(spec->pixel_format->GetInteger());
    if (spec->sample_format)
        frame->format = spec->sample_format->GetInteger();
    if (spec->width)
        frame->width = *spec->width;
    if (spec->height)
        frame->height = *spec->height;
    if (spec->nb_samples)
        frame->nb_samples = *spec->nb_samples;
    if (spec->picture_type)
        frame->pict_type = static_cast<AVPictureType>(spec->picture_type->GetInteger());
    if (spec->pts)
        frame->pts = *spec->pts;
    if (spec->sample_rate)
        frame->sample_rate = *spec->sample_rate;
    if (spec->flags)
        frame->flags = static_cast<int>(spec->flags->GetInteger());
    if (spec->color_range)
        frame->color_range = static_cast<AVColorRange>(spec->color_range->Get());
    if (spec->color_primaries)
        frame->color_primaries = static_cast<AVColorPrimaries>(spec->color_primaries->Get());
    if (spec->color_trc)
        frame->color_trc = static_cast<AVColorTransferCharacteristic>(spec->color_trc->Get());
    if (spec->color_space)
        frame->colorspace = static_cast<AVColorSpace>(spec->color_space->Get());
    if (spec->chroma_location)
        frame->chroma_location = static_cast<AVChromaLocation>(spec->chroma_location->Get());
    if (spec->duration)
        frame->duration = *spec->duration;
    if (spec->crop_top)
        frame->crop_top = *spec->crop_top;
    if (spec->crop_bottom)
        frame->crop_bottom = *spec->crop_bottom;
    if (spec->crop_left)
        frame->crop_left = *spec->crop_left;
    if (spec->crop_right)
        frame->crop_right = *spec->crop_right;

    ch_layout_free.Cancel();
    return {};
}

} // namespace anonymous

Frame::~Frame()
{
    if (frame_)
        av_frame_free(&frame_);
}

void Frame::NotifyViewProxiesOfDispose()
{
    for (FrameViewProxy *view_proxy : view_proxies_)
        view_proxy->NotifyParentDispose();
    view_proxies_.clear();
}

void Frame::AddFrameViewProxy(FrameViewProxy *proxy)
{
    CHECK(std::find(view_proxies_.begin(), view_proxies_.end(), proxy) == view_proxies_.end()
          && "duplicated FrameViewProxy in the list");
    view_proxies_.push_back(proxy);
}

void Frame::RemoveFrameViewProxy(FrameViewProxy *proxy)
{
    view_proxies_.remove(proxy);
}

ffi::RetLocal<v8::Value> Frame::MakeUnallocated(const ffi::IFace<FrameSpecification>& spec)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    AVFrame *frame = av_frame_alloc();
    CHECK(frame && "allocation failed");
    if (auto res = fill_frame_from_spec(frame, spec); res.HasError())
    {
        av_frame_free(&frame);
        return ffi::Fail(res.GetError().type,
            fmt::format("invalid specification: {}", res.GetError().message));
    }
    v8::Local<v8::Object> obj = ffi::JSObject::New<Frame>(isolate, frame);
    ffi::JSObject::Unwrap<Frame>(isolate, obj)->NotifyDisposeState(DisposeState::kDisposed);
    return obj;
}

ffi::Ret<void> Frame::allocate()
{
    const auto indices = std::ranges::views::iota(0, AV_NUM_DATA_POINTERS);
    if (std::any_of(indices.begin(), indices.end(), [this](int i) { return frame_->data[i]; }))
        return ffi::Fail(ffi::kErr, "buffers has already been allocated");

    if (int res = av_frame_get_buffer(frame_, 0); res < 0)
        return ffi::Fail(ffi::kErr, fmt::format("failed to allocate: {}", av_err2str(res)));
    NotifyDisposeState(DisposeState::kNot);
    return {};
}

ffi::RetLocal<v8::Value> Frame::clone()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<Frame>(isolate, av_frame_clone(frame_));
}

ffi::Ret<void> Frame::dispose()
{
    av_frame_free(&frame_);
    NotifyDisposeState(DisposeState::kDisposed);
    NotifyViewProxiesOfDispose();
    return {};
}

ffi::Ret<void> Frame::disposeReusable(const ffi::IFace<FrameSpecification>& spec)
{
    av_frame_unref(frame_);
    NotifyDisposeState(DisposeState::kDisposed);
    NotifyViewProxiesOfDispose();
    if (auto res = fill_frame_from_spec(frame_, spec); res.HasError())
    {
        return ffi::Fail(res.GetError().type,
            fmt::format("invalid specification: {}", res.GetError().message));
    }
    return {};
}

ffi::Ret<ffi::IFace<FrameSpecification>> Frame::specification(ffi::Enum<MediaType> type)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto spec = ffi::IFace<FrameSpecification>::Construct();
    if (*type == MediaType::kAudio)
    {
        spec->sample_format = static_cast<SampleFormat>(frame_->format);
        spec->nb_samples = frame_->nb_samples;
        spec->sample_rate = frame_->sample_rate;
        spec->channel_layout = ffi::JSObject::New<AChannelLayout>(
                isolate, frame_->ch_layout, AChannelLayout::kNotFreeOriginal);
    }
    else if (*type == MediaType::kVideo)
    {
        spec->pixel_format = static_cast<PixelFormat>(frame_->format);
        spec->width = frame_->width;
        spec->height = frame_->height;
        spec->picture_type = static_cast<PictureType>(frame_->pict_type);
        spec->sar = CreateJSRational(isolate, frame_->sample_aspect_ratio);
        spec->color_range = static_cast<ColorRange>(frame_->color_range);
        spec->color_primaries = static_cast<ColorPrimaries>(frame_->color_primaries);
        spec->color_trc = static_cast<ColorTransferCharacteristic>(frame_->color_trc);
        spec->color_space = static_cast<ColorSpace>(frame_->colorspace);
        spec->chroma_location = static_cast<ChromaLocation>(frame_->chroma_location);
        if (frame_->hw_frames_ctx)
        {
            spec->hw_frames_ctx = ffi::JSObject::New<HWFramesContext>(isolate, frame_->hw_frames_ctx);
            spec->pixel_format = PixelFormat::kNone;
        }

        spec->crop_top = frame_->crop_top;
        spec->crop_bottom = frame_->crop_bottom;
        spec->crop_left = frame_->crop_left;
        spec->crop_right = frame_->crop_right;
    }
    else
    {
        return ffi::Fail(ffi::kErr, "invalid media type");
    }

    if (frame_->pts != AV_NOPTS_VALUE)
        spec->pts = frame_->pts;
    spec->flags = static_cast<FrameFlags>(frame_->flags);
    if (frame_->duration > 0)
        spec->duration = frame_->duration;
    spec->best_effort_timestamp = frame_->best_effort_timestamp;

    return std::move(spec);
}

ffi::Ret<void> Frame::updateSpecification(const ffi::IFace<FrameSpecification>& spec)
{
    fill_frame_from_spec(frame_, spec);
    return {};
}

FrameViewProxy::FrameViewProxy(const ffi::Class<Frame>& frame)
{
    // The exception should be caught by our FFI trampoline and converted
    // into a JavaScript exception.
    if (frame->GetDisposeState() != ffi::JSObject::DisposeState::kNot)
        throw std::runtime_error("provided `Frame` instance has been disposed");
    if (frame->GetAVFrame()->hw_frames_ctx)
        throw std::runtime_error("creating a view from hardware frame is not allowed");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    frame_.Reset(isolate, frame->GetThisHandle(isolate));
    frame_->AddFrameViewProxy(this);
}

FrameViewProxy::~FrameViewProxy()
{
    // If the `Frame` is not disposed until this view proxy has been deleted,
    // remove the view proxy from the list.
    if (!frame_.IsEmpty())
        frame_->RemoveFrameViewProxy(this);
}

void FrameViewProxy::NotifyParentDispose()
{
    // We do not need to call `frame_->RemoveFrameViewProxy(this)`, as the
    // `Frame::NotifyViewProxiesOfDispose()` will remove all the proxies in the list.
    frame_.Reset();
}

AVFrame *FrameViewProxy::TryRetrieveHandle() const
{
    if (frame_.IsEmpty())
        return nullptr;
    return frame_->GetAVFrame();
}

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
