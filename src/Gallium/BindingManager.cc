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

#include <dlfcn.h>
#include "Core/Errors.h"
#include <vector>

#include "Core/Journal.h"
#include "Gallium/BindingManager.h"
#include "Gallium/bindings/Base.h"
#include "Gallium/bindings/EventEmitter.h"

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.BindingManager)

GALLIUM_NS_BEGIN

// This symbol is defined in generated source file.
// See `scripts/modulec.py` and `scripts/collect-internal-bindings.py` for details.
namespace bindings {
extern std::vector<bindings::BindingBase *> on_register_internal_bindings();
}

BindingManager::BindingManager(const Runtime::Options& options)
    : fAllowOverride(options.rt_allow_override)
    , fBlacklist(options.bindings_blacklist)
{
    // appendBinding(bindings::on_register_module_core());
    // appendBinding(new bindings::glamor_wrap::GlamorBinding());
    for (auto *ptr : bindings::on_register_internal_bindings())
    {
        CHECK(appendBinding(ptr) && "Failed in registering bindings");
    }
}

BindingManager::~BindingManager()
{
    for (bindings::BindingBase *ptr : fBindings)
    {
        CHECK(ptr);
        QLOG(LOG_DEBUG, "Unloading binding {}:{}", ptr->name(), ptr->onGetUniqueId());
        delete ptr;
    }
    for (void *ptr : fLibHandles)
        ::dlclose(ptr);
}

void BindingManager::NotifyIsolateHasCreated(v8::Isolate *isolate)
{
    bindings::EventEmitterBase::RegisterClass(isolate);
}

bindings::BindingBase *BindingManager::search(const std::string& name)
{
    for (bindings::BindingBase *ptr : fBindings)
    {
        if (ptr->name() == name)
            return ptr;
    }
    return nullptr;
}

void BindingManager::loadDynamicObject(const std::string& path)
{
    using HookFunc = bindings::BindingBase*(*)();
    void *pHandle = ::dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL);
    if (!pHandle)
    {
        throw RuntimeException(__func__,
                               fmt::format("Failed to load dynamic object {}", ::dlerror()));
    }

    auto hook = reinterpret_cast<HookFunc>(::dlsym(pHandle, DYN_HOOK_SYMBOL_STRING));
    if (!hook)
    {
        ::dlclose(pHandle);
        throw RuntimeException(__func__,
                               fmt::format("Failed to resolve hook function in {}", path));
    }
    // TODO: Passing some information about current runtime to hook function
    bindings::BindingBase *ptr = hook();
    if (!ptr)
    {
        delete ptr;
        ::dlclose(pHandle);
        throw RuntimeException(__func__,
                               fmt::format("Shared object {} cannot produce a valid binding object", path));
    }
    if (!appendBinding(ptr))
    {
        delete ptr;
        ::dlclose(pHandle);
    }
    else
        fLibHandles.push_back(pHandle);
}

bool BindingManager::appendBinding(bindings::BindingBase *ptr)
{
    auto pos = std::find(fBlacklist.begin(), fBlacklist.end(), ptr->name());
    if (pos != fBlacklist.end())
    {
        QLOG(LOG_DEBUG, "Binding {}:{} is blocked because of blacklist",
             ptr->name(), ptr->onGetUniqueId());
        return false;
    }
    for (auto & fBinding : fBindings)
    {
        if (fBinding->name() == ptr->name())
        {
            if (!fAllowOverride)
            {
                QLOG(LOG_DEBUG, "Binding {}:{} won't be loaded because of name conflict",
                     ptr->name(), ptr->onGetUniqueId());
                return false;
            }
            QLOG(LOG_WARNING, "Binding {}:{} is override by {}",
                 fBinding->name(), fBinding->onGetUniqueId(), ptr->onGetUniqueId());
            fBinding = ptr;
            return true;
        }
    }
    QLOG(LOG_DEBUG, "Binding {} is registered (import \'{}\')", ptr->onGetUniqueId(), ptr->name());
    fBindings.push_back(ptr);
    return true;
}

GALLIUM_NS_END
