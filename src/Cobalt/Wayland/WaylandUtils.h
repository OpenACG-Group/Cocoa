#ifndef COCOA_COBALT_WAYLAND_WAYLANDUTILS_H
#define COCOA_COBALT_WAYLAND_WAYLANDUTILS_H

#include <wayland-client-protocol.h>

#include "include/core/SkColor.h"
#include "Cobalt/Cobalt.h"
COBALT_NAMESPACE_BEGIN

SkColorType WlShmFormatToSkColorType(wl_shm_format type);
wl_shm_format SkColorTypeToWlShmFormat(SkColorType type);

COBALT_NAMESPACE_END
#endif //COCOA_COBALT_WAYLAND_WAYLANDUTILS_H
