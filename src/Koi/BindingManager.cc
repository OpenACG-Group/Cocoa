#include <dlfcn.h>
#include "Core/Errors.h"
#include <vector>

#include "Koi/BindingManager.h"
#include "Koi/bindings/Base.h"
#include "Koi/bindings/core/Exports.h"
#include "Core/Journal.h"

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Koi)

KOI_NS_BEGIN

BindingManager::BindingManager(const Runtime::Options& options)
    : fAllowOverride(options.rt_allow_override)
    , fBlacklist(options.bindings_blacklist)
{
    appendBinding(new bindings::CoreBinding());
}

BindingManager::~BindingManager()
{
    for (bindings::BindingBase *ptr : fBindings)
    {
        CHECK(ptr);
        QLOG(LOG_DEBUG, "Unloading binding {}:{}", ptr->name(), ptr->getUniqueId());
        delete ptr;
    }
    for (void *ptr : fLibHandles)
        ::dlclose(ptr);
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

    auto hook = reinterpret_cast<HookFunc>(::dlsym(pHandle, "__g_cocoa_hook"));
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
             ptr->name(), ptr->getUniqueId());
        return false;
    }
    for (auto itr = fBindings.begin(); itr != fBindings.end(); itr++)
    {
        if ((*itr)->name() == ptr->name())
        {
            if (!fAllowOverride)
            {
                QLOG(LOG_DEBUG, "Binding {}:{} won't be loaded because of name conflict",
                     ptr->name(), ptr->getUniqueId());
                return false;
            }
            QLOG(LOG_WARNING, "Binding {}:{} is overriden by {}",
                 (*itr)->name(), (*itr)->getUniqueId(), ptr->getUniqueId());
            *itr = ptr;
            return true;
        }
    }
    QLOG(LOG_DEBUG, "Binding {} is registered (import \'{}\')", ptr->getUniqueId(), ptr->name());
    fBindings.push_back(ptr);
    return true;
}

KOI_NS_END
