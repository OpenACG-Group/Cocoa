#include "Gallium/bindings/core/Exports.h"
GALLIUM_BINDINGS_NS_BEGIN

void CoreSetInstanceProperties(v8::Local<v8::Object> instance)
{
    v8::Isolate *i = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = i->GetCurrentContext();

    v8::Local<v8::Array> argv = v8::Array::New(i);
    instance->Set(ctx, binder::to_v8(i, "args"), argv).Check();
    auto pass = prop::Get()->next("Runtime")->next("Script")
                ->next("Pass")->as<PropertyArrayNode>();
    int32_t index = 0;
    for (const auto& node : *pass)
    {
        auto str = node->as<PropertyDataNode>()->extract<std::string>();
        argv->Set(ctx, index++, binder::to_v8(i, str)).Check();
    }

    v8::Local<v8::Object> property = PropertyWrap::GetWrap(i, prop::Get());
    instance->Set(ctx, binder::to_v8(i, "property"), property).Check();
}

GALLIUM_BINDINGS_NS_END
