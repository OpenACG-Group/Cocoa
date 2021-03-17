#ifndef COCOA_CRIMAGESURFACE_H
#define COCOA_CRIMAGESURFACE_H

#include "Ciallo/Cairo2d/CrSurface.h"
CIALLO_BEGIN_NS

class CrImageSurface : public CrSurface
{
public:
    CrImageSurface(int32_t width, int32_t height);
    explicit CrImageSurface(const std::string& file);
    ~CrImageSurface() override = default;

    int32_t width() override;
    int32_t height() override;
    int32_t stride() override;
    uint8_t *peekPixels() override;
};

CIALLO_END_NS
#endif //COCOA_CRIMAGESURFACE_H
