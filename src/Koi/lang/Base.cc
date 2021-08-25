#include <dlfcn.h>
#include "Koi/lang/Base.h"
KOI_LANG_NS_BEGIN

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
    BaseBindingModule *pCurrent = &gChain;
    while (pCurrent->next())
        pCurrent = pCurrent->next();
    pCurrent->setNext(pModule);
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
