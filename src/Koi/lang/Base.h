#ifndef COCOA_LANG_BASE_H
#define COCOA_LANG_BASE_H

#include <string>
#include <functional>

#include "Koi/KoiBase.h"
#include "Koi/binder/Module.h"

KOI_LANG_NS_BEGIN

class BaseBindingModule
{
public:
    BaseBindingModule(std::string name, std::string desc)
        : fName(std::move(name)),
          fDescription(std::move(desc)),
          fNext(nullptr) {}
    virtual ~BaseBindingModule()
    {
        delete fNext;
    }

    koi_nodiscard inline const std::string& name() const
    { return fName; }
    koi_nodiscard inline const std::string& description() const
    { return fDescription; }
    koi_nodiscard inline BaseBindingModule *next() const
    { return fNext; }

    inline void setNext(BaseBindingModule *next)
    { fNext = next; }

    virtual void getModule(binder::Module& mod) = 0;

private:
    std::string         fName;
    std::string         fDescription;
    BaseBindingModule  *fNext;
};

void RegisterBinding(BaseBindingModule *pModule);
BaseBindingModule *FindBindingByName(const std::string& name);
void ForEachBinding(const std::function<bool(BaseBindingModule*)>& cb,
                    BaseBindingModule *header = nullptr);

#define KOI_BINDING_PRELOAD_HOOK(name) _binding_##name##_preload_hook
#define KOI_DECL_BINDING_PRELOAD_HOOK(name) void KOI_BINDING_PRELOAD_HOOK(name)()

KOI_LANG_NS_END
#endif //COCOA_LANG_BASE_H
