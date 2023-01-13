/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

#include <unistd.h>
#include <fcntl.h>

#include <xf86drm.h>
#include <va/va_drm.h>

#define FFWRAP_AVUTIL_USE_HWCONTEXT_VAAPI
#include "Utau/ffwrappers/libavutil.h"
#include "Utau/ffwrappers/libavcodec.h"

#include "Core/Journal.h"
#include "Core/Exception.h"
#include "Core/Errors.h"
#include "Utau/HWDeviceContext.h"
UTAU_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Utau.HWDeviceContext)

struct HWDeviceContext::DevicePriv
{
    ~DevicePriv() {
        if (hwdevice_context)
        {
            CHECK(av_buffer_get_ref_count(hwdevice_context) == 1);
            av_buffer_unref(&hwdevice_context);
        }
        if (va_display)
            vaTerminate(va_display);

        if (drm_fd >= 0)
            close(drm_fd);
    }

    int32_t             drm_fd = -1;
    VADisplay           va_display = nullptr;
    AVBufferRef        *hwdevice_context = nullptr;
};

std::shared_ptr<HWDeviceContext> HWDeviceContext::MakeVAAPI()
{
    auto context = std::make_unique<HWDeviceContext>();
    auto& ctx_priv = context->priv_;

    // Query and open a DRM device
    const ContextOptions& options = GlobalContext::Ref().GetOptions();

    const char *device_path = "/dev/dri/renderD128";
    if (!options.hwdevice_drm_device_path.empty())
        device_path = options.hwdevice_drm_device_path.c_str();

    ctx_priv->drm_fd = ::open(device_path, O_RDWR | O_CLOEXEC);

    if (ctx_priv->drm_fd < 0)
    {
        QLOG(LOG_ERROR, "Failed to open an available DRM device");
        return nullptr;
    }

    QLOG(LOG_INFO, "Using DRM device \"{}\" for video hardware acceleration", device_path);

    // Initialize VAAPI context
    ctx_priv->va_display = vaGetDisplayDRM(ctx_priv->drm_fd);
    if (!ctx_priv->va_display)
    {
        QLOG(LOG_ERROR, "Failed to open VAAPI display");
        return nullptr;
    }

    int va_major, va_minor;
    VAStatus varet = vaInitialize(ctx_priv->va_display, &va_major, &va_minor);
    if (varet != VA_STATUS_SUCCESS)
    {
        QLOG(LOG_ERROR, "Failed to initialize VAAPI: {}", vaErrorStr(varet));
        return nullptr;
    }

    QLOG(LOG_INFO, "Using VAAPI for hardware acceleration, version {}.{}", va_major, va_minor);

    // Create libva hwdevice context
    ctx_priv->hwdevice_context = av_hwdevice_ctx_alloc(AV_HWDEVICE_TYPE_VAAPI);
    if (!ctx_priv->hwdevice_context)
    {
        QLOG(LOG_ERROR, "Failed in av_hwdevice_ctx_alloc, unsupported device type?");
        return nullptr;
    }

    auto *hwctx = reinterpret_cast<AVHWDeviceContext*>(
            ctx_priv->hwdevice_context->data);
    auto *va_hwctx = reinterpret_cast<AVVAAPIDeviceContext*>(hwctx->hwctx);
    va_hwctx->display = ctx_priv->va_display;

    int32_t ret = av_hwdevice_ctx_init(ctx_priv->hwdevice_context);
    if (ret < 0)
    {
        QLOG(LOG_ERROR, "Failed to initialize libav hardware device context");
        return nullptr;
    }

    return context;
}

HWDeviceContext::HWDeviceContext()
    : priv_(std::make_unique<DevicePriv>())
{
}

AVBufferRef *HWDeviceContext::GetAVContext()
{
    return priv_->hwdevice_context;
}

AVPixelFormat HWDeviceContext::GetDeviceFormat()
{
    return AV_PIX_FMT_VAAPI;
}

VADisplay HWDeviceContext::GetVADisplay()
{
    return priv_->va_display;
}

UTAU_NAMESPACE_END
