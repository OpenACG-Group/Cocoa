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

GALLIUM_BINDINGS_UTAU_NS_END
