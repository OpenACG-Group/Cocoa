#include "Core/Journal.h"
#include "Koi/ResourceObject.h"
#include "Koi/Runtime.h"
KOI_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Koi)

ResourceObject::ResourceObject(Runtime *runtime)
    : fRuntime(runtime),
      fRID(-1),
      fState(State::kReady)
{
}

ResourceObject::~ResourceObject()
{
    LOGF(LOG_DEBUG, "Resource RID:{} has been destroyed", fRID)
}

void ResourceObject::setRID(RID rid)
{
    fRID = rid;
    LOGF(LOG_DEBUG, "Resource RID:{} has been created with attached Runtime@{}",
         fRID, fmt::ptr(fRuntime))
}

KOI_NS_END
