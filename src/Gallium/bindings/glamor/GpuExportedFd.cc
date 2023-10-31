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

#include <unistd.h>

#include "Gallium/binder/ThrowExcept.h"
#include "Gallium/binder/Class.h"
#include "Gallium/bindings/glamor/GpuExportedFd.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

namespace {

class TransferData : public ExportableObjectBase::FlattenedData
{
public:
    TransferData(int32_t fd, GpuExportedFd::FdPayloadType type,
                 GpuExportedFd::PayloadData data)
                 : fd_(fd), type_(type), data_(std::move(data)) {}
    ~TransferData() override = default;

    v8::MaybeLocal<v8::Object> Deserialize(v8::Isolate *isolate,
                                           v8::Local<v8::Context> context) override
    {
        return binder::NewObject<GpuExportedFd>(isolate, fd_, type_, std::move(data_));
    }

private:
    int32_t fd_;
    GpuExportedFd::FdPayloadType type_;
    GpuExportedFd::PayloadData data_;
};

} // namespace anonymous

ExportableObjectBase::MaybeFlattened
GpuExportedFd::Serialize(v8::Isolate *isolate, ExportableObjectBase *base, bool pretest)
{
    auto *self = base->Cast<GpuExportedFd>();
    if (pretest)
        return FlattenPretestResult(!self->isImportedOrClosed());
    std::shared_ptr<FlattenedData> data = std::make_shared<TransferData>(
            self->CheckAndTakeDescriptor(), self->payload_type_, std::move(self->payload_data_));
    return v8::Just(data);
}

GpuExportedFd::GpuExportedFd(int32_t fd, FdPayloadType type, PayloadData payload_data)
    : ExportableObjectBase(kTransferable_Attr, {}, Serialize, nullptr)
    , fd_(fd)
    , payload_type_(type)
    , payload_data_(std::move(payload_data))
{
}

GpuExportedFd::~GpuExportedFd()
{
    if (fd_ >= 0)
        ::close(fd_);
}

void GpuExportedFd::close()
{
    if (fd_ < 0)
        g_throw(Error, "Exported file descriptor has been closed");
    ::close(fd_);
    fd_ = -1;
}

bool GpuExportedFd::isImportedOrClosed() const
{
    return (fd_ < 0);
}

int32_t GpuExportedFd::CheckAndTakeDescriptor()
{
    if (fd_ < 0)
        g_throw(Error, "Exported file descriptor has been closed");
    int32_t fd = fd_;
    fd_ = -1;
    return fd;
}

GALLIUM_BINDINGS_GLAMOR_NS_END
