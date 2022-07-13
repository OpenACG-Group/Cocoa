#include "fmt/format.h"

#include "Core/EnumClassBitfield.h"
#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/PromiseHelper.h"
#include "Glamor/Surface.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

SkRect CkRectToSkRectCast(v8::Isolate *isolate, v8::Local<v8::Value> object)
{
    if (!object->IsObject())
        g_throw(TypeError, "GskRect is not an object");

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

    v8::Local<v8::Object> r = object.As<v8::Object>();

    float ltrb[4] = {0, 0, 0, 0};
    float *pw = &ltrb[0];

    for (const char *prop : {"left", "top", "right", "bottom"})
    {
        auto jsName = binder::to_v8(isolate, prop);
        if (!r->HasOwnProperty(ctx, jsName).FromMaybe(false))
            g_throw(TypeError, fmt::format("GskRect does not contain property '{}'", prop));
        *(pw++) = binder::from_v8<float>(isolate, r->Get(ctx, jsName).ToLocalChecked());
    }

    return SkRect::MakeLTRB(ltrb[0], ltrb[1], ltrb[2], ltrb[3]);
}

SkIRect CkRectToSkIRectCast(v8::Isolate *isolate, v8::Local<v8::Value> object)
{
    SkRect rect = CkRectToSkRectCast(isolate, object);
    return SkIRect::MakeLTRB(static_cast<int32_t>(rect.left()),
                             static_cast<int32_t>(rect.top()),
                             static_cast<int32_t>(rect.right()),
                             static_cast<int32_t>(rect.bottom()));
}

SurfaceWrap::SurfaceWrap(glamor::Shared<glamor::RenderClientObject> object)
    : RenderClientObjectWrap(std::move(object))
{
    defineSignal("closed", CRSI_SURFACE_CLOSED, nullptr);
    defineSignal("resize", CRSI_SURFACE_RESIZE,
                 [](v8::Isolate *isolate, i::RenderHostSlotCallbackInfo& info) -> InfoAcceptorResult {
        std::vector<v8::Local<v8::Value>> ret{
            binder::to_v8(isolate, info.Get<int32_t>(0)),
            binder::to_v8(isolate, info.Get<int32_t>(1))
        };
        return std::move(ret);
    });
    defineSignal("close", CRSI_SURFACE_CLOSE, nullptr);
    defineSignal("configure", CRSI_SURFACE_CONFIGURE,
                 [](v8::Isolate *isolate, i::RenderHostSlotCallbackInfo& info) -> InfoAcceptorResult {
        std::vector<v8::Local<v8::Value>> ret{
            binder::to_v8(isolate, info.Get<int32_t>(0)),
            binder::to_v8(isolate, info.Get<int32_t>(1)),
            binder::to_v8(isolate, info.Get<Bitfield<i::ToplevelStates>>(2).value())
        };
        return std::move(ret);
    });
    defineSignal("frame", CRSI_SURFACE_FRAME,
                 [](v8::Isolate *isolate, i::RenderHostSlotCallbackInfo& info) -> InfoAcceptorResult {
        std::vector<v8::Local<v8::Value>> ret{
            binder::to_v8(isolate, info.Get<uint32_t>(0))
        };
        return std::move(ret);
    });
}

SurfaceWrap::~SurfaceWrap() = default;

int32_t SurfaceWrap::getWidth()
{
    return getObject()->Cast<i::Surface>()->GetWidth();
}

int32_t SurfaceWrap::getHeight()
{
    return getObject()->Cast<i::Surface>()->GetHeight();
}

v8::Local<v8::Value> SurfaceWrap::close()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto closure = PromiseClosure::New(isolate, nullptr);
    getObject()->Invoke(CROP_SURFACE_CLOSE, closure, PromiseClosure::HostCallback);
    return closure->getPromise();
}

v8::Local<v8::Value> SurfaceWrap::setTitle(const std::string& str)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto closure = PromiseClosure::New(isolate, nullptr);
    getObject()->Invoke(CROP_SURFACE_SET_TITLE, closure, PromiseClosure::HostCallback, str);
    return closure->getPromise();
}

v8::Local<v8::Value> SurfaceWrap::resize(int32_t width, int32_t height)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    using LV = v8::Local<v8::Value>;
    auto closure = PromiseClosure::New(isolate,
        [](v8::Isolate *isolate, i::RenderHostCallbackInfo& info) -> LV {
            return v8::Boolean::New(isolate, info.GetReturnValue<bool>());
    });

    getObject()->Invoke(CROP_SURFACE_RESIZE, closure, PromiseClosure::HostCallback,
                        width, height);
    return closure->getPromise();
}

v8::Local<v8::Value> SurfaceWrap::getBuffersDescriptor()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    using LV = v8::Local<v8::Value>;
    auto closure = PromiseClosure::New(isolate,
        [](v8::Isolate *isolate, i::RenderHostCallbackInfo& info) -> LV {
        return binder::to_v8(isolate, info.GetReturnValue<std::string>());
    });

    getObject()->Invoke(CROP_SURFACE_GET_BUFFERS_DESCRIPTOR,
                        closure, PromiseClosure::HostCallback);

    return closure->getPromise();
}

v8::Local<v8::Value> SurfaceWrap::requestNextFrame()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    using LV = v8::Local<v8::Value>;
    auto closure = PromiseClosure::New(isolate,
        [](v8::Isolate *isolate, i::RenderHostCallbackInfo& info) -> LV {
            return binder::to_v8(isolate, info.GetReturnValue<uint32_t>());
    });

    getObject()->Invoke(CROP_SURFACE_REQUEST_NEXT_FRAME, closure, PromiseClosure::HostCallback);

    return closure->getPromise();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
