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

#include "Gallium/ffi/Context.h"
#include "Gallium/ffi/EnumRegistry.h"
#include "Gallium/ffi/ClassRegistry.h"
#include "Gallium/ffi/InterfaceRegistry.h"
#include "Gallium/ffi/Module.h"
#include "Gallium/RuntimeBase.h"
GALLIUM_FFI_NS_BEGIN

GlobalValueStorage::GlobalValueStorage(v8::Isolate *isolate)
    : isolate_(isolate)
{
}

void GlobalValueStorage::Store(uint64_t id, Local<v8::Value> value)
{
    CHECK(!value.IsEmpty());
    map_[id].Reset(isolate_, value);
}

Local<Value> GlobalValueStorage::Load(uint64_t id)
{
    auto itr = map_.find(id);
    if (itr == map_.end())
        return {};
    return itr->second.Get(isolate_);
}

bool GlobalValueStorage::Sweep(uint64_t id)
{
    return map_.erase(id);
}

TypeContext *TypeContext::FromIsolate(Isolate *isolate)
{
    return RuntimeBase::FromIsolate(isolate)->GetFFIContext();
}

std::unique_ptr<TypeContext> TypeContext::Make(Isolate *isolate)
{
    return std::make_unique<TypeContext>(isolate);
}

TypeContext::TypeContext(Isolate *isolate)
    : enum_registry_(std::make_unique<EnumRegistry>(isolate))
    , class_registry_(std::make_unique<ClassRegistry>(isolate))
    , iface_registry_(std::make_unique<InterfaceRegistry>(isolate))
    , module_registry_(std::make_unique<ModuleRegistry>())
    , global_storage_(std::make_unique<GlobalValueStorage>(isolate))
{
}

void TypeContext::Dispose()
{
    module_registry_.reset();
    class_registry_->Dispose();
    class_registry_.reset();
    iface_registry_.reset();
    enum_registry_.reset();
    global_storage_.reset();
}

GALLIUM_FFI_NS_END
