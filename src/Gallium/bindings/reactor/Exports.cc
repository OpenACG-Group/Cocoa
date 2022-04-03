#include "Gallium/bindings/reactor/Exports.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_REACTOR_NS_BEGIN

ReactorBinding::ReactorBinding()
    : BindingBase("reactor", "LLVM Based Machine Code Generator")
{
    using P = reactor::Options::CodeOptPass;

    Bitfield<P> passes;
    passes |= P::kCFGSimplification;
    passes |= P::kLICM;
    passes |= P::kAggressiveDCE;
    passes |= P::kGVN;
    passes |= P::kInstructionCombining;
    passes |= P::kReassociate;
    passes |= P::kDeadStoreElimination;
    passes |= P::kSCCP;
    passes |= P::kSROA;
    passes |= P::kEarlyCSE;

    reactor::InitializePlatform({
        reactor::Options::CodeOptLevels::kDefault,
        passes
    });
}

ReactorBinding::~ReactorBinding()
{
    reactor::DisposePlatform();
}

void ReactorBinding::onRegisterClasses(v8::Isolate *isolate)
{
    gshader_builder_wrap_ = NewClassExport<GShaderBuilderWrap>(isolate);
    (*gshader_builder_wrap_)
        .constructor<const std::string&>()
        .set("mainTestCodeGen", &GShaderBuilderWrap::mainTestCodeGen)
        .set("insertJSFunctionSymbol", &GShaderBuilderWrap::insertJSFunctionSymbol);

    gshader_module_wrap_ = NewClassExport<GShaderModuleWrap>(isolate);
    (*gshader_module_wrap_)
        .set_static_func("Compile", GShaderModuleWrap::Compile)
        .set("executeMain", &GShaderModuleWrap::executeMain);
}

GALLIUM_BINDINGS_REACTOR_NS_END
