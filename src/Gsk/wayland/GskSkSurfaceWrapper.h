#ifndef COCOA_GSKSKSURFACEWRAPPER_H
#define COCOA_GSKSKSURFACEWRAPPER_H

#include <wayland-client-protocol.h>
#include "include/core/SkSurface.h"

#include "Gsk/Gsk.h"
GSK_NAMESPACE_BEGIN

class GskWaylandDisplay;

struct WaylandSharedMemoryBuffer
{
    using wpointer = void*;

    wpointer                    data;
    size_t                      length;
    wl_shm_pool                *pool;
    wl_buffer                  *buffer;
    Weak<GskWaylandDisplay>     display;
    uint32_t                    scale;
};

/**
 * This wrapper class packs a SkSurface and a wayland shared-memory-based
 * buffer together.
 * There must NOT be any other objects owns the SkSurface object when
 * GskSkSurfaceWrapper destructs.
 */
class GskSkSurfaceWrapper
{
public:
    GskSkSurfaceWrapper(const sk_sp<SkSurface>& surface,
                        WaylandSharedMemoryBuffer *buffer)
        : fSurface(surface), fBuffer(buffer) {}
    ~GskSkSurfaceWrapper() {
        CHECK(fSurface->unique() && "SkSurface could not be destroyed immediately");
        fSurface.reset();
    }

    g_nodiscard g_inline sk_sp<SkSurface> getSurface() const {
        return fSurface;
    }

    g_nodiscard g_inline WaylandSharedMemoryBuffer *getBuffer() const {
        return fBuffer;
    }

private:
    sk_sp<SkSurface>                    fSurface;
    WaylandSharedMemoryBuffer          *fBuffer;
};

GSK_NAMESPACE_END
#endif //COCOA_GSKSKSURFACEWRAPPER_H
