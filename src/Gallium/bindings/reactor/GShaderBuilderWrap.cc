#include "Reactor/Reactor.h"
#include "Gallium/bindings/reactor/Exports.h"
GALLIUM_BINDINGS_REACTOR_NS_BEGIN

GShaderBuilderWrap::GShaderBuilderWrap(const std::string& name)
    : builder_(std::make_shared<reactor::GShaderBuilder>(name))
{
}

void GShaderBuilderWrap::insertJSFunctionSymbol(v8::Local<v8::Value> func, const std::string& name)
{
    if (!func->IsFunction())
        g_throw(TypeError, "callback must be a Function object");
    builder_->InsertV8FunctionSymbol(func.As<v8::Function>(), name);
}

void GShaderBuilderWrap::mainTestCodeGen()
{
    builder_->MainTestCodeGen();
}

GALLIUM_BINDINGS_REACTOR_NS_END
