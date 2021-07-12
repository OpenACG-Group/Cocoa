#include <iostream>

#include "ShaderLibrary.h"

#define VAS_MAJOR   1
#define VAS_MINOR   0
#define VAS_PATCH   0

namespace {

std::shared_ptr<VaShaderHostInterface> hostInterface;

bool library_initializer(std::shared_ptr<VaShaderHostInterface> host) noexcept
{
    hostInterface = std::move(host);
    hostInterface->messenger(VaShaderHostInterface::MessageType::kInfo,
                             "Vanilla Shader Language Library is loaded and initialized",
                             hostInterface->closure);
    return true;
}

void library_finalizer() noexcept
{
}

} // namespace anonymous

std::shared_ptr<VaShaderHostInterface> GetHostInterface()
{
    return hostInterface;
}

VAS_EXPORTS_BEGIN

vas_exported void vas_mangle_name(print_int)(int a)
{
    std::cout << "shader_func: a=" << a << std::endl;
}

vas_exported VaShaderLDT vas_mangle_name(global_ldt) = {
        .vendor = "org.OpenACG.Cocoa",
        .version = { VAS_MAJOR, VAS_MINOR, VAS_PATCH },
        .featuresNum = 1,
        .features = {VaShaderLDT::Feature::kLang},
        .initializer = library_initializer,
        .finalizer = library_finalizer
};

VAS_EXPORTS_END
