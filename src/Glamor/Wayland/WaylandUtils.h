#ifndef COCOA_GLAMOR_WAYLAND_WAYLANDUTILS_H
#define COCOA_GLAMOR_WAYLAND_WAYLANDUTILS_H

#include <wayland-client-protocol.h>

#include "include/core/SkColor.h"
#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

SkColorType WlShmFormatToSkColorType(wl_shm_format type);
wl_shm_format SkColorTypeToWlShmFormat(SkColorType type);

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_WAYLANDUTILS_H
