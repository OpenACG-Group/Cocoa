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

/* TSDecl: class Value */
class ValueWrap
{
public:
    explicit ValueWrap(llvm::Value *v);

    llvm::Value *value_;
};

/* TSDecl: class BasicBlock */
class BasicBlockWrap
{
public:
    BasicBlockWrap(std::shared_ptr<reactor::GShaderBuilder> builder,
                   llvm::BasicBlock *BB);

    /* TSDecl: function newByte(): Value
           function newByte(x: number): Value */
    v8::Local<v8::Value> newByte(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newByte2(): Value
               function newByte2(x: number, y: number): Value */
    v8::Local<v8::Value> newByte2(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newByte4(): Value
               function newByte4(x: number, y: number, z: number, w: number): Value */
    v8::Local<v8::Value> newByte4(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newSByte(): Value
               function newSByte(x: number): Value */
    v8::Local<v8::Value> newSByte(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newSByte2(): Value
               function newSByte2(x: number, y: number): Value */
    v8::Local<v8::Value> newSByte2(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newSByte4(): Value
               function newSByte4(x: number, y: number, z: number, w: number): Value */
    v8::Local<v8::Value> newSByte4(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newShort(): Value
               function newShort(x: number): Value */
    v8::Local<v8::Value> newShort(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newShort2(): Value
               function newShort2(x: number, y: number): Value */
    v8::Local<v8::Value> newShort2(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newShort4(): Value
               function newShort4(x: number, y: number, z: number, w: number): Value */
    v8::Local<v8::Value> newShort4(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newUShort(): Value
               function newUShort(x: number): Value */
    v8::Local<v8::Value> newUShort(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newUShort2(): Value
               function newUShort2(x: number, y: number): Value */
    v8::Local<v8::Value> newUShort2(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newUShort4(): Value
               function newUShort4(x: number, y: number, z: number, w: number): Value */
    v8::Local<v8::Value> newUShort4(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newInt(): Value
               function newInt(x: number): Value */
    v8::Local<v8::Value> newInt(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newInt2(): Value
               function newInt2(x: number, y: number): Value */
    v8::Local<v8::Value> newInt2(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newInt4(): Value
               function newInt4(x: number, y: number, z: number, w: number): Value */
    v8::Local<v8::Value> newInt4(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newUInt(): Value
               function newUInt(x: number): Value */
    v8::Local<v8::Value> newUInt(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newUInt2(): Value
               function newUInt2(x: number, y: number): Value */
    v8::Local<v8::Value> newUInt2(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newUInt4(): Value
               function newUInt4(x: number, y: number, z: number, w: number): Value */
    v8::Local<v8::Value> newUInt4(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newLong(): Value
               function newLong(x: number): Value */
    v8::Local<v8::Value> newLong(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newLong2(): Value
               function newLong2(x: number, y: number): Value */
    v8::Local<v8::Value> newLong2(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newLong4(): Value
               function newLong4(x: number, y: number, z: number, w: number): Value */
    v8::Local<v8::Value> newLong4(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newULong(): Value
               function newULong(x: number): Value */
    v8::Local<v8::Value> newULong(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newULong2(): Value
               function newULong2(x: number, y: number): Value */
    v8::Local<v8::Value> newULong2(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newULong4(): Value
               function newULong4(x: number, y: number, z: number, w: number): Value */
    v8::Local<v8::Value> newULong4(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newFloat(): Value
               function newFloat(x: number): Value */
    v8::Local<v8::Value> newFloat(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newFloat2(): Value
               function newFloat2(x: number, y: number): Value */
    v8::Local<v8::Value> newFloat2(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function newFloat4(): Value
               function newFloat4(x: number, y: number, z: number, w: number): Value */
    v8::Local<v8::Value> newFloat4(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function createReturn(): void
               function createReturn(value: Value): void */
    void createReturn(const v8::FunctionCallbackInfo<v8::Value>& info);

    /* TSDecl: function createJSFunctionCall(name: string): void */
    void createJSFunctionCall(const std::string& name);

    std::shared_ptr<reactor::GShaderBuilder> builder_;
    llvm::BasicBlock *basic_block_;
};

/* TSDecl: class GShaderBuilder */
class GShaderBuilderWrap
{
public:
    /* TSDecl: constructor(name: string) */
    explicit GShaderBuilderWrap(const std::string& name);

    /* TSDecl: function mainTestCodeGen(): void */
    void mainTestCodeGen();

    /* TSDecl: function insertJSFunctionSymbol(func: Function, name: string): void */
    void insertJSFunctionSymbol(v8::Local<v8::Value> func, const std::string& name);

    /* TSDecl: function userMainEntrypointBasicBlock(codegen: Function): void */
    void userMainEntrypointBasicBlock(v8::Local<v8::Value> codegen);

    std::shared_ptr<reactor::GShaderBuilder> builder_;
};

/* TSDecl: GShaderModuleWrap */
class GShaderModuleWrap
{
public:
    explicit GShaderModuleWrap(std::shared_ptr<reactor::GShaderModule> module);

    /* TSDecl: function Compile(builder: GShaderBuilder): Promise<GShaderModuleWrap> */
    static v8::Local<v8::Object> Compile(v8::Local<v8::Object> builder);

    /* TSDecl: function executeMain(): void */
    void executeMain();

    std::shared_ptr<reactor::GShaderModule> module_;
};

GALLIUM_BINDINGS_REACTOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_REACTOR_EXPORTS_H
