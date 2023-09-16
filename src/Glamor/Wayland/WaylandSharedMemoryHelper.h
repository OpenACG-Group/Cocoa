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

#ifndef COCOA_GLAMOR_WAYLAND_WAYLANDSHAREDMEMORYHELPER_H
#define COCOA_GLAMOR_WAYLAND_WAYLANDSHAREDMEMORYHELPER_H

#include <wayland-client.h>

#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

/**
 * Some wayland mechanisms depend on the interprocess shared memory to
 * exchange data with the wayland compositor. In this way to exchange data,
 * the client and the compositor respectively hold a file descriptor which
 * refers to the same physical memory area, and they are expected to map
 * that file descriptor to their own virtual memory address space.
 * The client can send or receive large data through the shared memory
 * instead of relying on the unix domain socket.
 *
 * The client can create `wl_buffer` objects from shared memory pool and
 * use them as surfaces to display or for other purposes.
 */
class WaylandSharedMemoryHelper
{
public:
    enum BufferRole
    {
        kRasterRenderTarget_Role = 0,
        kCursorSurface_Role      = 1,
        kGeneric_Role            = 3
    };

    WaylandSharedMemoryHelper(wl_shm *shm, wl_shm_pool *pool, size_t size, void *ptr);
    ~WaylandSharedMemoryHelper();

    g_nodiscard static std::shared_ptr<WaylandSharedMemoryHelper>
    Make(wl_shm *shm, size_t size, BufferRole role);

    g_nodiscard g_inline wl_shm *GetShm() const {
        return shm_registry_;
    }

    g_nodiscard g_inline wl_shm_pool *GetShmPool() const {
        return shm_pool_;
    }

    g_nodiscard g_inline size_t GetPoolSize() const {
        return pool_size_;
    }

    g_nodiscard g_inline void *GetMappedAddress() const {
        return vma_mapped_address_;
    }

private:
    wl_shm              *shm_registry_;
    wl_shm_pool         *shm_pool_;
    size_t               pool_size_;
    void                *vma_mapped_address_;

};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_WAYLANDSHAREDMEMORYHELPER_H
