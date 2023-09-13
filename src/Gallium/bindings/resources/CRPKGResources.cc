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

#include "Core/Errors.h"
#include "Core/EventLoop.h"
#include "CRPKG/VirtualDisk.h"
#include "Gallium/bindings/resources/Exports.h"
#include "Gallium/binder/ThrowExcept.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_RESOURCES_NS_BEGIN

size_t CRPKGStorageWrap::byteLength() const
{
    return storage_.size;
}

namespace {

struct AsyncReadContext
{
    v8::Isolate *isolate;
    v8::Global<v8::Promise::Resolver> resolver;
    std::shared_ptr<v8::BackingStore> store;
    uint8_t *dstptr;
    const uint8_t *srcptr;
    size_t size;
};

} // namespace anonymous

v8::Local<v8::Value> CRPKGStorageWrap::read(size_t src_offset, v8::Local<v8::Value> dst,
                                            size_t dst_offset, size_t size) const
{
    if (!storage_.addr)
        g_throw(Error, "Operate on unreferenced storage object");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!dst->IsUint8Array())
        g_throw(TypeError, "Argument `dst` must be a Uint8Array");

    v8::Local<v8::Uint8Array> array = dst.As<v8::Uint8Array>();
    if (dst_offset > array->ByteLength())
        g_throw(RangeError, "Invalid offset and size for `dst` buffer");

    if (src_offset > storage_.size)
        g_throw(RangeError, "Invalid offset and size for source buffer");

    size = std::min(size, array->ByteLength() - dst_offset);
    size = std::min(size, storage_.size - src_offset);

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    auto resolver = v8::Promise::Resolver::New(ctx).ToLocalChecked();

    if (size <= 0)
    {
        resolver->Resolve(ctx, v8::Number::New(isolate, 0)).ToChecked();
        return resolver->GetPromise();
    }

    uint8_t *dstptr = reinterpret_cast<uint8_t*>(array->Buffer()->Data())
                        + array->ByteOffset() + dst_offset;
    const uint8_t *srcptr = storage_.addr + src_offset;

    auto *async_read_ctx = new AsyncReadContext{
        .isolate = isolate,
        .resolver = v8::Global<v8::Promise::Resolver>(isolate, resolver),
        .store = array->Buffer()->GetBackingStore(),
        .dstptr = dstptr,
        .srcptr = srcptr,
        .size = size
    };

    EventLoop::GetCurrent()->enqueueThreadPoolTrivialTask(
            [async_read_ctx]() {
                std::memcpy(async_read_ctx->dstptr,
                            async_read_ctx->srcptr,
                            async_read_ctx->size);
            },
            [async_read_ctx]() {
                v8::Isolate *i = async_read_ctx->isolate;
                v8::HandleScope scope(i);
                v8::Local<v8::Promise::Resolver> r = async_read_ctx->resolver.Get(i);
                v8::Local<v8::Context> ctx = i->GetCurrentContext();

                r->Resolve(ctx, v8::Number::New(
                        i, static_cast<double>(async_read_ctx->size))).ToChecked();

                delete async_read_ctx;
            }
    );

    return resolver->GetPromise();
}

v8::Local<v8::Value> CRPKGStorageWrap::readSync(size_t src_offset,
                                                v8::Local<v8::Value> dst,
                                                size_t dst_offset,
                                                size_t size) const
{
    if (!storage_.addr)
        g_throw(Error, "Operate on unreferenced storage object");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!dst->IsUint8Array())
        g_throw(TypeError, "Argument `dst` must be a Uint8Array");

    v8::Local<v8::Uint8Array> array = dst.As<v8::Uint8Array>();
    if (!array->HasBuffer())
        g_throw(TypeError, "Argument `dst` must be an allocated Uint8Array");
    if (dst_offset > array->ByteLength())
        g_throw(RangeError, "Invalid offset and size for `dst` buffer");

    if (src_offset > storage_.size)
        g_throw(RangeError, "Invalid offset and size for source buffer");

    size = std::min(size, array->ByteLength() - dst_offset);
    size = std::min(size, storage_.size - src_offset);

    if (size <= 0)
        return v8::Number::New(isolate, 0);

    uint8_t *dstptr = reinterpret_cast<uint8_t*>(array->Buffer()->Data())
                      + array->ByteOffset() + dst_offset;
    const uint8_t *srcptr = storage_.addr + src_offset;

    std::memcpy(dstptr, srcptr, size);

    return v8::Number::New(isolate, static_cast<double>(size));
}

void CRPKGStorageWrap::unref()
{
    disk_.reset();
    storage_.size = 0;
    storage_.addr = nullptr;
}

namespace {

std::shared_ptr<Data> create_data_from_source(v8::Isolate *isolate,
                                              v8::Local<v8::Value> prop_type_val,
                                              v8::Local<v8::Value> prop_source_val)
{
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

    if (!prop_type_val->IsUint32())
        g_throw(TypeError, "Invalid value for property `type` of an object in `layers`");

    uint32_t type = prop_type_val->Int32Value(ctx).ToChecked();
    if (type > static_cast<uint32_t>(CRPKGSourceType::kLastEnum))
    {
        g_throw(RangeError, "Invalid enumeration value for property"
                            " `type` of an object in `layers`");
    }

    switch (static_cast<CRPKGSourceType>(type))
    {
    case CRPKGSourceType::kFilePath:
    {
        if (!prop_source_val->IsString())
        {
            g_throw(TypeError, "Invalid value for property "
                               "`source` of an object in `layers`");
        }
        v8::String::Utf8Value str(isolate, prop_source_val);

        // CRPKG requires that the data source of a CRPKG package must
        // have an accessible buffer (linear buffer), so the data must
        // be created from mapping a file instead of open it directly.
        // Note that this operation may fail on some filesystems which
        // does not support file mapping.
        return Data::MakeFromFileMapped(std::string(*str, str.length()),
                                        {vfs::OpenFlags::kReadonly});
    }

    case CRPKGSourceType::kCRPKGStorage:
    {
        CRPKGStorageWrap *wrap =
                binder::UnwrapObject<CRPKGStorageWrap>(isolate, prop_source_val);
        if (!wrap)
        {
            g_throw(TypeError, "Invalid value for property "
                               " `source` of an object in `layers`");
        }

        return Data::MakeFromExternal(
                const_cast<uint8_t*>(wrap->GetStorage().addr),
                wrap->GetStorage().size,
                [sp = wrap->GetDisk()](void *) mutable { sp.reset(); }
        );
    }

    case CRPKGSourceType::kUint8Array:
    {
        if (!prop_source_val->IsUint8Array())
        {
            g_throw(TypeError, "Invalid value for property "
                               " `source` of an object in `layers`");
        }

        v8::Local<v8::Uint8Array> arr = prop_source_val.As<v8::Uint8Array>();
        auto *ptr = reinterpret_cast<uint8_t*>(arr->Buffer()->Data()) + arr->ByteOffset();
        return Data::MakeFromExternal(
                ptr, arr->ByteLength(),
                [sp = arr->Buffer()->GetBackingStore()](void*) mutable { sp.reset(); }
        );
    }
    }

    MARK_UNREACHABLE();
}

} // namespace anonymous

v8::Local<v8::Value> CRPKGVirtualDiskWrap::MakeLayers(v8::Local<v8::Value> layers)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!layers->IsArray())
        g_throw(TypeError, "Argument `layers` is not an array of objects");

    auto layers_array = layers.As<v8::Array>();
    if (layers_array->Length() == 0)
        return v8::Null(isolate);

    auto keyname_type = v8::String::NewFromUtf8Literal(isolate, "type");
    auto keyname_source = v8::String::NewFromUtf8Literal(isolate, "source");

    std::vector<std::shared_ptr<Data>> layers_data(layers_array->Length());

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    for (int i = 0; i < layers_array->Length(); i++)
    {
        auto v = layers_array->Get(ctx, i).ToLocalChecked();
        if (!v->IsObject())
            g_throw(TypeError, "Elements of `layers` are not objects");

        auto obj = v.As<v8::Object>();

        v8::Local<v8::Value> prop_type_val;
        if (!obj->Get(ctx, keyname_type).ToLocal(&prop_type_val))
            g_throw(TypeError, "Missing `type` property for an object in `layers`");

        v8::Local<v8::Value> prop_source_val;
        if (!obj->Get(ctx, keyname_source).ToLocal(&prop_source_val))
            g_throw(TypeError, "Missing `source` property for an object in `layers`");

        layers_data[i] = create_data_from_source(isolate, prop_type_val, prop_source_val);
        if (!layers_data[i])
            g_throw(Error, fmt::format("Invalid data source layers[{}]", i));
    }

    auto disk = crpkg::VirtualDisk::MakeLayerDisk(layers_data);
    if (!disk)
        g_throw(Error, "Failed to create CRPKG layered virtual disk");

    return binder::NewObject<CRPKGVirtualDiskWrap>(isolate, std::move(disk));
}

v8::Local<v8::Value> CRPKGVirtualDiskWrap::resolve(const v8::Local<v8::String>& path)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::String::Utf8Value str(isolate, path);
    std::string_view sv(*str, str.length());

    std::optional<crpkg::VirtualDisk::Storage> storage = disk_->GetStorage(sv);
    if (!storage)
        return v8::Null(isolate);

    // Storage wrapper holds a reference of `crpkg::VirtualDisk` to keep
    // the storage data available.
    return binder::NewObject<CRPKGStorageWrap>(
            isolate, disk_, *storage);
}

void CRPKGVirtualDiskWrap::unref()
{
    disk_.reset();
}

GALLIUM_BINDINGS_RESOURCES_NS_END
