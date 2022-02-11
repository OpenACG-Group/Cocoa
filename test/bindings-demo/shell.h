#include "Koi/bindings/Base.h"
KOI_BINDINGS_NS_BEGIN

class ShellBinding : public BindingBase
{
public:
    ShellBinding()
        : BindingBase("core", "A language binding demo") {}
    ~ShellBinding() override = default;

    const char *onGetUniqueId() override;
    void onGetModule(binder::Module& module) override;
    const char **onGetExports() override;
};

uint8_t execute(const std::string& path);

KOI_BINDINGS_NS_END
