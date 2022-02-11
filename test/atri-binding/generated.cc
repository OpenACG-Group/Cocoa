#include "atri.h"
namespace cocoa::koi::bindings {
void AtriBinding::getModule(binder::Module& __mod) {
    __mod.set("wakeup", ATRIWakeup);
}
namespace {
const char *__g_this_exports[] = {
   "wakeup",
   nullptr
};
const char *__g_unique_id = "200337a1ddf2877f";
} // namespace anonymous
const char **AtriBinding::getExports() { return __g_this_exports; }
const char *AtriBinding::getUniqueId() { return __g_unique_id; }
} // namespace cocoa::koi::bindings
