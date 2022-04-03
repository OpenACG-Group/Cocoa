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

class ReactorBinding : public BindingBase
{
    GALLIUM_BINDING_OBJECT

public:
    ReactorBinding();
    ~ReactorBinding() override;

    void onRegisterClasses(v8::Isolate *isolate) override;

    ClassExport<GShaderBuilderWrap> gshader_builder_wrap_;
    ClassExport<GShaderModuleWrap>  gshader_module_wrap_;
};

/* JSDecl: class GShaderBuilder */
class GShaderBuilderWrap
{
public:
    /* JSDecl: constructor(name: string) */
    explicit GShaderBuilderWrap(const std::string& name);

    /* JSDecl: mainTestCodeGen(): void */
    void mainTestCodeGen();

    /* JSDecl: insertJSFunctionSymbol(func: Function, name: string): void */
    void insertJSFunctionSymbol(v8::Local<v8::Value> func, const std::string& name);

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
