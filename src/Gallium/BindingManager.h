#ifndef COCOA_GALLIUM_BINDINGMANAGER_H
#define COCOA_GALLIUM_BINDINGMANAGER_H

#include <vector>

#include "Core/UniquePersistent.h"
#include "Gallium/Gallium.h"
#include "Gallium/Runtime.h"
GALLIUM_NS_BEGIN

namespace bindings { class BindingBase; }

class BindingManager : public UniquePersistent<BindingManager>
{
public:
    explicit BindingManager(const Runtime::Options& options);
    ~BindingManager();

    gal_nodiscard inline bool isAllowOverride() const {
        return fAllowOverride;
    }

    void loadDynamicObject(const std::string& path);
    bindings::BindingBase *search(const std::string& name);

    static void NotifyIsolateHasCreated(v8::Isolate *isolate);

private:
    bool appendBinding(bindings::BindingBase *ptr);

    bool                            fAllowOverride;
    std::vector<std::string>        fBlacklist;
    std::vector<bindings::BindingBase*> fBindings;
    std::vector<void*>              fLibHandles;
};

GALLIUM_NS_END
#endif //COCOA_GALLIUM_BINDINGMANAGER_H
