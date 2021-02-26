#include <memory>

#include "Ciallo/GrBase.h"
#include "Ciallo/DDR/GrBasePlatform.h"
#include "Ciallo/DDR/GrBaseCompositor.h"

CIALLO_BEGIN_NS

GrBasePlatform::GrBasePlatform(GrPlatformKind kind,
                               const GrPlatformOptions& opts)
    : fCompositor(nullptr),
      fKind(kind),
      fOptions(opts)
{
}

std::shared_ptr<GrBaseCompositor> GrBasePlatform::compositor()
{
    if (fCompositor == nullptr)
        fCompositor = onCreateCompositor();
    return fCompositor;
}

void GrBasePlatform::present(const uint8_t *pBuffer)
{
    onPresent(pBuffer);
}

CIALLO_END_NS
