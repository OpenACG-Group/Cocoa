#ifndef COCOA_CRRECORDINGSURFACE_H
#define COCOA_CRRECORDINGSURFACE_H

#include "Ciallo/Cairo2d/CrSurface.h"
CIALLO_BEGIN_NS

class CrRecordingSurface : public CrSurface
{
public:
    CrRecordingSurface(cairo_content_t content, const CrRect& cullRect);
    explicit CrRecordingSurface(cairo_content_t content);
    ~CrRecordingSurface() override = default;

    int32_t width() override;
    int32_t height() override;
};

CIALLO_END_NS
#endif //COCOA_CRRECORDINGSURFACE_H
