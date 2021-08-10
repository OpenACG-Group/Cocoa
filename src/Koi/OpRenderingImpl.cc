#include "Koi/Ops.h"
#include "Koi/Runtime.h"
#include "Koi/GpObjectWrapper.h"
#include "Vanilla/Context.h"
#include "Vanilla/Display.h"
#include "Vanilla/Window.h"
KOI_NS_BEGIN

OpHandlerImpl(op_va_ctx_create)
{
    using namespace js_literal;

    v8::Local<v8::Context> context = param.runtime()->context();
    v8::Isolate *isolate = param.isolate();
    v8::Local<v8::Object> args = param.get();

    vanilla::Context::Backend backendEnum;
    {
        if (args->Has(context, "backend"_js).IsNothing())
            return -OP_EINVARG;
        std::string backend(*v8::String::Utf8Value(isolate, param["backend"]));
        if (backend == "X11" || backend == "XCB")
            backendEnum = vanilla::Context::Backend::kXcb;
        else
            return -OP_EINVARG;
    }

    auto vanillaContext = vanilla::Context::Make(param.runtime()->eventLoop(), backendEnum);
    if (vanillaContext == nullptr)
        return -OP_EINTERNAL;

    return param.runtime()->resourcePool().resourceGen<GpObjectWrapper>(param.runtime(),
                                                                         std::move(vanillaContext),
                                                                         [](GpObjectWrapper *pThis) -> void {
        auto ctx = pThis->get<vanilla::Context>();
        if (!ctx->allDisplaysAreUnique())
        {
            throw vanilla::VanillaException("op_va_ctx_create::lambda",
                                            "Displays are owned by other objects while disposing");
        }
    })->getRID();
}

OpHandlerImpl(op_va_ctx_connect)
{
    using namespace js_literal;

    v8::Local<v8::Context> context = param.context();
    v8::Local<v8::Object> args = param.get();

    RID rid = OpExtractRIDFromArgs(param);
    if (rid < 0)
        return rid;
    ResourceDescriptorPool::ScopedAcquire<GpObjectWrapper> scope(param.runtime()->resourcePool(), rid);
    if (!scope.valid())
        return -OP_EBADRID;
    auto ctx = scope->get<vanilla::Context>();

    const char *displayName = nullptr;
    if (!args->Has(context, "displayName"_js).IsNothing()
        && !param["displayName"]->IsNull())
    {
        displayName = *v8::String::Utf8Value(param.isolate(), param["displayName"]);
    }

    int32_t displayId;
    if (args->Has(context, "displayId"_js).IsNothing())
        return -OP_EINVARG;
    displayId = param["displayId"]->ToInt32(context).ToLocalChecked()->Value();

    if (ctx->hasDisplay(displayId))
        return -OP_EBUSY;

    ctx->connectTo(displayName, displayId);
    return OP_SUCCESS;
}

namespace {

std::tuple<vanilla::Handle<vanilla::Display>, OpRet> extract_display_from_args(OpParameterInfo& param)
{
    using namespace js_literal;

    v8::Local<v8::Context> context = param.context();
    v8::Local<v8::Object> args = param.get();

    RID rid = OpExtractRIDFromArgs(param);
    if (rid < 0)
        return {nullptr, rid};
    ResourceDescriptorPool::ScopedAcquire<GpObjectWrapper> scope(param.runtime()->resourcePool(), rid);
    if (!scope.valid())
        return {nullptr, -OP_EBADRID};
    auto ctx = scope->get<vanilla::Context>();

    if (args->Has(context, "displayId"_js).IsNothing())
        return {nullptr, -OP_EINVARG};
    int32_t displayId = param["displayId"]->ToInt32(context).ToLocalChecked()->Value();
    if (!ctx->hasDisplay(displayId))
        return {nullptr, -OP_EINVARG};
    vanilla::Handle<vanilla::Display> display = ctx->display(displayId);

    return {display, displayId};
}

} // namespace anonymous

OpHandlerImpl(op_va_dis_close)
{
    auto [display, ret] = extract_display_from_args(param);
    if (ret < 0)
        return ret;
    if (display == nullptr)
        return -OP_EINTERNAL;

    display->getContext()->detachDisplay(ret);
    display->dispose();
    return OP_SUCCESS;
}

OpHandlerImpl(op_va_dis_geometry)
{
    using namespace js_literal;

    v8::Local<v8::Context> context = param.context();
    v8::Local<v8::Object> args = param.get();

    if (args->Has(context, "out"_js).IsNothing())
        return -OP_EINVARG;
    v8::Local<v8::Object> outputObject;
    if (!param["out"]->ToObject(context).ToLocal(&outputObject))
        return -OP_ETYPE;

    auto [display, ret] = extract_display_from_args(param);
    if (ret < 0)
        return ret;
    if (display == nullptr)
        return -OP_EINTERNAL;

    outputObject->Set(context, "width"_js, v8::Integer::New(param.isolate(), display->width())).Check();
    outputObject->Set(context, "height"_js, v8::Integer::New(param.isolate(), display->height())).Check();
    return OP_SUCCESS;
}

KOI_NS_END
