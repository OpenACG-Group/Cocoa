#include "Artifact.h"

namespace cocoa::gallium::bindings {
void ArtifactBinding::onGetModule(binder::Module& __mod) {
    __mod.set("__trampoline", ArtifactTrampoline);
}
namespace {
const char *__g_this_exports[] = {
        "__trampoline",
        nullptr
};
const char *__g_unique_id = "ffdc5c71bccd2fe5";
} // namespace anonymous
const char **ArtifactBinding::onGetExports() { return __g_this_exports; }
const char *ArtifactBinding::onGetUniqueId() { return __g_unique_id; }
} // namespace cocoa::gallium::bindings

GALLIUM_BINDINGS_NS_BEGIN

extern "C" void __artifact_main();

void ArtifactTrampoline()
{
    __artifact_main();
}

GALLIUM_BINDING_LOADER_HOOK
{
    GALLIUM_HOOK_RET(new ArtifactBinding());
}

GALLIUM_BINDINGS_NS_END
