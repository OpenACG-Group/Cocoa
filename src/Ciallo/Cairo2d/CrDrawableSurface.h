#ifndef COCOA_CRDRAWABLESURFACE_H
#define COCOA_CRDRAWABLESURFACE_H

#include "Ciallo/Drawable.h"
#include "Ciallo/Cairo2d/CrSurface.h"
CIALLO_BEGIN_NS

class CrDrawableSurface : public CrSurface
{
public:
    explicit CrDrawableSurface(Drawable *drawable);
    ~CrDrawableSurface() override = default;

    int32_t width() override;
    int32_t height() override;
    void resize(int w, int h) override;

private:
    Drawable    *fDrawable;
};

CIALLO_END_NS
#endif //COCOA_CRDRAWABLESURFACE_H
