#include <iostream>
#include <sstream>
#include <thread>

#include "llvm/Support/TargetSelect.h"

#include "Core/Exception.h"
#include "Core/Journal.h"
#include "Vanilla/Base.h"
#include "Vanilla/Typetraits.h"
#include "Vanilla/Shader/ShaderExecutor.h"

#include "Vanilla/Shader/ShaderLibrary.h"
VANILLA_NS_BEGIN

#define SHADER_DYLIB_NAME   "libShader.so"

namespace {

constinit bool initialized = false;
void initialize_llvm()
{
    if (initialized)
        return;

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();

    initialized = true;
}

std::ostream& operator<<(std::ostream& os, VaShaderLDT::Feature feature)
{
    switch (feature)
    {
    case VaShaderLDT::Feature::kLang:
        os << "Feature::Lang";
        break;

    default:
        os << "Feature::Unknown";
        break;
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<VaShaderLDT::Feature>& features)
{
    bool first = true;
    for (auto f : features)
    {
        if (!first)
            os << ", ";
        os << f;
        first = false;
    }
    return os;
}

const VaShaderLDT *initialize_shader_library(const UniqueHandle<llvm::orc::LLJIT>& jit, llvm::orc::JITDylib& lib)
{
    auto expectSymbol = jit->lookup(lib, vas_mangle_name_str(global_ldt));
    if (!expectSymbol)
        throw VanillaException(__func__, "Failed to lookup LDT in shader library");

    auto *pLDT = reinterpret_cast<const VaShaderLDT*>(expectSymbol->getAddress());

    std::vector<VaShaderLDT::Feature> features(pLDT->featuresNum);
    for (int32_t i = 0; i < pLDT->featuresNum; i++)
        features[i] = pLDT->features[i];

    {
        std::ostringstream oss;
        oss << features;

        Journal::Ref()(LOG_INFO,
                   "Loaded shader library [LDT @ {}]:\n  Vendor: {}\n  Version: {}.{}.{}\n  Supported features: {}",
                       fmt::ptr(pLDT), pLDT->vendor,
                       pLDT->version[0], pLDT->version[1], pLDT->version[2], oss.str());
    }

    if (std::strcmp(pLDT->vendor, "org.OpenACG.Cocoa") != 0)
        Journal::Ref()(LOG_WARNING, "Loading an unofficial shader library may cause security problems.");

    Bitfield<VaShaderLDT::Feature> featureBits(features);
    if (!(featureBits & VaShaderLDT::Feature::kLang))
    {
        Journal::Ref()(LOG_ERROR,
                       "The shader library must support Feature::Lang feature for language specification.");
        throw VanillaException(__func__, "Unsupported shader library");
    }

    return pLDT;
}

} // namespace anonymous

Handle<ShaderExecutor> ShaderExecutor::Create()
{
    initialize_llvm();

    auto jit = llvm::orc::LLJITBuilder().setNumCompileThreads(2).create();
    if (!jit)
        throw VanillaException(__func__, "Failed to create a LLVM JIT instance");

    auto dylib = (*jit)->createJITDylib(SHADER_DYLIB_NAME);
    if (!dylib)
        throw VanillaException(__func__, "Failed to create a shared object for JIT");

    auto generator = llvm::orc::DynamicLibrarySearchGenerator::Load("./src/Vanilla/libShader.so",
                                                                    (*jit)->getDataLayout().getGlobalPrefix());
    if (!generator)
        throw VanillaException(__func__, "Failed to load libShader shared object");

    dylib->addGenerator(std::move(*generator));
    (*jit)->getMainJITDylib().addToLinkOrder(*dylib,
                                             llvm::orc::JITDylibLookupFlags::MatchExportedSymbolsOnly);
    const VaShaderLDT *ldt = initialize_shader_library(*jit, *dylib);

    return std::make_shared<ShaderExecutor>(std::move(*jit), ldt);
}

namespace {

void libshader_messenger(VaShaderHostInterface::MessageType type, const char *str, void *closure)
{
    LogType level;

    switch (type)
    {
    case VaShaderHostInterface::MessageType::kDebug:
        level = LOG_DEBUG;
        break;
    case VaShaderHostInterface::MessageType::kInfo:
        level = LOG_INFO;
        break;
    case VaShaderHostInterface::MessageType::kWarning:
        level = LOG_WARNING;
        break;
    case VaShaderHostInterface::MessageType::kError:
        level = LOG_ERROR;
        break;
    }

    Journal::Ref()(level, "%fg<hl><Shader>%reset {}", str);
}

uint32_t libshader_get_cpu_count(void *closure)
{
    return std::thread::hardware_concurrency();
}

} // namespace anonymous

ShaderExecutor::ShaderExecutor(UniqueHandle<llvm::orc::LLJIT>&& jit,
                               const VaShaderLDT *ldt)
    : fJit(std::move(jit)),
      fShaderLibrary(nullptr),
      fpLDT(ldt)
{
    fShaderLibrary = fJit->getJITDylibByName(SHADER_DYLIB_NAME);

    auto hostInterface = std::make_shared<VaShaderHostInterface>();
    hostInterface->closure = this;
    hostInterface->messenger = libshader_messenger;
    hostInterface->getCpuCount = libshader_get_cpu_count;

    if (fpLDT->initializer)
    {
        if (!fpLDT->initializer(std::move(hostInterface)))
            throw VanillaException(__func__, "Could not initialize shader library");
    }
}

ShaderExecutor::~ShaderExecutor()
{
    if (fpLDT->finalizer)
        fpLDT->finalizer();
}

VANILLA_NS_END
