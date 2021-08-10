#include "Vanilla/Base.h"
#include "Vanilla/Codec/VideoHWDeviceContext.h"
VANILLA_NS_BEGIN

std::unique_ptr<VideoHWDeviceContext> VideoHWDeviceContext::Make(const std::string& deviceName)
{
    ::AVBufferRef *pHwDeviceContext = nullptr;
    int32_t ret = ::av_hwdevice_ctx_create(&pHwDeviceContext,
                                           AV_HWDEVICE_TYPE_VAAPI,
                                           deviceName.c_str(),
                                           nullptr,
                                           0);
    if (ret < 0 || !pHwDeviceContext)
        return nullptr;
    return std::make_unique<VideoHWDeviceContext>(pHwDeviceContext);
}

VideoHWDeviceContext::VideoHWDeviceContext(::AVBufferRef *context)
    : fContext(context)
{
}

VideoHWDeviceContext::~VideoHWDeviceContext()
{
    while (fContext)
        ::av_buffer_unref(&fContext);
}

::AVBufferRef *VideoHWDeviceContext::ref()
{
    return ::av_buffer_ref(fContext);
}

void VideoHWDeviceContext::unref()
{
    ::av_buffer_unref(&fContext);
}

VANILLA_NS_END
