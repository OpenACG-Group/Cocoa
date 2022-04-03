#include "Core/EnumClassBitfield.h"
#include "Gallium/bindings/cobalt/Exports.h"
#include "Gallium/bindings/cobalt/PromiseHelper.h"
#include "Cobalt/Surface.h"
GALLIUM_BINDINGS_COBALT_NS_BEGIN

SurfaceWrap::SurfaceWrap(cobalt::co_sp<cobalt::RenderClientObject> object)
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

GALLIUM_BINDINGS_COBALT_NS_END
