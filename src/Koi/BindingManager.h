#ifndef COCOA_BINDINGMANAGER_H
#define COCOA_BINDINGMANAGER_H

#include <vector>

#include "Core/UniquePersistent.h"
#include "Koi/KoiBase.h"
#include "Koi/Runtime.h"
KOI_NS_BEGIN

namespace lang { class BindingBase; }

class BindingManager : public UniquePersistent<BindingManager>
{
public:
    explicit BindingManager(const Runtime::Options& options);
    ~BindingManager();

    koi_nodiscard inline bool isAllowOverride() const {
        return fAllowOverride;
    }

    void loadDynamicObject(const std::string& path);
    lang::BindingBase *search(const std::string& name);

private:
    bool appendBinding(lang::BindingBase *ptr);

    bool                            fAllowOverride;
    std::vector<std::string>        fBlacklist;
    std::vector<lang::BindingBase*> fBindings;
    std::vector<void*>              fLibHandles;
};

KOI_NS_END
#endif //COCOA_BINDINGMANAGER_H
