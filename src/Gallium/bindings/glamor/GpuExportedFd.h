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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_GPUEXPORTEDFD_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_GPUEXPORTEDFD_H

#include "include/v8.h"
#include <vulkan/vulkan_core.h>

#include "Gallium/bindings/ExportableObjectBase.h"
#include "Gallium/bindings/glamor/Types.h"
#include "Glamor/SkiaGpuContextOwner.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

//! TSDecl: class GpuExportedFd
class GpuExportedFd : public ExportableObjectBase
{
public:
    enum class FdPayloadType
    {
        kSemaphore,
        kSkSurface
    };

    using SkSurfacePayload = gl::SkiaGpuContextOwner::ExportedSkSurfaceInfo;

    using PayloadData = std::unique_ptr<uint8_t[]>;

    GpuExportedFd(int32_t fd, FdPayloadType type, PayloadData payload_data = nullptr);
    ~GpuExportedFd();

    g_nodiscard FdPayloadType GetPayloadType() const {
        return payload_type_;
    }

    template<typename T>
    g_nodiscard const T *GetPayload() const {
        CHECK(payload_data_ && "Empty payload data");
        return reinterpret_cast<T*>(payload_data_.get());
    }

    int32_t CheckAndTakeDescriptor();

    //! TSDecl: function close(): void
    void close();

    //! TSDecl: function isImportedOrClosed(): boolean
    bool isImportedOrClosed() const;

private:
    static MaybeFlattened Serialize(v8::Isolate *isolate, ExportableObjectBase *base, bool pretest);

    int32_t                 fd_;
    FdPayloadType           payload_type_;
    PayloadData             payload_data_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_GPUEXPORTEDFD_H
