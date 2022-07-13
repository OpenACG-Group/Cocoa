#ifndef COCOA_GLAMOR_FRAMENOTIFICATIONROUTER_H
#define COCOA_GLAMOR_FRAMENOTIFICATIONROUTER_H

#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

// Abstracted interface for `Surface` and `RenderTarget` to notify the signal
// of frame updating.
class FrameNotificationRouter
{
public:
    virtual void OnFrameNotification(uint32_t sequence) = 0;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_FRAMENOTIFICATIONROUTER_H
