#ifndef COCOA_LAYERPROPERTYGROUP_H
#define COCOA_LAYERPROPERTYGROUP_H

#include "include/core/SkRect.h"
#include "include/core/SkRRect.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkImageFilter.h"

#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class LayerPropertyGroup
{
public:
    enum class Clipping
    {
        kNone,
        kRRect,     /* Round rectangle clipping */
        kRect,      /* Normal rectangle clipping */
        kPath       /* Path clipping */
    };



private:
    uint32_t                 fWidth;
    uint32_t                 fHeight;
    Clipping                 fClipping;
    SkRect                   fClipRect;
    SkRRect                  fClipRRect;
    SkPath                   fClipPath;
    bool                     fHasMatrix;
    SkMatrix                 fMatrix;
    sk_sp<SkImageFilter>     fImageFilter;
    sk_sp<SkColorFilter>     fColorFilter;
};

VANILLA_NS_END
#endif //COCOA_LAYERPROPERTYGROUP_H
