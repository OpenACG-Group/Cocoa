#include "atri.h"
namespace cocoa::koi::bindings {
void AtriBinding::getModule(binder::Module& __mod) {
    __mod.set("fma", atriRoboko);
}
namespace {
const char *__g_this_exports[] = {
   "fma",
   nullptr
};
const char *__g_unique_id = "c2a3ce62d70f9426";
} // namespace anonymous
const char **AtriBinding::getExports() { return __g_this_exports; }
const char *AtriBinding::getUniqueId() { return __g_unique_id; }
} // namespace cocoa::koi::bindings
