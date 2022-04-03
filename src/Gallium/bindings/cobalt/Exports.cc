#include "Gallium/bindings/cobalt/Exports.h"
#include "Cobalt/Surface.h"
GALLIUM_BINDINGS_COBALT_NS_BEGIN

CobaltBinding::CobaltBinding()
    : BindingBase("cobalt", "Cobalt Rendering Infrastructure")
{
}

CobaltBinding::~CobaltBinding() = default;

void CobaltBinding::onRegisterClasses(v8::Isolate *isolate)
{
    render_host_wrap_ = NewClassExport<RenderHostWrap>(isolate);
    (*render_host_wrap_)
        .set_static_func("Initialize", RenderHostWrap::Initialize)
        .set_static_func("Dispose", RenderHostWrap::Dispose)
        .set_static_func("Connect", RenderHostWrap::Connect);

    render_client_object_wrap_class_ = NewClassExport<RenderClientObjectWrap>(isolate);
    (*render_client_object_wrap_class_)
        .set("connect", &RenderClientObjectWrap::connect)
        .set("disconnect", &RenderClientObjectWrap::disconnect);

    display_wrap_class_ = NewClassExport<DisplayWrap>(isolate);
    (*display_wrap_class_)
        .inherit<RenderClientObjectWrap>()
        .set("close", &DisplayWrap::close)
        .set("createRasterSurface", &DisplayWrap::createRasterSurface)
        .set("createHWComposeSurface", &DisplayWrap::createHWComposeSurface);

    using i = cobalt::ToplevelStates;
    surface_wrap_class_ = NewClassExport<SurfaceWrap>(isolate);
    (*surface_wrap_class_)
        .inherit<RenderClientObjectWrap>()
        .set_static("TOPLEVEL_MAXIMIZED", static_cast<uint32_t>(i::kMaximized))
        .set_static("TOPLEVEL_FULLSCREEN", static_cast<uint32_t>(i::kFullscreen))
        .set_static("TOPLEVEL_RESIZING", static_cast<uint32_t>(i::kResizing))
        .set_static("TOPLEVEL_ACTIVATED", static_cast<uint32_t>(i::kActivated))
        .set_static("TOPLEVEL_TILED_LEFT", static_cast<uint32_t>(i::kTiledLeft))
        .set_static("TOPLEVEL_TILED_RIGHT", static_cast<uint32_t>(i::kTiledRight))
        .set_static("TOPLEVEL_TILED_TOP", static_cast<uint32_t>(i::kTiledTop))
        .set_static("TOPLEVEL_TILED_BOTTOM", static_cast<uint32_t>(i::kTiledBottom))
        .set("width", binder::Property(&SurfaceWrap::getWidth))
        .set("height", binder::Property(&SurfaceWrap::getHeight))
        .set("close", &SurfaceWrap::close)
        .set("resize", &SurfaceWrap::resize)
        .set("setTitle", &SurfaceWrap::setTitle)
        .set("getBuffersDescriptor", &SurfaceWrap::getBuffersDescriptor);
}

void CobaltBinding::onSetInstanceProperties(v8::Local<v8::Object> instance)
{
}

GALLIUM_BINDINGS_COBALT_NS_END
