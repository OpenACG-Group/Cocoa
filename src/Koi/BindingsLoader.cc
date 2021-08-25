#include "Core/Project.h"
#include "Core/Journal.h"

#include "Koi/KoiBase.h"
#include "Koi/lang/Base.h"
#include "Koi/lang/CoreBinding.h"

#include <dlfcn.h>
KOI_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Koi)

void PreloadBindings()
{
    lang::RegisterBinding(new lang::CoreBindingModule());

    const lang::LbpBlock *block = lang::LbpHookAppender::GetLbpBlock();
    for (size_t i = 0; i < block->n_hooks; i++)
    {
        if (!block->lbp_hooks[i])
        {
            LOGF(LOG_DEBUG, "(LBP) Skipped a hook function in LbpBlock {}", fmt::ptr(block))
            continue;
        }
        LOGF(LOG_DEBUG, "(LBP) Calling hook function {} in LbpBlock {}",
             fmt::ptr(block->lbp_hooks[i]), fmt::ptr(block))
        block->lbp_hooks[i](block);
    }
}

bool LoadBindingsFromDynamicLibrary(const std::string& file)
{
    void *handle = ::dlopen(file.c_str(), RTLD_LOCAL | RTLD_LAZY);
    if (!handle)
    {
        LOGF(LOG_ERROR, "Failed to load language bindings: {}", ::dlerror())
        return false;
    }

    auto hook = reinterpret_cast<lang::LbpBlock::LbpHook>(::dlsym(handle,
                                                                  LANG_BINDING_HOOK_SYM_STR));
    if (!hook)
    {
        ::dlclose(handle);
        LOGF(LOG_ERROR, "Failed to load language bindings from dynamic library {}: No hook function",
             file)
        return false;
    }
    lang::LbpHookAppender::Append(hook, handle);
    return true;
}

KOI_NS_END
