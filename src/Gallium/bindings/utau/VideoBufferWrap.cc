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

#include "libavutil/intreadwrite.h"
#include "Utau/ffwrappers/libswscale.h"

#include "fmt/format.h"

#include "Core/Exception.h"
#include "Core/EventLoop.h"
#include "Gallium/bindings/utau/Exports.h"
#include "Utau/VideoBuffer.h"
GALLIUM_BINDINGS_UTAU_NS_BEGIN

VideoBufferWrap::VideoBufferWrap(std::shared_ptr<utau::VideoBuffer> buffer)
    : approximate_size_(0)
    , buffer_(std::move(buffer))
{
    if (buffer_)
        approximate_size_ = buffer_->ComputeApproximateSizeInBytes();

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    isolate->AdjustAmountOfExternalAllocatedMemory(
            static_cast<int64_t>(approximate_size_));
}

VideoBufferWrap::~VideoBufferWrap()
{
    dispose();
}

void VideoBufferWrap::dispose()
{
    if (!buffer_)
        return;

    buffer_.reset();

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (isolate)
    {
        isolate->AdjustAmountOfExternalAllocatedMemory(
                -static_cast<int64_t>(approximate_size_));
    }
}

v8::Local<v8::Value> VideoBufferWrap::clone()
{
    if (!buffer_)
        g_throw(Error, "Buffer reference is disposed");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::NewObject<VideoBufferWrap>(isolate, buffer_);
}

v8::Local<v8::Value> VideoBufferWrap::getStrides()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    int nb_planes = buffer_->GetInfo().GetColorInfo().GetPlanesCount();
    std::vector<v8::Local<v8::Value>> res(nb_planes);
    for (int32_t p = 0; p < nb_planes; p++)
        res[p] = v8::Int32::New(isolate, buffer_->GetInfo().GetStride(p));
    return binder::to_v8(isolate, res);
}

namespace {

template<typename T>
void read_image_component_line(T *dst,
                               AVFrame *frame,
                               const AVPixFmtDescriptor *desc,
                               int32_t x,
                               int32_t y,
                               int32_t component,
                               int32_t width)
{
    CHECK(component >= 0 && component < desc->nb_components);
    const AVComponentDescriptor& comp = desc->comp[component];

    int plane = comp.plane;
    int depth = comp.depth;
    uint32_t mask = (1ULL << depth) - 1;
    int shift = comp.shift;
    int step = comp.step;

    bool has_palette = desc->flags & AV_PIX_FMT_FLAG_PAL;

    if (desc->flags & AV_PIX_FMT_FLAG_BITSTREAM)
    {
        if (depth == 10)
        {
            // Assume all channels are packed into a 32bit value
            const uint8_t *byte_p = frame->data[plane] + y * frame->linesize[plane];
            const auto *p = reinterpret_cast<const uint32_t*>(byte_p);

            while (width--)
            {
                uint32_t val = AV_RB32(p);
                val = (val >> comp.offset) & mask;
                if (has_palette)
                    val = frame->data[1][4 * val + component];
                *dst++ = val;
                p++;
            }
        }
        else
        {
            int skip = x * step + comp.offset;
            const uint8_t *p = frame->data[plane] + y * frame->linesize[plane] + (skip >> 3);
            shift = 8 - depth - (skip & 7);

            while (width--)
            {
                uint32_t val = (*p >> shift) & mask;
                if (has_palette)
                    val = frame->data[1][4 * val + component];
                shift -= step;
                p -= shift >> 3;
                shift &= 7;
                *dst++ = val;
            }
        }
    }
    else
    {
        const uint8_t *p = frame->data[plane] + y * frame->linesize[plane]
                         + x * step + comp.offset;
        bool is_8bit  = shift + depth <= 8;
        bool is_16bit = shift + depth <= 16;

        if (is_8bit)
            p += !!(desc->flags & AV_PIX_FMT_FLAG_BE);

        while (width--)
        {
            uint32_t val;
            if (is_8bit)
                val = *p;
            else if (is_16bit)
                val = desc->flags & AV_PIX_FMT_FLAG_BE ? AV_RB16(p) : AV_RL16(p);
            else
                val = desc->flags & AV_PIX_FMT_FLAG_BE ? AV_RB32(p) : AV_RL32(p);
            val = (val >> shift) & mask;
            if (has_palette)
                val = frame->data[1][4 * val + component];
            p += step;
            *dst++ = val;
        }
    }
}

template<typename T>
void read_image_component(uint8_t *dst_u8_ptr,
                          AVFrame *frame,
                          const AVPixFmtDescriptor *desc,
                          int32_t component,
                          int32_t slice_w,
                          int32_t slice_h,
                          int32_t src_x,
                          int32_t src_y,
                          int32_t dst_stride_in_elements)
{
    auto *dst = reinterpret_cast<T*>(dst_u8_ptr);
    for (int32_t y = src_y; y < src_y + slice_h; y++)
    {
        read_image_component_line(dst, frame, desc, src_x, y, component, slice_w);
        dst += dst_stride_in_elements;
    }
}

v8::Local<v8::Value> read_component_impl(const std::shared_ptr<utau::VideoBuffer>& buffer,
                                         int32_t component,
                                         v8::Local<v8::Value> dst,
                                         int32_t slice_w,
                                         int32_t slice_h,
                                         int32_t src_x,
                                         int32_t src_y,
                                         int32_t dst_stride_in_elements,
                                         bool async)
{
    const utau::VideoBufferInfo& buf_info = buffer->GetInfo();
    const utau::VideoColorInfo& color_info = buf_info.GetColorInfo();
    if (color_info.FormatIsHWAccel())
        g_throw(Error, "Hardware frame cannot be read directly. Use data transfer functions instead");

    const AVPixFmtDescriptor *fmtdesc = av_pix_fmt_desc_get(color_info.GetFormat());
    CHECK(fmtdesc);

    if (src_x < 0 || slice_w < 0 || src_x + slice_w > buf_info.GetWidth() ||
        src_y < 0 || slice_h < 0 || src_y + slice_h > buf_info.GetHeight())
    {
        g_throw(RangeError, "Invalid slice specifiers (srcX, srcY, sliceW, sliceH)");
    }

    if (slice_w == 0 || slice_h == 0)
        return v8::Undefined(v8::Isolate::GetCurrent());

    int comp_idx = -1;
    switch (static_cast<ComponentSelector>(component))
    {
        case ComponentSelector::kLuma:
            if (!(fmtdesc->flags & AV_PIX_FMT_FLAG_RGB))
            {
                // If the RGB flag is not set, then luma is always 0.
                comp_idx = 0;
            }
            break;

        case ComponentSelector::kChromaU:
        case ComponentSelector::kChromaV:
            if (!(fmtdesc->flags & AV_PIX_FMT_FLAG_RGB) && fmtdesc->nb_components >= 3)
            {
                // comp_idx = 1,  if is kChromaU
                //          = 2,  if is kChromaV
                comp_idx = component - static_cast<int>(ComponentSelector::kChromaU) + 1;
            }
            break;

        case ComponentSelector::kR:
        case ComponentSelector::kG:
        case ComponentSelector::kB:
            if ((fmtdesc->flags & AV_PIX_FMT_FLAG_RGB) && fmtdesc->nb_components >= 3)
            {
                // comp_idx = 0,  if is kR
                //            1,  if is kG
                //            2,  if is kB
                comp_idx = component - static_cast<int>(ComponentSelector::kR);
            }
            break;

        case ComponentSelector::kAlpha:
            if (fmtdesc->flags & AV_PIX_FMT_FLAG_ALPHA)
            {
                // If present, alpha is always the last component
                comp_idx = fmtdesc->nb_components - 1;
            }
            break;
    }
    if (comp_idx < 0)
        g_throw(Error, "Invalid component selector");

    const AVComponentDescriptor& comp = fmtdesc->comp[comp_idx];

    if (!dst->IsTypedArray() || !dst.As<v8::TypedArray>()->HasBuffer())
        g_throw(TypeError, "Argument `dst` must be an allocated TypedArray");
    auto dst_typed_arr = dst.As<v8::TypedArray>();

    if (dst_typed_arr->Length() < dst_stride_in_elements * slice_h)
        g_throw(Error, "TypedArray is not big enough");

    // The number of bits of each element in TypedArray.
    // Although this can be computed by
    // `dst_typed_arr->ByteLength() / dst_typed_arr->Length() * 8`,
    // these if statements are used to make sure user provides
    // an UNSIGNED TypedArray.
    int dst_arr_depth;
    if (dst->IsUint8Array())
        dst_arr_depth = 8;
    else if (dst->IsUint16Array())
        dst_arr_depth = 16;
    else if (dst->IsUint32Array() || dst->IsFloat32Array())
        dst_arr_depth = 32;
    else
        g_throw(TypeError, "Argument `dst` must be a Uint{8,16,32}Array or Float32Array");

    if (dst_arr_depth < comp.depth)
    {
        g_throw(TypeError, fmt::format(
                "Uint{}Array cannot store the pixel component with depth {}",
                dst_arr_depth, comp.depth));
    }

    auto *dst_u8_ptr = reinterpret_cast<uint8_t*>(dst_typed_arr->Buffer()->Data())
                       + dst_typed_arr->ByteOffset();

#define READ_IMAGE(type)                            \
    read_image_component<type>(                     \
        dst_u8_ptr,                                 \
        buffer->CastUnderlyingPointer<AVFrame>(),   \
        fmtdesc,                                    \
        comp_idx,                                   \
        slice_w,                                    \
        slice_h,                                    \
        src_x,                                      \
        src_y,                                      \
        dst_stride_in_elements)

    std::function<void(void)> read_image_func;

#define READ_IMG_CAPTURES \
    dst_owned = dst_typed_arr->Buffer()->GetBackingStore(), \
    dst_u8_ptr, buffer, fmtdesc, comp_idx, slice_w, slice_h, \
    src_x, src_y, dst_stride_in_elements

    if (dst_arr_depth == 8)
    {
        read_image_func = [READ_IMG_CAPTURES]() mutable {
            READ_IMAGE(uint8_t);
            dst_owned.reset();
        };
    }
    else if (dst_arr_depth == 16)
    {
        read_image_func = [READ_IMG_CAPTURES]() mutable {
            READ_IMAGE(uint16_t);
            dst_owned.reset();
        };
    }
    else if (dst_arr_depth == 32)
    {
        read_image_func = [READ_IMG_CAPTURES]() mutable {
            READ_IMAGE(uint32_t);
            dst_owned.reset();
        };
    }
    else
    {
        MARK_UNREACHABLE();
    }

#undef READ_IMG_CAPTURES
#undef READ_IMAGE

    // Synchronous mode
    if (!async)
    {
        read_image_func();
        return v8::Undefined(v8::Isolate::GetCurrent());
    }

    // Asynchronous mode
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    auto resolver = v8::Promise::Resolver::New(ctx).ToLocalChecked();
    auto global_resolver_sp = std::make_shared<v8::Global<v8::Promise::Resolver>>(
            isolate, resolver);

    EventLoop::Ref().enqueueThreadPoolTrivialTask(
            read_image_func,
            [global_resolver_sp, isolate]() {
                v8::HandleScope scope(isolate);
                auto r = global_resolver_sp->Get(isolate);
                r->Resolve(isolate->GetCurrentContext(), v8::Undefined(isolate)).ToChecked();
            }
    );

    return resolver->GetPromise();
}

} // namespace anonymous

void VideoBufferWrap::readComponent(int32_t component,
                                    v8::Local<v8::Value> dst,
                                    int32_t slice_w,
                                    int32_t slice_h,
                                    int32_t src_x,
                                    int32_t src_y,
                                    int32_t dst_stride_in_elements)
{
    read_component_impl(buffer_,
                        component,
                        dst,
                        slice_w,
                        slice_h,
                        src_x,
                        src_y,
                        dst_stride_in_elements,
                        false);
}

v8::Local<v8::Value>
VideoBufferWrap::readComponentAsync(int32_t component,
                                    v8::Local<v8::Value> dst,
                                    int32_t slice_w,
                                    int32_t slice_h,
                                    int32_t src_x,
                                    int32_t src_y,
                                    int32_t dst_stride_in_elements)
{
    return read_component_impl(buffer_,
                               component,
                               dst,
                               slice_w,
                               slice_h,
                               src_x,
                               src_y,
                               dst_stride_in_elements,
                               true);
}

void VideoBufferWrap::readGrayscale(v8::Local<v8::Value> dst,
                                    int32_t slice_w,
                                    int32_t slice_h,
                                    int32_t src_x,
                                    int32_t src_y,
                                    int32_t dst_stride)
{
    if (!buffer_)
        g_throw(Error, "Disposed video buffer");

    if (!dst->IsUint8Array() || !dst.As<v8::Uint8Array>()->HasBuffer())
        g_throw(TypeError, "Argument `dst` must be an allocate Uint8Array");
    auto u8_array = dst.As<v8::Uint8Array>();

    auto *frame = buffer_->CastUnderlyingPointer<AVFrame>();
    if (slice_w <= 0 || slice_h <= 0 || src_x < 0 || src_y < 0 ||
        src_x + slice_w > frame->width || src_y + slice_h > frame->height)
    {
        g_throw(RangeError, "Slice specifier quadruple (sliceW, sliceH, srcX, srcY) is out of range");
    }

    if (u8_array->ByteLength() < slice_h * dst_stride)
        g_throw(Error, "Destination buffer is not big enough");

    // Do frame cropping before converting it into grayscale.
    // To remain the original frame unchanged, it should be cloned
    // before cropping operation.
    frame = av_frame_clone(frame);
    frame->crop_left = src_x;
    frame->crop_top = src_y;
    frame->crop_right = src_x + slice_w;
    frame->crop_bottom = src_y + slice_h;
    av_frame_apply_cropping(frame, 0);

    ScopeExitAutoInvoker frame_releaser([&frame] {
        av_frame_free(&frame);
    });

    // Prepare for conversion
    SwsContext *swsctx = sws_getContext(slice_w,
                                        slice_h,
                                        static_cast<AVPixelFormat>(frame->format),
                                        slice_w,
                                        slice_h,
                                        AV_PIX_FMT_GRAY8,
                                        0,
                                        nullptr,
                                        nullptr,
                                        nullptr);
    CHECK(swsctx && "Failed to create swscale context");

    ScopeExitAutoInvoker swsctx_releaser([swsctx] {
        sws_freeContext(swsctx);
    });

    // Do color conversion
    AVFrame *dst_frame = av_frame_alloc();
    CHECK(dst_frame && "Failed to allocate frame");
    ScopeExitAutoInvoker dst_frame_releaser([&dst_frame] {
        av_frame_free(&dst_frame);
    });

    int ret = sws_scale_frame(swsctx, dst_frame, frame);
    if (ret < 0)
    {
        char err[512];
        av_strerror(ret, err, sizeof(err));
        g_throw(Error, fmt::format("Failed to convert format: {}", err));
    }

    // Copy result
    auto *dst_ptr = reinterpret_cast<uint8_t*>(u8_array->Buffer()->Data())
                    + u8_array->ByteOffset();
    for (int32_t y = 0; y < slice_h; y++)
    {
        std::memcpy(dst_ptr, dst_frame->data[0] + y * dst_frame->linesize[0], slice_w);
        dst_ptr += dst_stride;
    }
}

v8::Local<v8::Value> VideoBufferWrap::transferHardwareFrameDataTo(int32_t expect_format)
{
    auto *this_frame = buffer_->CastUnderlyingPointer<AVFrame>();
    if (!this_frame->hw_frames_ctx)
        g_throw(Error, "Not a hardware frame");

    AVPixelFormat *formats;
    int ret = av_hwframe_transfer_get_formats(this_frame->hw_frames_ctx,
                                              AV_HWFRAME_TRANSFER_DIRECTION_FROM,
                                              &formats,
                                              0);
    if (ret)
        g_throw(Error, "Failed to query available pixel formats");

    ScopeExitAutoInvoker format_releaser([formats] {
        av_free(formats);
    });

    if (expect_format != AV_PIX_FMT_NONE)
    {
        bool found_format = false;
        while (*formats != AV_PIX_FMT_NONE)
        {
            if (*formats == expect_format)
            {
                found_format = true;
                break;
            }
            formats++;
        }
        if (!found_format)
            g_throw(Error, "Unsupported destination format");
    }

    AVFrame *dst = av_frame_alloc();
    if (expect_format != AV_PIX_FMT_NONE)
        dst->format = expect_format;

    ret = av_hwframe_transfer_data(dst, this_frame, 0);
    if (ret)
    {
        char err[512];
        av_strerror(ret, err, sizeof(err));
        g_throw(Error, fmt::format("Failed to transfer data: {}", err));
    }

    std::shared_ptr<utau::VideoBuffer> buffer = utau::VideoBuffer::MakeFromAVFrame(dst);
    CHECK(buffer);

    return binder::NewObject<VideoBufferWrap>(v8::Isolate::GetCurrent(), buffer);
}

v8::Local<v8::Value> VideoBufferWrap::queryHardwareTransferableFormats()
{
    auto *this_frame = buffer_->CastUnderlyingPointer<AVFrame>();
    if (!this_frame->hw_frames_ctx)
        g_throw(Error, "Not a hardware frame");

    AVPixelFormat *formats;
    int ret = av_hwframe_transfer_get_formats(this_frame->hw_frames_ctx,
                                              AV_HWFRAME_TRANSFER_DIRECTION_FROM,
                                              &formats,
                                              0);
    if (ret)
    {
        char err[512];
        av_strerror(ret, err, sizeof(err));
        g_throw(Error, fmt::format("Failed to query available pixel formats: {}", err));
    }
    ScopeExitAutoInvoker format_releaser([formats] {
        av_free(formats);
    });

    int cnt = 0;
    while (formats[cnt] != AV_PIX_FMT_NONE)
        cnt++;

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (cnt == 0)
        return v8::Array::New(isolate, 0);
    return binder::to_v8(isolate, std::vector<AVPixelFormat>(formats, formats + cnt));
}

GALLIUM_BINDINGS_UTAU_NS_END
