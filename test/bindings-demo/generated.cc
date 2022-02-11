#include "shell.h"
namespace cocoa::koi::bindings {
void ShellBinding::getModule(binder::Module& __mod) {
    __mod.set("execute", execute);
}
namespace {
const char *__g_this_exports[] = {
   "execute",
   nullptr
};
const char *__g_unique_id = "5703d20298c84e1d";
} // namespace anonymous
const char **ShellBinding::getExports() { return __g_this_exports; }
const char *ShellBinding::getUniqueId() { return __g_unique_id; }
} // namespace cocoa::koi::bindings
