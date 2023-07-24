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

#ifndef COCOA_UTAU_AVGENERICBUFFER_H
#define COCOA_UTAU_AVGENERICBUFFER_H

#include "Utau/Utau.h"
UTAU_NAMESPACE_BEGIN

/**
 * AVGenericBuffer is a wrapper class of FFmpeg's `AVFrame` object.
 * The underlying `AVFrame` object can be accessed by an opaque handle
 * (type-erased pointer), which is designed to be a private API.
 * Users should never try to operate the underlying `AVFrame` object directly.
 */
class AVGenericBuffer
{
public:
    using UnderlyingPtr = void*;
    struct BufferPriv;

    AVGenericBuffer(const AVGenericBuffer& clone);
    AVGenericBuffer(AVGenericBuffer&& other) noexcept;
    ~AVGenericBuffer();

    g_private_api g_nodiscard UnderlyingPtr GetUnderlyingPointer() const;

    template<typename T>
    g_private_api g_nodiscard g_inline T *CastUnderlyingPointer() const {
        return reinterpret_cast<T*>(GetUnderlyingPointer());
    }

    g_private_api g_nodiscard UnderlyingPtr CloneUnderlyingBuffer();

    template<typename T>
    g_private_api g_nodiscard g_inline T *CloneTypedUnderlyingBuffer() {
        return reinterpret_cast<T*>(CloneUnderlyingBuffer());
    }

    g_nodiscard size_t ComputeApproximateSizeInBytes();

    g_nodiscard int64_t GetPresentationTimestamp();
    g_nodiscard int64_t GetDuration();

protected:
    /**
     * The `AVGenericBuffer` only can be created from a valid `AVFrame`
     * object by subclasses. It stores and owns an `AVFrame`
     * object which is cloned from `ptr`.
     */
    explicit AVGenericBuffer(UnderlyingPtr ptr);

private:
    std::unique_ptr<BufferPriv> priv_;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_AVGENERICBUFFER_H
