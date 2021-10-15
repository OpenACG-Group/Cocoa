#include <dlfcn.h>
#include "Core/Journal.h"
#include "Core/Exception.h"
#include "Koi/BindingsLoader.h"
#include "Koi/lang/Base.h"
KOI_LANG_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Koi.lang)

void BaseBindingModule::dispose()
{
    if (fNext)
        fNext->dispose();
    delete fNext;
    fNext = nullptr;
}

class BindingChain : public BaseBindingModule
{
public:
    BindingChain()
        : BaseBindingModule("langBindingChainHeader",
                            "First element in the binding chain") {}
    ~BindingChain() override = default;
    void getModule(binder::Module& mod) override {}
};

namespace {
BindingChain gChain;
LbpBlock lbpBlock {.n_hooks = 0};
} // namespace anonymous

void LbpHookAppender::Append(LbpBlock::LbpHook hook, void *dynlib)
{
    assert(hook);
    if (lbpBlock.n_hooks >= LbpBlock::MAX_HOOKS)
    {
        throw std::runtime_error("koi::lang::LbpHookAppender: Too many LBP hooks");
    }
    lbpBlock.lbp_hooks[lbpBlock.n_hooks] = hook;
    lbpBlock.lbp_dynlib_handles[lbpBlock.n_hooks] = dynlib;
    ++lbpBlock.n_hooks;
}

const LbpBlock *LbpHookAppender::GetLbpBlock()
{
    return &lbpBlock;
}

void RegisterBinding(BaseBindingModule *pModule)
{
    if (pModule->name().empty())
    {
        delete pModule;
        throw RuntimeException(__func__, "Invalid language binding name");
    }

    std::string name = pModule->name();
    BaseBindingModule *pCurrent = gChain.next();
    BaseBindingModule *pPrev = &gChain;
    while (pCurrent)
    {
        /* Name conflicts with existing modules */
        if (pCurrent->name() == name)
        {
            if (!GetGlobalRuntimeOptions().lbp_allow_override)
            {
                delete pModule;
                LOGF(LOG_WARNING,
                     "%bg<re>%fg<hl>(Vulnerability)%reset Language binding name conflict ({}), use --lbp-allow-override to ignore this",
                     name)
                throw RuntimeException(__func__, "Language binding name conflict");
            }
            pPrev->setNext(pModule);
            pModule->setNext(pCurrent->next());
            delete pCurrent;
            return;
        }

        pPrev = pCurrent;
        pCurrent = pCurrent->next();
    }
    pPrev->setNext(pModule);
}

BaseBindingModule *FindBindingByName(const std::string& name)
{
    BaseBindingModule *pCurrent = &gChain;
    while (pCurrent)
    {
        if (pCurrent->name() == name)
            break;
        pCurrent = pCurrent->next();
    }
    return pCurrent;
}

void ForEachBinding(const std::function<bool(BaseBindingModule*)>& cb, BaseBindingModule *header)
{
    auto pCurrent = header ? header : gChain.next();
    while (pCurrent)
    {
        if (!cb(pCurrent))
            break;
        pCurrent = pCurrent->next();
    }
}

void Dispose()
{
    gChain.dispose();
    for (size_t i = 0; i < lbpBlock.n_hooks; i++)
    {
        if (lbpBlock.lbp_dynlib_handles[i])
            ::dlclose(lbpBlock.lbp_dynlib_handles[i]);
    }
}

KOI_LANG_NS_END
