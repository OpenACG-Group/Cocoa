#include "Gallium/bindings/cobalt/Exports.h"
#include "Gallium/bindings/cobalt/PromiseHelper.h"
#include "Cobalt/Display.h"
GALLIUM_BINDINGS_COBALT_NS_BEGIN

DisplayWrap::DisplayWrap(cobalt::co_sp<cobalt::RenderClientObject> object)
        : RenderClientObjectWrap(std::move(object))
{
    defineSignal("closed", CRSI_DISPLAY_CLOSED, nullptr);
}

DisplayWrap::~DisplayWrap() = default;

v8::Local<v8::Value> DisplayWrap::close()
{
    auto closure = PromiseClosure::New(v8::Isolate::GetCurrent(), nullptr);
    getObject()->Invoke(CROP_DISPLAY_CLOSE,
                        closure,
                        PromiseClosure::HostCallback);
    return closure->getPromise();
}

namespace {

v8::Local<v8::Value> create_surface_invoke(const i::co_sp<i::RenderClientObject>& display,
                                           bool hwCompose,
                                           v8::Isolate *isolate, int32_t width, int32_t height)
{
    if (width <= 0 || height <= 0)
        g_throw(RangeError, "Surface width and height must be positive integers");

    using Sp = i::co_sp<i::RenderClientObject>;
    auto closure = PromiseClosure::New(isolate,
                                       PromiseClosure::CreateObjectConverter<SurfaceWrap, Sp>);

    // TODO: Select a appropriate color format
    display->Invoke(hwCompose ? CROP_DISPLAY_CREATE_HW_COMPOSE_SURFACE : CROP_DISPLAY_CREATE_RASTER_SURFACE,
                    closure,
                    PromiseClosure::HostCallback,
                    width, height, SkColorType::kBGRA_8888_SkColorType);

    return closure->getPromise();
}

}

v8::Local<v8::Value> DisplayWrap::createRasterSurface(int32_t width, int32_t height)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return create_surface_invoke(getObject(), false, isolate, width, height);
}

v8::Local<v8::Value> DisplayWrap::createHWComposeSurface(int32_t width, int32_t height)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return create_surface_invoke(getObject(), true, isolate, width, height);
}

GALLIUM_BINDINGS_COBALT_NS_END
