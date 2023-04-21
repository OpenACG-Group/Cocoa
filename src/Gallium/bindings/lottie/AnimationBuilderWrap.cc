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

#include "Gallium/bindings/resources/Exports.h"
#include "Gallium/bindings/lottie/Exports.h"
#include "Gallium/bindings/glamor/CkFontMgrWrap.h"
#include "Gallium/bindings/glamor/CkCanvasWrap.h"
#include "Gallium/binder/Class.h"
#include "Gallium/binder/ThrowExcept.h"
GALLIUM_BINDINGS_LOTTIE_NS_BEGIN

namespace {

class JSLoggerImpl : public skottie::Logger
{
public:
    JSLoggerImpl(v8::Isolate *isolate, v8::Local<v8::Function> func)
        : isolate_(isolate), func_(isolate, func) {}

    void log(Level level, const char *message, const char *json) override
    {
        v8::HandleScope scope(isolate_);
        v8::Local<v8::Value> args[] = {
            v8::Uint32::NewFromUnsigned(isolate_, static_cast<uint32_t>(level)),
            v8::String::NewFromUtf8(isolate_, message).ToLocalChecked(),
            json ? v8::String::NewFromUtf8(isolate_, json).ToLocalChecked().As<v8::Value>()
                 : v8::Null(isolate_).As<v8::Value>()
        };

        v8::Local<v8::Function> func = func_.Get(isolate_);
        v8::Local<v8::Context> ctx = isolate_->GetCurrentContext();

        (void) func->Call(ctx, v8::Null(isolate_), 3, args);
    }

private:
    v8::Isolate *isolate_;
    v8::Global<v8::Function> func_;
};

class JSMarkerObserverImpl : public skottie::MarkerObserver
{
public:
    JSMarkerObserverImpl(v8::Isolate *isolate, v8::Local<v8::Function> func)
        : isolate_(isolate), func_(isolate, func) {}

    void onMarker(const char *name, float t0, float t1) override
    {
        v8::HandleScope scope(isolate_);
        v8::Local<v8::Value> args[] = {
            v8::String::NewFromUtf8(isolate_, name).ToLocalChecked(),
            v8::Number::New(isolate_, t0),
            v8::Number::New(isolate_, t1)
        };

        v8::Local<v8::Function> func = func_.Get(isolate_);
        v8::Local<v8::Context> ctx = isolate_->GetCurrentContext();

        (void) func->Call(ctx, v8::Null(isolate_), 3, args);
    }

private:
    v8::Isolate *isolate_;
    v8::Global<v8::Function> func_;
};

class JSExternalLayerImpl : public skottie::ExternalLayer
{
public:
    JSExternalLayerImpl(v8::Isolate *isolate, v8::Local<v8::Function> func)
        : isolate_(isolate), func_(isolate, func) {}

    void render(SkCanvas *canvas, double t) override
    {
        CHECK(canvas);

        v8::HandleScope scope(isolate_);
        v8::Local<v8::Object> canvas_obj =
                binder::Class<glamor_wrap::CkCanvas>::create_object(isolate_, canvas);

        v8::Local<v8::Value> args[] = {
            canvas_obj,
            v8::Number::New(isolate_, t)
        };

        v8::Local<v8::Function> func = func_.Get(isolate_);
        v8::Local<v8::Context> ctx = isolate_->GetCurrentContext();

        (void) func->Call(ctx, v8::Null(isolate_), 2, args);
    }

private:
    v8::Isolate *isolate_;
    v8::Global<v8::Function> func_;
};

class JSPrecompInterceptorImpl : public skottie::PrecompInterceptor
{
public:
    JSPrecompInterceptorImpl(v8::Isolate *isolate, v8::Local<v8::Function> func)
        : isolate_(isolate), func_(isolate, func) {}

    sk_sp<skottie::ExternalLayer> onLoadPrecomp(const char *id,
                                                const char *name,
                                                const SkSize &size) override
    {
        v8::HandleScope scope(isolate_);
        v8::Local<v8::Value> args[] = {
            v8::String::NewFromUtf8(isolate_, id).ToLocalChecked(),
            v8::String::NewFromUtf8(isolate_, name).ToLocalChecked(),
            v8::Number::New(isolate_, size.width()),
            v8::Number::New(isolate_, size.height())
        };

        v8::Local<v8::Function> func = func_.Get(isolate_);
        v8::Local<v8::Context> ctx = isolate_->GetCurrentContext();

        v8::Local<v8::Value> ret;
        if (!func->Call(ctx, v8::Null(isolate_), 4, args).ToLocal(&ret) || !ret->IsFunction())
        {
            // Fallback to use the Lottie file content
            return nullptr;
        }

        return sk_make_sp<JSExternalLayerImpl>(isolate_, ret.As<v8::Function>());
    }

private:
    v8::Isolate *isolate_;
    v8::Global<v8::Function> func_;
};

} // namespace anonymous

AnimationBuilderWrap::AnimationBuilderWrap(uint32_t flags)
    : builder_(flags)
{
}

v8::Local<v8::Object> AnimationBuilderWrap::ReturnThis()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Object> obj =
            binder::Class<AnimationBuilderWrap>::find_object(isolate, this);

    CHECK(!obj.IsEmpty());
    return obj;
}

v8::Local<v8::Value> AnimationBuilderWrap::setResourceProvider(v8::Local<v8::Value> rp)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    resources_wrap::ResourceProviderWrap *rp_wrap =
            binder::Class<resources_wrap::ResourceProviderWrap>::unwrap_object(isolate, rp);
    if (!rp_wrap)
        g_throw(TypeError, "Argument `rp` must be an instance of `ResourceProvider`");

    builder_.setResourceProvider(rp_wrap->Get());

    return ReturnThis();
}

v8::Local<v8::Value> AnimationBuilderWrap::setFontManager(v8::Local<v8::Value> mgr)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    glamor_wrap::CkFontMgr *mgr_wrap =
            binder::Class<glamor_wrap::CkFontMgr>::unwrap_object(isolate, mgr);
    if (!mgr_wrap)
        g_throw(TypeError, "Argument `mgr` must be an instance of `CkFontMgr`");

    builder_.setFontManager(mgr_wrap->getSkiaObject());

    return ReturnThis();
}

v8::Local<v8::Value> AnimationBuilderWrap::setLogger(v8::Local<v8::Value> func)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!func->IsFunction())
        g_throw(TypeError, "Argument `func` must be a function");

    builder_.setLogger(sk_make_sp<JSLoggerImpl>(
            isolate, func.As<v8::Function>()));

    return ReturnThis();
}

v8::Local<v8::Value> AnimationBuilderWrap::setMarkerObserver(v8::Local<v8::Value> func)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!func->IsFunction())
        g_throw(TypeError, "Argument `func` must be a function");

    builder_.setMarkerObserver(sk_make_sp<JSMarkerObserverImpl>(
            isolate, func.As<v8::Function>()));

    return ReturnThis();
}

v8::Local<v8::Value> AnimationBuilderWrap::setPrecompInterceptor(v8::Local<v8::Value> func)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!func->IsFunction())
        g_throw(TypeError, "Argument `func` must be a function");

    builder_.setPrecompInterceptor(sk_make_sp<JSPrecompInterceptorImpl>(
            isolate, func.As<v8::Function>()));

    return ReturnThis();
}

v8::Local<v8::Value> AnimationBuilderWrap::setExpressionManager(v8::Local<v8::Value> manager)
{
    // TODO(sora): implement this
    return ReturnThis();
}

v8::Local<v8::Value> AnimationBuilderWrap::make(v8::Local<v8::Value> json)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!json->IsString())
        g_throw(TypeError, "Argument `json` must be a string");

    v8::String::Utf8Value str(isolate, json);
    sk_sp<skottie::Animation> animation = builder_.make(*str, str.length());
    if (!animation)
        g_throw(Error, "Failed to parse Lottie animation JSON");

    return binder::Class<AnimationWrap>::create_object(isolate, animation);
}

v8::Local<v8::Value> AnimationBuilderWrap::makeFromFile(const std::string& path)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    sk_sp<skottie::Animation> animation = builder_.makeFromFile(path.c_str());
    if (!animation)
        g_throw(Error, "Failed to parse Lottie animation JSON");

    return binder::Class<AnimationWrap>::create_object(isolate, animation);
}

GALLIUM_BINDINGS_LOTTIE_NS_END
