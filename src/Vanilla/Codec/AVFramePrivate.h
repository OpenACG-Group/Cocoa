#ifndef COCOA_AVFRAMEPRIVATE_H
#define COCOA_AVFRAMEPRIVATE_H

extern "C" {
#include <libavutil/frame.h>
}

#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

struct AVFrame::AVFramePrivate
{
    ::AVRational     timeBase;
    ::AVFrame       *pFrame;
};

VANILLA_NS_END
#endif //COCOA_AVFRAMEPRIVATE_H
