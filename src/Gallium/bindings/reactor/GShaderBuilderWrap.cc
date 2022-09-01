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

namespace {

void call_codegen_function(v8::Local<v8::Value> codegen, v8::Local<v8::Object> basicBlock)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    if (!codegen->IsFunction())
        g_throw(TypeError, "codegen callback must be a function");

    v8::Local<v8::Value> args[] = {basicBlock};

    v8::TryCatch tryCatch(isolate);
    codegen.As<v8::Function>()->Call(ctx, ctx->Global(), 1, args).ToLocalChecked();
    if (tryCatch.HasCaught())
        g_throw(TypeError, "codegen function threw an exception");
}

llvm::Value *extract_llvm_value(v8::Local<v8::Value> object)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    ValueWrap *w = binder::Class<ValueWrap>::unwrap_object(isolate, object);
    if (w == nullptr)
        g_throw(TypeError, "Invalid Value object");

    return w->value_;
}

template<typename T>
using CreateScalarFunc = llvm::Constant*(reactor::GShaderBuilder::*)(T);

template<typename T>
using CreateVec2Func = llvm::Constant*(reactor::GShaderBuilder::*)(T, T);

template<typename T>
using CreateVec4Func = llvm::Constant*(reactor::GShaderBuilder::*)(T, T, T, T);

template<typename T>
llvm::Value *create_constant_value_scalar(reactor::GShaderBuilder *b,
                                          CreateScalarFunc<T> method,
                                          const v8::FunctionCallbackInfo<v8::Value>& info)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return (b->*method)(binder::from_v8<T>(isolate, info[0]));
}

template<typename T>
llvm::Value *create_constant_value_vec2(reactor::GShaderBuilder *b,
                                        CreateVec2Func<T> method,
                                        const v8::FunctionCallbackInfo<v8::Value>& info)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return (b->*method)(binder::from_v8<T>(isolate, info[0]),
                        binder::from_v8<T>(isolate, info[1]));
}

template<typename T>
llvm::Value *create_constant_value_vec4(reactor::GShaderBuilder *b,
                                        CreateVec4Func<T> method,
                                        const v8::FunctionCallbackInfo<v8::Value>& info)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return (b->*method)(binder::from_v8<T>(isolate, info[0]),
                        binder::from_v8<T>(isolate, info[1]),
                        binder::from_v8<T>(isolate, info[2]),
                        binder::from_v8<T>(isolate, info[3]));
}

}

void GShaderBuilderWrap::userMainEntrypointBasicBlock(v8::Local<v8::Value> codegen)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto object = binder::Class<BasicBlockWrap>::create_object(isolate, builder_,
            builder_->GetMainEntrypointBasicBlock());
    call_codegen_function(codegen, object);
}

ValueWrap::ValueWrap(llvm::Value *v) : value_(v)
{
}

BasicBlockWrap::BasicBlockWrap(std::shared_ptr<reactor::GShaderBuilder> builder,
                               llvm::BasicBlock *BB)
    : builder_(std::move(builder))
    , basic_block_(BB)
{
}

#define VW(x) binder::Class<ValueWrap>::create_object(isolate, x)

#define DECL_NV_METHOD(rtype, ctype)                                                                 \
v8::Local<v8::Value> BasicBlockWrap::new##rtype (const v8::FunctionCallbackInfo<v8::Value>& info)    \
{                                                                                                    \
    v8::Isolate *isolate = v8::Isolate::GetCurrent();                                                \
    if (info.Length() == 0)                                                                          \
        return VW(builder_->New##rtype ());                                                          \
    else if (info.Length() == 1) {                                                                   \
        return VW(create_constant_value_scalar<ctype>(builder_.get(),                                \
                &reactor::GShaderBuilder::New##rtype, info));                                        \
    }                                                                                                \
    g_throw(TypeError, "Bad number of arguments");                                                   \
}                                                                                                    \
v8::Local<v8::Value> BasicBlockWrap::new##rtype##2 (const v8::FunctionCallbackInfo<v8::Value>& info) \
{                                                                                                    \
    v8::Isolate *isolate = v8::Isolate::GetCurrent();                                                \
    if (info.Length() == 0)                                                                          \
        return VW(builder_->New##rtype##2 ());                                                       \
    else if (info.Length() == 2) {                                                                   \
        return VW(create_constant_value_vec2<ctype>(builder_.get(),                                  \
                &reactor::GShaderBuilder::New##rtype##2, info));                                     \
    }                                                                                                \
    g_throw(TypeError, "Bad number of arguments");                                                   \
}                                                                                                    \
v8::Local<v8::Value> BasicBlockWrap::new##rtype##4 (const v8::FunctionCallbackInfo<v8::Value>& info) \
{                                                                                                    \
    v8::Isolate *isolate = v8::Isolate::GetCurrent();                                                \
    if (info.Length() == 0)                                                                          \
        return VW(builder_->New##rtype##4 ());                                                       \
    else if (info.Length() == 4) {                                                                   \
        return VW(create_constant_value_vec4<ctype>(builder_.get(),                                  \
                &reactor::GShaderBuilder::New##rtype##4, info));                                     \
    }                                                                                                \
    g_throw(TypeError, "Bad number of arguments");                                                   \
}

DECL_NV_METHOD(Byte, uint8_t)
DECL_NV_METHOD(SByte, int8_t)
DECL_NV_METHOD(Short, int16_t)
DECL_NV_METHOD(UShort, uint16_t)
DECL_NV_METHOD(Int, int32_t)
DECL_NV_METHOD(UInt, uint32_t)
DECL_NV_METHOD(Long, int64_t)
DECL_NV_METHOD(ULong, uint64_t)
DECL_NV_METHOD(Float, float)

void BasicBlockWrap::createReturn(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    llvm::IRBuilder<> emitter(basic_block_);
    if (info.Length() == 0)
        emitter.CreateRetVoid();
    else if (info.Length() == 1)
        emitter.CreateRet(extract_llvm_value(info[0]));
    else
        g_throw(TypeError, "Bad number of arguments");
}

void BasicBlockWrap::createJSFunctionCall(const std::string& name)
{
    builder_->CreateBuiltinV8FunctionCall(basic_block_, name);
}

GALLIUM_BINDINGS_REACTOR_NS_END
