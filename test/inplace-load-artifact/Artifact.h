#ifndef COCOA_ARTIFACT_H
#define COCOA_ARTIFACT_H

#include "Gallium/bindings/Base.h"
GALLIUM_BINDINGS_NS_BEGIN

class ArtifactBinding : public BindingBase
{
public:
    ArtifactBinding() : BindingBase("artifact", "わたしわ、高性能ですから！") {}
    ~ArtifactBinding() override {}
    const char *onGetUniqueId() override;
    void onGetModule(binder::Module& self) override;
    const char **onGetExports() override;
};

void ArtifactTrampoline();

GALLIUM_BINDINGS_NS_END
#endif //COCOA_ARTIFACT_H
