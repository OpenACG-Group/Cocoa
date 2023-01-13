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

#ifndef COCOA_UTAU_HWDEVICECONTEXT_H
#define COCOA_UTAU_HWDEVICECONTEXT_H

#include <va/va.h>

#define FFWRAP_AVUTIL_USE_HWCONTEXT_VAAPI

#include "Utau/Utau.h"
#include "Utau/ffwrappers/libavutil.h"
UTAU_NAMESPACE_BEGIN

class HWDeviceContext
{
public:
    struct DevicePriv;

    HWDeviceContext();
    ~HWDeviceContext() = default;

    g_nodiscard static std::shared_ptr<HWDeviceContext> MakeVAAPI();

    g_nodiscard AVBufferRef *GetAVContext();

    g_nodiscard AVPixelFormat GetDeviceFormat();

    g_nodiscard VADisplay GetVADisplay();

private:
    std::unique_ptr<DevicePriv> priv_;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_HWDEVICECONTEXT_H
