#include "Koi/bindings/cobalt/Exports.h"
KOI_BINDINGS_NS_BEGIN

CobaltBinding::CobaltBinding()
    : BindingBase("cobalt", "Cobalt Rendering Infrastructure")
    , render_client_object_wrap_class_(nullptr)
    , display_wrap_class_(nullptr)
{
}

CobaltBinding::~CobaltBinding()
{
    delete render_client_object_wrap_class_;
    delete display_wrap_class_;
}

void CobaltBinding::onRegisterClasses(v8::Isolate *isolate)
{
    render_client_object_wrap_class_ = new binder::Class<RenderClientObjectWrap>(isolate);
    (*render_client_object_wrap_class_)
        .constructor<cobalt::co_sp<cobalt::RenderClientObject>>()
        .set("connect", &RenderClientObjectWrap::connect)
        .set("disconnect", &RenderClientObjectWrap::disconnect);

    display_wrap_class_ = new binder::Class<DisplayWrap>(isolate);
    (*display_wrap_class_)
        .inherit<RenderClientObjectWrap>()
        .constructor<cobalt::co_sp<cobalt::RenderClientObject>>()
        .set("close", &DisplayWrap::close);
}

void CobaltBinding::onSetInstanceProperties(v8::Local<v8::Object> instance)
{
}

KOI_BINDINGS_NS_END
