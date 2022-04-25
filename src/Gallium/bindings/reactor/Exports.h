#ifndef COCOA_GALLIUM_BINDINGS_REACTOR_EXPORTS_H
#define COCOA_GALLIUM_BINDINGS_REACTOR_EXPORTS_H

#include "Reactor/Reactor.h"
#include "Gallium/bindings/Base.h"
#include "Gallium/binder/Class.h"

#define GALLIUM_BINDINGS_REACTOR_NS_BEGIN   namespace cocoa::gallium::bindings::reactor_wrap {
#define GALLIUM_BINDINGS_REACTOR_NS_END     }

GALLIUM_BINDINGS_REACTOR_NS_BEGIN

class GShaderModuleWrap;
class GShaderBuilderWrap;
class BasicBlockWrap;
class ValueWrap;

class ReactorBinding : public BindingBase
{
    GALLIUM_BINDING_OBJECT

public:
    ReactorBinding();
    ~ReactorBinding() override;

    void onRegisterClasses(v8::Isolate *isolate) override;

    ClassExport<GShaderBuilderWrap> gshader_builder_wrap_;
    ClassExport<GShaderModuleWrap>  gshader_module_wrap_;
    ClassExport<BasicBlockWrap>     basic_block_wrap_;
    ClassExport<ValueWrap>          value_wrap_;
};

/* JSDecl: class Value */
class ValueWrap
{
public:
    explicit ValueWrap(llvm::Value *v);

    llvm::Value *value_;
};

/* JSDecl: class BasicBlock */
class BasicBlockWrap
{
public:
    BasicBlockWrap(std::shared_ptr<reactor::GShaderBuilder> builder,
                   llvm::BasicBlock *BB);

    /* JSDecl: function newByte(): Value
           function newByte(x: number): Value */
    v8::Local<v8::Value> newByte(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newByte2(): Value
               function newByte2(x: number, y: number): Value */
    v8::Local<v8::Value> newByte2(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newByte4(): Value
               function newByte4(x: number, y: number, z: number, w: number): Value */
    v8::Local<v8::Value> newByte4(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newSByte(): Value
               function newSByte(x: number): Value */
    v8::Local<v8::Value> newSByte(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newSByte2(): Value
               function newSByte2(x: number, y: number): Value */
    v8::Local<v8::Value> newSByte2(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newSByte4(): Value
               function newSByte4(x: number, y: number, z: number, w: number): Value */
    v8::Local<v8::Value> newSByte4(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newShort(): Value
               function newShort(x: number): Value */
    v8::Local<v8::Value> newShort(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newShort2(): Value
               function newShort2(x: number, y: number): Value */
    v8::Local<v8::Value> newShort2(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newShort4(): Value
               function newShort4(x: number, y: number, z: number, w: number): Value */
    v8::Local<v8::Value> newShort4(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newUShort(): Value
               function newUShort(x: number): Value */
    v8::Local<v8::Value> newUShort(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newUShort2(): Value
               function newUShort2(x: number, y: number): Value */
    v8::Local<v8::Value> newUShort2(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newUShort4(): Value
               function newUShort4(x: number, y: number, z: number, w: number): Value */
    v8::Local<v8::Value> newUShort4(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newInt(): Value
               function newInt(x: number): Value */
    v8::Local<v8::Value> newInt(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newInt2(): Value
               function newInt2(x: number, y: number): Value */
    v8::Local<v8::Value> newInt2(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newInt4(): Value
               function newInt4(x: number, y: number, z: number, w: number): Value */
    v8::Local<v8::Value> newInt4(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newUInt(): Value
               function newUInt(x: number): Value */
    v8::Local<v8::Value> newUInt(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newUInt2(): Value
               function newUInt2(x: number, y: number): Value */
    v8::Local<v8::Value> newUInt2(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newUInt4(): Value
               function newUInt4(x: number, y: number, z: number, w: number): Value */
    v8::Local<v8::Value> newUInt4(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newLong(): Value
               function newLong(x: number): Value */
    v8::Local<v8::Value> newLong(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newLong2(): Value
               function newLong2(x: number, y: number): Value */
    v8::Local<v8::Value> newLong2(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newLong4(): Value
               function newLong4(x: number, y: number, z: number, w: number): Value */
    v8::Local<v8::Value> newLong4(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newULong(): Value
               function newULong(x: number): Value */
    v8::Local<v8::Value> newULong(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newULong2(): Value
               function newULong2(x: number, y: number): Value */
    v8::Local<v8::Value> newULong2(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newULong4(): Value
               function newULong4(x: number, y: number, z: number, w: number): Value */
    v8::Local<v8::Value> newULong4(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newFloat(): Value
               function newFloat(x: number): Value */
    v8::Local<v8::Value> newFloat(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newFloat2(): Value
               function newFloat2(x: number, y: number): Value */
    v8::Local<v8::Value> newFloat2(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function newFloat4(): Value
               function newFloat4(x: number, y: number, z: number, w: number): Value */
    v8::Local<v8::Value> newFloat4(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function createReturn(): void
               function createReturn(value: Value): void */
    void createReturn(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* JSDecl: function createJSFunctionCall(name: string): void */
    void createJSFunctionCall(const std::string& name);

    std::shared_ptr<reactor::GShaderBuilder> builder_;
    llvm::BasicBlock *basic_block_;
};

/* JSDecl: class GShaderBuilder */
class GShaderBuilderWrap
{
public:
    /* JSDecl: constructor(name: string) */
    explicit GShaderBuilderWrap(const std::string& name);

    /* JSDecl: function mainTestCodeGen(): void */
    void mainTestCodeGen();

    /* JSDecl: function insertJSFunctionSymbol(func: Function, name: string): void */
    void insertJSFunctionSymbol(v8::Local<v8::Value> func, const std::string& name);

    /* JSDecl: function userMainEntrypointBasicBlock(codegen: Function): void */
    void userMainEntrypointBasicBlock(v8::Local<v8::Value> codegen);

    std::shared_ptr<reactor::GShaderBuilder> builder_;
};

/* JSDecl: GShaderModuleWrap */
class GShaderModuleWrap
{
public:
    explicit GShaderModuleWrap(std::shared_ptr<reactor::GShaderModule> module);

    /* JSDecl: function Compile(builder: GShaderBuilder): Promise<GShaderModuleWrap> */
    static v8::Local<v8::Object> Compile(v8::Local<v8::Object> builder);

    /* JSDecl: function executeMain(): void */
    void executeMain();

    std::shared_ptr<reactor::GShaderModule> module_;
};

GALLIUM_BINDINGS_REACTOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_REACTOR_EXPORTS_H
