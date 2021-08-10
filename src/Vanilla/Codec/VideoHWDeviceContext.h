#ifndef COCOA_VIDEOHWDEVICECONTEXT_H
#define COCOA_VIDEOHWDEVICECONTEXT_H

#include <memory>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
}

#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class VideoHWDeviceContext
{
public:
    explicit VideoHWDeviceContext(::AVBufferRef *context);
    ~VideoHWDeviceContext();

    static std::unique_ptr<VideoHWDeviceContext> Make(const std::string& deviceName);

    ::AVBufferRef *ref();
    void unref();

private:
    ::AVBufferRef   *fContext;
};

VANILLA_NS_END
#endif //COCOA_VIDEOHWDEVICECONTEXT_H
