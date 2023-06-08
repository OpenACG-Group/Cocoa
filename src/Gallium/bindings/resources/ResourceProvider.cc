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

#include "Gallium/binder/ThrowExcept.h"
#include "Gallium/binder/Class.h"
#include "Gallium/bindings/resources/Exports.h"
#include "Gallium/bindings/glamor/CkMatrixWrap.h"
#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/CkTypefaceWrap.h"
GALLIUM_BINDINGS_RESOURCES_NS_BEGIN

namespace {

struct U8ArrayData
{
    std::shared_ptr<v8::BackingStore> store;
    size_t size;
    size_t offset;
};

#define PROPS(n, ...) std::array<std::string_view, n>{ __VA_ARGS__ }

template<size_t N, size_t kInfoSize>
void check_object_properties(v8::Isolate *isolate, v8::Local<v8::Value> object,
                             const char (&info)[kInfoSize],
                             std::array<std::string_view, N> props)
{
    v8::HandleScope scope(isolate);

    if (!object->IsObject())
        g_throw(TypeError, fmt::format("{}: must be an object", info));

    auto obj = v8::Local<v8::Object>::Cast(object);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    for (const std::string_view& name : props)
    {
        auto str = v8::String::NewFromUtf8(isolate,
                                           name.data(),
                                           v8::NewStringType::kNormal,
                                           static_cast<int>(name.length()))
                .ToLocalChecked();

        if (!obj->Has(context, str).FromMaybe(false))
            g_throw(TypeError, fmt::format("{}: missing required property \'{}\'", info, name));
    }
}

class JSMethodInvoker
{
public:
    explicit JSMethodInvoker(v8::Isolate *isolate, v8::Local<v8::Object> object)
        : this_(isolate, object) {}
    ~JSMethodInvoker() = default;

#define ARGV(n, ...) \
    std::array<v8::Local<v8::Value>, n>{ __VA_ARGS__ }

    template<size_t N, size_t K>
    v8::MaybeLocal<v8::Value> Invoke(v8::Isolate *isolate, const char (&method)[N],
                                     std::array<v8::Local<v8::Value>, K> argv) const
    {
        CHECK(isolate);

        v8::EscapableHandleScope scope(isolate);
        v8::Local<v8::Object> object = this_.Get(isolate);
        v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

        v8::MaybeLocal<v8::Value> prop =
                object->Get(ctx, v8::String::NewFromUtf8Literal(isolate, method));
        if (prop.IsEmpty())
            return {};

        v8::Local <v8::Value> prop_value = prop.ToLocalChecked();
        if (!prop_value->IsFunction())
            return {};

        return scope.EscapeMaybe(
                prop_value.As<v8::Function>()->Call(ctx, object, K, argv.data()));
    }

private:
    v8::Global<v8::Object>   this_;
};

#define SKRESOURCES_IMPL_OBJECT(type)                           \
    private:                                                    \
        v8::Isolate *isolate_;                                  \
        JSMethodInvoker invoker_;                               \
    public:                                                     \
        type(v8::Isolate *isolate, v8::Local<v8::Object> impl)  \
            : isolate_(isolate), invoker_(isolate, impl) {}     \
        ~type() override = default;

#define INVOKE_CHECKED(ret, method, n_argv, ...)                         \
    v8::MaybeLocal<v8::Value> maybe_##ret =                              \
        invoker_.Invoke(isolate_, #method, ARGV(n_argv, __VA_ARGS__));   \
    if (maybe_##ret.IsEmpty()) {                                         \
        g_throw(Error, "Failed to invoke '" #method "' method");         \
    }

#define INVOKE_CHECKED_RET(ret, method, n_argv, ...) \
    INVOKE_CHECKED(ret, method, n_argv, __VA_ARGS__) \
    v8::Local<v8::Value> ret = maybe_##ret.ToLocalChecked();

class ExternalTrackAssetImpl : public skresources::ExternalTrackAsset
{
    SKRESOURCES_IMPL_OBJECT(ExternalTrackAssetImpl)

public:
    void seek(float t) override
    {
        v8::HandleScope scope(isolate_);
        INVOKE_CHECKED(ret, seek, 1, v8::Number::New(isolate_, t))
    }
};

class ImageAssetImpl : public skresources::ImageAsset
{
    SKRESOURCES_IMPL_OBJECT(ImageAssetImpl);

public:
    FrameData getFrameData(float t) override
    {
        v8::HandleScope scope(isolate_);
        INVOKE_CHECKED_RET(ret, getFrameData, 1, v8::Number::New(isolate_, t))
        if (!ret->IsObject())
            g_throw(TypeError, "Implementor of method 'getFrameData' must return an object");

        auto obj = v8::Local<v8::Object>::Cast(ret);
        v8::Local<v8::Context> context = isolate_->GetCurrentContext();

#define JS_PROPERTY(key, receiver)                                           \
        v8::Local<v8::Value> receiver;                                       \
        do {                                                                 \
            auto val = obj->Get(context,                                     \
                v8::String::NewFromUtf8Literal(isolate_, #key));             \
            if (val.IsEmpty()) {                                             \
                g_throw(TypeError, "Return value of method 'getFrameData':"  \
                " missing property " #key);                                  \
            }                                                                \
            receiver = val.ToLocalChecked();                                 \
        } while (false)

        JS_PROPERTY(image, image_value);
        JS_PROPERTY(sampling, sampling_value);
        JS_PROPERTY(matrix, matrix_value);
        JS_PROPERTY(scaling, scaling_value);

#undef JS_PROPERTY

        glamor_wrap::CkImageWrap *image_wrap =
                binder::UnwrapObject<glamor_wrap::CkImageWrap>(isolate_, image_value);
        glamor_wrap::CkMatrix *matrix_wrap =
                binder::UnwrapObject<glamor_wrap::CkMatrix>(isolate_, matrix_value);

        if (!image_wrap || !matrix_wrap ||
            !sampling_value->IsUint32() || !scaling_value->IsUint32())
        {
            g_throw(TypeError, "Invalid return value of method 'getFrameData'");
        }

        auto sampling_options = glamor_wrap::SamplingToSamplingOptions(
                sampling_value->Int32Value(context).ToChecked());

        uint32_t scaling_enum = scaling_value->Uint32Value(context).ToChecked();
        if (scaling_enum > static_cast<int>(SizeFit::kNone))
        {
            g_throw(TypeError, "Invalid return value of method 'getFrameData'"
                               " (invalid enumeration for property 'scaling')");
        }

        return {
            image_wrap->getImage(),
            sampling_options,
            matrix_wrap->GetMatrix(),
            static_cast<SizeFit>(scaling_enum)
        };
    }

    bool isMultiFrame() override
    {
        v8::HandleScope scope(isolate_);
        INVOKE_CHECKED_RET(ret, isMultiFrame, 0)
        if (!ret->IsBoolean())
            g_throw(TypeError, "Invalid return value of method 'isMultiFrame'");
        return ret->BooleanValue(isolate_);
    }
};

class ResourceProviderImpl : public skresources::ResourceProvider
{
    SKRESOURCES_IMPL_OBJECT(ResourceProviderImpl)

public:
    sk_sp<SkData> load(const char *path, const char *name) const override
    {
        INVOKE_CHECKED_RET(ret, load, 2,
                           v8::String::NewFromUtf8(isolate_, path).ToLocalChecked(),
                           v8::String::NewFromUtf8(isolate_, name).ToLocalChecked())

        if (!ret->IsUint8Array())
            g_throw(TypeError, "ResourceProvider: Invalid return value from 'load' method");

        v8::Local<v8::Uint8Array> array = v8::Local<v8::Uint8Array>::Cast(ret);
        if (!array->HasBuffer())
            g_throw(TypeError, "ResourceProvider: Unallocated buffer returned by 'load' method");

        auto *closure = new U8ArrayData{
            array->Buffer()->GetBackingStore(),
            array->ByteLength(),
            array->ByteOffset()
        };

        auto releaser = +[](const void *ptr, void *userdata) {
            CHECK(userdata);
            delete reinterpret_cast<U8ArrayData*>(userdata);
        };

        return SkData::MakeWithProc(
                reinterpret_cast<uint8_t*>(array->Buffer()->Data()) + closure->offset,
                closure->size, releaser, closure
        );
    }

    sk_sp<skresources::ImageAsset> loadImageAsset(const char *path,
                                                  const char *name,
                                                  const char *id) const override
    {
        INVOKE_CHECKED_RET(ret, loadImageAsset, 3,
                           v8::String::NewFromUtf8(isolate_, path).ToLocalChecked(),
                           v8::String::NewFromUtf8(isolate_, name).ToLocalChecked(),
                           v8::String::NewFromUtf8(isolate_, id).ToLocalChecked())

        ImageAssetWrap *wrap = binder::UnwrapObject<ImageAssetWrap>(isolate_, ret);
        if (!wrap)
            g_throw(TypeError, "ResourceProvider: Invalid return value from 'loadImageAsset' method");

        return wrap->Get();
    }

    sk_sp<skresources::ExternalTrackAsset> loadAudioAsset(const char *path,
                                                          const char *name,
                                                          const char *id) override
    {
        INVOKE_CHECKED_RET(ret, loadImageAsset, 3,
                           v8::String::NewFromUtf8(isolate_, path).ToLocalChecked(),
                           v8::String::NewFromUtf8(isolate_, name).ToLocalChecked(),
                           v8::String::NewFromUtf8(isolate_, id).ToLocalChecked())

        ExternalTrackAssetWrap *wrap =
                binder::UnwrapObject<ExternalTrackAssetWrap>(isolate_, ret);
        if (!wrap)
            g_throw(TypeError, "ResourceProvider: Invalid return value from 'loadAudioAsset' method");

        return wrap->Get();
    }

    sk_sp<SkTypeface> loadTypeface(const char *name, const char *url) const override
    {
        INVOKE_CHECKED_RET(ret, loadTypeface, 2,
                           v8::String::NewFromUtf8(isolate_, name).ToLocalChecked(),
                           v8::String::NewFromUtf8(isolate_, url).ToLocalChecked())

        glamor_wrap::CkTypeface *wrap =
                binder::UnwrapObject<glamor_wrap::CkTypeface>(isolate_, ret);
        if (!wrap)
            g_throw(TypeError, "ResourceProvider: Invalid return value from 'loadTypeface' method");

        return wrap->GetSkObject();
    }
};

} // namespace anonymous

v8::Local<v8::Value> ExternalTrackAssetWrap::MakeImpl(v8::Local<v8::Value> impl)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    check_object_properties(isolate, impl, "Argument 'impl'", PROPS(1, "seek"));

    auto asset = sk_make_sp<ExternalTrackAssetImpl>(isolate, impl.As<v8::Object>());
    return binder::NewObject<ExternalTrackAssetWrap>(isolate, std::move(asset));
}

v8::Local<v8::Value> ImageAssetWrap::MakeMultiFrame(v8::Local<v8::Value> data, bool predecode)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    if (!data->IsUint8Array())
        g_throw(TypeError, "Argument 'data' must be a Uint8Array");

    auto array = v8::Local<v8::Uint8Array>::Cast(data);
    if (!array->HasBuffer())
        g_throw(TypeError, "Argument 'data' must be a Uint8Array with an allocated buffer");

    auto *closure = new U8ArrayData{
        array->Buffer()->GetBackingStore(),
        array->ByteLength(),
        array->ByteOffset()
    };

    auto release_proc = +[](const void *ptr, void *userdata) {
        CHECK(ptr && userdata);
        delete reinterpret_cast<U8ArrayData*>(userdata);
    };

    sk_sp<SkData> skdata = SkData::MakeWithProc(
            reinterpret_cast<uint8_t*>(closure->store->Data()) + closure->offset,
            closure->size, release_proc, closure);

    return binder::NewObject<ImageAssetWrap>(
            isolate, skresources::MultiFrameImageAsset::Make(skdata, predecode));
}

v8::Local<v8::Value> ImageAssetWrap::MakeImpl(v8::Local<v8::Value> impl)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    check_object_properties(isolate, impl, "Argument 'impl'",
                            PROPS(2, "isMultiFrame", "getFrameData"));

    auto asset = sk_make_sp<ImageAssetImpl>(isolate, impl.As<v8::Object>());
    return binder::NewObject<ImageAssetWrap>(isolate, std::move(asset));
}

v8::Local<v8::Value> ResourceProviderWrap::MakeImpl(v8::Local<v8::Value> impl)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    check_object_properties(isolate, impl, "Argument 'impl'",
                            PROPS(4, "load", "loadImageAsset", "loadAudioAsset", "loadTypeface"));

    auto rp = sk_make_sp<ResourceProviderImpl>(isolate, impl.As<v8::Object>());
    return binder::NewObject<ResourceProviderWrap>(isolate, std::move(rp));
}

v8::Local<v8::Value> ResourceProviderWrap::MakeFile(const std::string& base_dir, bool predecode)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    sk_sp<skresources::FileResourceProvider> rp =
            skresources::FileResourceProvider::Make(SkString(base_dir), predecode);
    if (!rp)
        g_throw(TypeError, "Failed to create FileResourceProvider");

    return binder::NewObject<ResourceProviderWrap>(isolate, std::move(rp));
}

v8::Local<v8::Value> ResourceProviderWrap::MakeCachingProxy(v8::Local<v8::Value> rp)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    ResourceProviderWrap *wrap =
            binder::UnwrapObject<ResourceProviderWrap>(isolate, rp);
    if (!wrap)
        g_throw(TypeError, "Argument `rp` must be an instance of `ResourceProvider`");

    sk_sp<skresources::ResourceProvider> proxy =
            skresources::CachingResourceProvider::Make(wrap->Get());

    CHECK(proxy);

    return binder::NewObject<ResourceProviderWrap>(isolate, std::move(proxy));
}

v8::Local<v8::Value> ResourceProviderWrap::MakeDataURIProxy(v8::Local<v8::Value> rp,
                                                            bool predecode)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    ResourceProviderWrap *wrap =
            binder::UnwrapObject<ResourceProviderWrap>(isolate, rp);

    if (!wrap)
        g_throw(TypeError, "Argument `rp` must be an instance of `ResourceProvider`");

    sk_sp<skresources::ResourceProvider> proxy =
            skresources::DataURIResourceProviderProxy::Make(wrap->Get(), predecode);

    CHECK(proxy);

    return binder::NewObject<ResourceProviderWrap>(isolate, std::move(proxy));
}

v8::Local<v8::Value> ResourceProviderWrap::MakeProxyImpl(v8::Local<v8::Value> impl)
{
    // TODO(sora): implement this
    g_throw(Error, "Not implemented yet!");
    // return v8::Undefined(v8::Isolate::GetCurrent());
}

GALLIUM_BINDINGS_RESOURCES_NS_END
