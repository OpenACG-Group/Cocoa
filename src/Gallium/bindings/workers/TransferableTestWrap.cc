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

#include "Gallium/RuntimeBase.h"
#include "Gallium/bindings/workers/Exports.h"
GALLIUM_BINDINGS_WORKERS_NS_BEGIN

TransferableTestWrap::TransferableTestWrap(int32_t value)
    : ExportableObjectBase(kTransferable_Attr | kCloneable_Attr,
                           {}, TransferHandler, TransferHandler)
    , value_(value)
{
}

class TestTransferFlattenedData : public ExportableObjectBase::FlattenedData
{
public:
    explicit TestTransferFlattenedData(int32_t value) : value_(value) {}
    ~TestTransferFlattenedData() override = default;
    v8::MaybeLocal<v8::Object> Deserialize(v8::Isolate *isolate,
                                           v8::Local<v8::Context> context) override
    {
        return binder::NewObject<TransferableTestWrap>(isolate, value_);
    }

    int32_t value_;
};

v8::Maybe<std::shared_ptr<ExportableObjectBase::FlattenedData>>
TransferableTestWrap::TransferHandler(v8::Isolate *isolate,
                                      ExportableObjectBase *base,
                                      bool pretest)
{
    using RetT = std::shared_ptr<FlattenedData>;
    if (pretest)
        return v8::Just<RetT>(nullptr);

    return v8::Just<RetT>(std::make_shared<TestTransferFlattenedData>(
            base->Cast<TransferableTestWrap>()->value_));
}

GALLIUM_BINDINGS_WORKERS_NS_END
