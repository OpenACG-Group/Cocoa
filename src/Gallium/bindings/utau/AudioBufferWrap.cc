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

#include <cstring>

#include "Gallium/bindings/utau/Exports.h"
#include "Utau/AudioBuffer.h"
GALLIUM_BINDINGS_UTAU_NS_BEGIN

AudioBufferWrap::AudioBufferWrap(std::shared_ptr<utau::AudioBuffer> buffer)
    : approximate_size_(0)
    , buffer_(std::move(buffer))
{
    if (buffer_)
        approximate_size_ = buffer_->ComputeApproximateSizeInBytes();

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    isolate->AdjustAmountOfExternalAllocatedMemory(
            static_cast<int64_t>(approximate_size_));
}

AudioBufferWrap::~AudioBufferWrap()
{
    dispose();
}

void AudioBufferWrap::dispose()
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

int32_t AudioBufferWrap::read(int32_t plane, int32_t sample_count, int32_t sample_offset,
                              size_t dst_bytes_offset, v8::Local<v8::Value> dst)
{
    const auto& info = buffer_->GetInfo();

    // Non-planar formats only have 1 buffer
    int num_planes = 1;
    if (info.IsPlanarFormat())
        num_planes = info.GetPlanesCount();

    if (plane < 0 || plane >= num_planes)
        g_throw(RangeError, "Invalid plane number to read");

    // `GetSamplesCount()` gives us the number of samples per channel
    int total_samples = info.GetSamplesCount();
    if (!info.IsPlanarFormat())
    {
        // For interleaved formats, the total number of samples is:
        // `samples_per_channel * num_channels`
        total_samples *= info.GetChannels();
    }

    int32_t avail_samples = total_samples - sample_offset;
    if (avail_samples < 0)
        g_throw(RangeError, "Invalid sample offset to read");
    if (avail_samples == 0)
        return 0;

    int32_t read_samples = std::min(sample_count, avail_samples);

    if (!binder::IsSome<v8::ArrayBuffer>(dst))
        g_throw(TypeError, "Argument `dst` must be an ArrayBuffer");

    auto dstbuf = dst.As<v8::ArrayBuffer>();

    int per_sample_size = utau::GetPerSampleSize(info.GetSampleFormat());
    int size = read_samples * per_sample_size;
    if (dstbuf->ByteLength() - dst_bytes_offset < size)
        g_throw(Error, "`dst` buffer is not big enough");

    auto *dstptr = reinterpret_cast<uint8_t*>(dstbuf->Data()) + dst_bytes_offset;
    auto *srcptr = buffer_->GetAddress(plane) + sample_offset * per_sample_size;

    std::memcpy(dstptr, srcptr, size);

    return read_samples;
}

namespace {

template<typename T>
void copy_interleaved_channel(const uint8_t *src, uint8_t *dst, int32_t ch,
                              int32_t size_per_sample, int32_t num_channels,
                              int32_t src_sample_offset, int32_t sample_count)
{
    CHECK(src && dst);
    CHECK(size_per_sample == sizeof(T));
    CHECK(num_channels > 1);
    CHECK(ch >= 0 && ch < num_channels);

    const auto *sptr = reinterpret_cast<const T*>(src);
    sptr += num_channels * src_sample_offset;

    auto *dptr = reinterpret_cast<T*>(dst);

    for (int32_t i = 0; i < sample_count; i++)
        dptr[i] = sptr[i * num_channels + ch];
}

} // namespace anonymous

int32_t AudioBufferWrap::readChannel(int32_t ch, int32_t sample_count, int32_t sample_offset,
                                     size_t dst_bytes_offset, v8::Local<v8::Value> dst)
{
    const auto& info = buffer_->GetInfo();

    if (ch < 0 || ch >= info.GetChannels())
        g_throw(RangeError, "Invalid channel index");

    // For planar formats, channel datas are stored in separated buffers
    // and can be read directly by plane index.
    if (info.IsPlanarFormat() || info.GetChannels() == 1)
        return read(ch, sample_count, sample_offset, dst_bytes_offset, dst);

    int read_samples = std::min(info.GetSamplesCount() - sample_offset, sample_count);

    int per_sample_size = utau::GetPerSampleSize(info.GetSampleFormat());
    size_t min_arraybuf_avail_size = per_sample_size * read_samples;

    // For non-planar (interleaved) formats, we should separate
    // channel datas manually.
    if (!binder::IsSome<v8::ArrayBuffer>(dst))
        g_throw(TypeError, "Argument `dst` must be an ArrayBuffer");

    auto arraybuf = dst.As<v8::ArrayBuffer>();
    if (arraybuf->ByteLength() - dst_bytes_offset < min_arraybuf_avail_size)
        g_throw(Error, "Destination buffer is not big enough");

    auto *dstptr = reinterpret_cast<uint8_t*>(arraybuf->Data()) + dst_bytes_offset;
    auto *srcptr = buffer_->GetAddress(0);

#define COPY_SAMPLES(t)                                                              \
        copy_interleaved_channel<t>(srcptr, dstptr, ch, per_sample_size,             \
                                    info.GetChannels(), sample_offset, read_samples)

    switch (info.GetSampleFormat())
    {
    case utau::SampleFormat::kU8:
        COPY_SAMPLES(uint8_t);
        break;
    case utau::SampleFormat::kS16:
        COPY_SAMPLES(int16_t);
        break;
    case utau::SampleFormat::kS32:
    case utau::SampleFormat::kF32:
        COPY_SAMPLES(int32_t);
        break;
    case utau::SampleFormat::kF64:
        COPY_SAMPLES(int64_t);
        break;
    default:
        MARK_UNREACHABLE();
    }

#undef COPY_SAMPLES

    return read_samples;
}

v8::Local<v8::Value> AudioBufferWrap::clone()
{
    if (!buffer_)
        g_throw(Error, "Disposed audio buffer");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::NewObject<AudioBufferWrap>(isolate, buffer_);
}

GALLIUM_BINDINGS_UTAU_NS_END
