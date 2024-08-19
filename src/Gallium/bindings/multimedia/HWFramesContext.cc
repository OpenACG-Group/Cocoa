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

#include "Gallium/bindings/multimedia/HWFramesContext.h"
#include "Utau/HWDeviceContext.h"
#include "Utau/Utau.h"

#include "Gallium/bindings/multimedia/Frame.h"
#include "HWDeviceContext.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

HWFramesContext::~HWFramesContext()
{
    av_buffer_unref(&hwframes_ctx_);
}

ffi::RetLocal<v8::Value> HWFramesContext::GetPlatformConstraints(ffi::Class<HWDeviceContext> device)
{
    if (device->GetDisposeState() != DisposeState::kNot)
        return ffi::Fail(ffi::kErr, "device context has been disposed");
    AVHWFramesConstraints *cons = av_hwdevice_get_hwframe_constraints(device->GetContext(), nullptr);
    if (!cons)
        return ffi::Fail(ffi::kErr, "the constraints information is not available");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    std::vector<v8::Local<v8::Value>> formats_jsv;
    for (auto *ptr = cons->valid_sw_formats; *ptr != AV_PIX_FMT_NONE; ptr++)
        formats_jsv.emplace_back(v8::Int32::New(isolate, static_cast<int>(*ptr)));

    ffi::ObjectLiteralMap result{
        { "formats", v8::Array::New(isolate, formats_jsv.data(), formats_jsv.size()) },
        { "minWidth", v8::Int32::New(isolate, cons->min_width) },
        { "minHeight", v8::Int32::New(isolate, cons->min_height) },
        { "maxWidth", v8::Int32::New(isolate, cons->max_width) },
        { "maxHeight", v8::Int32::New(isolate, cons->max_height) }
    };

    double infinity = std::numeric_limits<double>::infinity();
    if (cons->max_width == INT_MAX)
        result["maxWidth"] = v8::Number::New(isolate, infinity);
    if (cons->max_height == INT_MAX)
        result["maxHeight"] = v8::Number::New(isolate, infinity);
    av_hwframe_constraints_free(&cons);

    return ffi::Cast<ffi::ObjectLiteralMap>::ToChecked(isolate, result);
}

ffi::RetLocal<v8::Value> HWFramesContext::Make(ffi::Class<HWDeviceContext> device, int32_t width, int32_t height,
                                               ffi::Enum<PixelFormat> format, int32_t initial_pool_size)
{
    if (width <= 0 || height <= 0 || initial_pool_size < 0)
        return ffi::Fail(ffi::kErr, "invalid arguments");

    if (device->GetDisposeState() != DisposeState::kNot)
        return ffi::Fail(ffi::kErr, "device context has been disposed");

    AVBufferRef *hwframes_ctx_ref = av_hwframe_ctx_alloc(device->GetContext());
    CHECK(hwframes_ctx_ref && "allocation failed");
    AVHWFramesContext *hwframes_ctx = reinterpret_cast<AVHWFramesContext*>(hwframes_ctx_ref->data);

    utau::HWDeviceContext *embedded = utau::HWDeviceContext::GetEmbedded(device->GetContext());

    hwframes_ctx->width = width;
    hwframes_ctx->height = height;
    hwframes_ctx->format = embedded->GetDeviceFormat();
    hwframes_ctx->sw_format = static_cast<AVPixelFormat>(*format);
    hwframes_ctx->initial_pool_size = initial_pool_size;
    if (!embedded->FillHWFramesContextBackendSpecific(hwframes_ctx))
    {
        av_buffer_unref(&hwframes_ctx_ref);
        return ffi::Fail(ffi::kErr, "failed to initialize backend-specific information");
    }

    if (int res = av_hwframe_ctx_init(hwframes_ctx_ref); res < 0)
    {
        av_buffer_unref(&hwframes_ctx_ref);
        return ffi::Fail(ffi::kErr, fmt::format("failed to initialize context: {}", av_err2str(res)));
    }

    return ffi::JSObject::New<HWFramesContext>(v8::Isolate::GetCurrent(), hwframes_ctx_ref);
}

ffi::RetLocal<v8::Value> HWFramesContext::getBuffer(const ffi::Opt<ffi::Class<Frame>>& reuse)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    AVFrame *avframe;
    if (reuse)
    {
        if (!(*reuse)->IsReusable())
            return ffi::Fail(ffi::kErr, "provided Frame instance is not reusable");
        avframe = (*reuse)->GetAVFrame();
    }
    else
    {
        avframe = av_frame_alloc();
        CHECK(avframe && "allocation failed");
    }

    if (int res = av_hwframe_get_buffer(hwframes_ctx_, avframe, 0); res < 0)
    {
        if (!reuse)
            av_frame_free(&avframe);
        return ffi::Fail(ffi::kErr, fmt::format("failed to get buffer: {}", av_err2str(res)));
    }

    if (reuse)
    {
        (*reuse)->ResetDisposeState();
        return (*reuse)->GetThisHandle(isolate);
    }
    return ffi::JSObject::New<Frame>(isolate, avframe);
}

ffi::Ret<bool> HWFramesContext::equalTo(ffi::Class<HWFramesContext> other)
{
    return other->hwframes_ctx_->data == hwframes_ctx_->data;
}

ffi::RetLocal<v8::Value> HWFramesContext::queryTransferFormats()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    AVPixelFormat *src_formats = nullptr;
    int res = av_hwframe_transfer_get_formats(
            hwframes_ctx_, AV_HWFRAME_TRANSFER_DIRECTION_TO, &src_formats, 0);
    if (res < 0)
        return ffi::Fail(ffi::kErr, fmt::format("failed to query src formats: {}", av_err2str(res)));

    std::vector<v8::Local<v8::Value>> src_formats_jsvec;
    for (auto *ptr = src_formats; *ptr != AV_PIX_FMT_NONE; ptr++)
        src_formats_jsvec.emplace_back(v8::Int32::New(isolate, static_cast<int>(*ptr)));
    av_free(src_formats);

    AVPixelFormat *dst_formats = nullptr;
    res = av_hwframe_transfer_get_formats(
            hwframes_ctx_, AV_HWFRAME_TRANSFER_DIRECTION_FROM, &dst_formats, 0);
    if (res < 0)
        return ffi::Fail(ffi::kErr, fmt::format("failed to query dst formats: {}", av_err2str(res)));

    std::vector<v8::Local<v8::Value>> dst_formats_jsvec;
    for (auto *ptr = dst_formats; *ptr != AV_PIX_FMT_NONE; ptr++)
        dst_formats_jsvec.emplace_back(v8::Int32::New(isolate, static_cast<int>(*ptr)));
    av_free(dst_formats);

    return ffi::Cast<ffi::ObjectLiteralMap>::ToChecked(isolate, {
        { "src", v8::Array::New(isolate, src_formats_jsvec.data(), src_formats_jsvec.size()) },
        { "dst", v8::Array::New(isolate, dst_formats_jsvec.data(), dst_formats_jsvec.size()) }
    });
}

ffi::Ret<void> HWFramesContext::Transfer(ffi::Class<Frame> dst, ffi::Class<Frame> src)
{
    if (src->GetDisposeState() != DisposeState::kNot)
        return ffi::Fail(ffi::kErr, "src frame has been disposed");
    if (dst->GetDisposeState() != DisposeState::kNot && !dst->IsReusable())
        return ffi::Fail(ffi::kErr, "dst frame is neither allocated nor reusable");

    bool dst_unallocated = dst->IsReusable();
    int res = av_hwframe_transfer_data(dst->GetAVFrame(), src->GetAVFrame(), 0);
    if (res < 0)
        return ffi::Fail(ffi::kErr, fmt::format("failed to transfer data: {}", av_err2str(res)));

    if (dst_unallocated)
        dst->ResetDisposeState();
    return {};
}

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
