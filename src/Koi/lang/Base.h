#ifndef COCOA_LANG_BASE_H
#define COCOA_LANG_BASE_H

#include <string>
#include <functional>

#include "Koi/KoiBase.h"
#include "Koi/binder/Module.h"

KOI_LANG_NS_BEGIN

#define CHECK_AND_JS_THROW(cond, msg) \
    do {                                  \
        if ((cond)) {                     \
            binder::throw_(v8::Isolate::GetCurrent(), msg);  \
        } \
    } while (false)

#define CHECK_AND_JS_THROW_WITH_RET(cond, msg, ret) \
    do {                                  \
        if ((cond)) {                     \
            binder::throw_(v8::Isolate::GetCurrent(), msg); \
            return ret; \
        } \
    } while (false)

class BaseBindingModule
{
public:
    BaseBindingModule(std::string name, std::string desc)
        : fName(std::move(name)),
          fDescription(std::move(desc)),
          fNext(nullptr) {}
    virtual ~BaseBindingModule() = default;

    static void DefaultDeleter(BaseBindingModule *ptr) {
        delete ptr;
    }

    void dispose();

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
void Dispose();

struct LbpBlock
{
    using LbpHook = void(*)(const LbpBlock*);
    static constexpr size_t MAX_HOOKS = 128;

    LbpHook     lbp_hooks[MAX_HOOKS];
    void       *lbp_dynlib_handles[MAX_HOOKS];
    size_t      n_hooks;
};

class LbpHookAppender
{
public:
    koi_nodiscard static const LbpBlock *GetLbpBlock();
    static void Append(LbpBlock::LbpHook hook, void *dynlib);
};

#define _LANG_STRINGIFY(x)                #x
#define LANG_STRINGIFY(s)           _LANG_STRINGIFY(s)
#define LANG_BINDING_HOOK_SYM       _lbp_hook
#define LANG_BINDING_HOOK_SYM_STR   LANG_STRINGIFY(LANG_BINDING_HOOK_SYM)

KOI_LANG_NS_END
#endif //COCOA_LANG_BASE_H
