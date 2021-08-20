#include "Koi/lang/Base.h"
KOI_LANG_NS_BEGIN

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
} // namespace anonymous

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

KOI_LANG_NS_END
