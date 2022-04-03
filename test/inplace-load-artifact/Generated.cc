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
