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

#ifndef COCOA_GALLIUM_FFI_CONTEXT_H
#define COCOA_GALLIUM_FFI_CONTEXT_H

#include "Gallium/Gallium.h"
#include "Gallium/ffi/DataTypes.h"
GALLIUM_FFI_NS_BEGIN

class EnumRegistry;
class ClassRegistry;
class InterfaceRegistry;
class ModuleRegistry;

#define FFI_GVSTORE_DECLARE_ID(name) extern bool kGVStoreID_##name;
#define FFI_GVSTORE_DEFINE_ID(name) bool kGVStoreID_##name = true;
#define FFI_GVSTORE_USE_ID(name) reinterpret_cast<uint64_t>(&kGVStoreID_##name)

/**
 * Store a global JavaScript value identified by a globally unique ID.
 * To make sure the ID the globally unique, the address of a global
 * variable can be used as the ID. The storage will not be shared among
 * different Isolates.
 */
class GlobalValueStorage
{
public:
    explicit GlobalValueStorage(v8::Isolate *isolate);

    void Store(uint64_t id, Local<Value> value);
    Local<Value> Load(uint64_t id);

    bool Sweep(uint64_t id);

private:
    v8::Isolate *isolate_;
    std::map<uint64_t, Global<Value>> map_;
};

class TypeContext
{
public:
    static std::unique_ptr<TypeContext> Make(Isolate *isolate);

    static TypeContext *FromIsolate(Isolate *isolate);

    explicit TypeContext(Isolate *isolate);
    ~TypeContext() = default;

    void Dispose();

    g_nodiscard EnumRegistry *GetEnumRegistry() const {
        CHECK(enum_registry_);
        return enum_registry_.get();
    }

    g_nodiscard ClassRegistry *GetClassRegistry() const {
        CHECK(class_registry_);
        return class_registry_.get();
    }

    g_nodiscard InterfaceRegistry *GetInterfaceRegistry() const {
        CHECK(iface_registry_);
        return iface_registry_.get();
    }

    g_nodiscard ModuleRegistry *GetModuleRegistry() const {
        CHECK(module_registry_);
        return module_registry_.get();
    }

    g_nodiscard GlobalValueStorage *GetValueStorage() const {
        CHECK(global_storage_);
        return global_storage_.get();
    }

private:
    std::unique_ptr<EnumRegistry> enum_registry_;
    std::unique_ptr<ClassRegistry> class_registry_;
    std::unique_ptr<InterfaceRegistry> iface_registry_;
    std::unique_ptr<ModuleRegistry> module_registry_;
    std::unique_ptr<GlobalValueStorage> global_storage_;
};

GALLIUM_FFI_NS_END
#endif //COCOA_GALLIUM_FFI_CONTEXT_H
