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

#include "Core/Errors.h"
#include "Utau/AVGenericBuffer.h"
#include "Utau/ffwrappers/libavutil.h"
UTAU_NAMESPACE_BEGIN

struct AVGenericBuffer::BufferPriv
{
    ~BufferPriv() {
        if (frame)
        {
            av_frame_free(&frame);
        }
    }
    AVFrame *frame = nullptr;
};

AVGenericBuffer::~AVGenericBuffer() = default;

AVGenericBuffer::AVGenericBuffer(UnderlyingPtr ptr)
    : priv_(std::make_unique<BufferPriv>())
{
    CHECK(ptr && "Invalid underlying pointer: NULL");

    auto *frame = reinterpret_cast<AVFrame*>(ptr);
    priv_->frame = av_frame_clone(frame);
    CHECK(priv_->frame && "Failed to clone AVFrame");
}

AVGenericBuffer::AVGenericBuffer(const AVGenericBuffer& clone)
    : priv_(std::make_unique<BufferPriv>())
{
    CHECK(clone.priv_ && "Invalid AVGenericBuffer to be copied");
    CHECK(clone.priv_->frame && "Invalid underlying pointer: NULL");

    priv_->frame = av_frame_clone(clone.priv_->frame);
    CHECK(priv_->frame && "Failed to clone AVFrame");
}

AVGenericBuffer::AVGenericBuffer(AVGenericBuffer&& other) noexcept
    : priv_(std::move(other.priv_))
{
    CHECK(other.priv_ && "Invalid AVGenericBuffer to be moved");
    CHECK(other.priv_->frame && "Invalid underlying pointer: NULL");
}

AVGenericBuffer::UnderlyingPtr AVGenericBuffer::GetUnderlyingPointer() const
{
    CHECK(priv_ && priv_->frame);
    return priv_->frame;
}

AVGenericBuffer::UnderlyingPtr AVGenericBuffer::CloneUnderlyingBuffer()
{
    CHECK(priv_ && priv_->frame);

    AVFrame *ret = av_frame_clone(priv_->frame);
    CHECK(ret && "Failed to clone AVFrame");

    return ret;
}

size_t AVGenericBuffer::ComputeApproximateSizeInBytes()
{
    CHECK(priv_ && priv_->frame);

    size_t total_size = 0;
    for (AVBufferRef *buf : priv_->frame->buf)
    {
        if (!buf)
            break;
        total_size += buf->size;
    }
    return total_size;
}

UTAU_NAMESPACE_END
