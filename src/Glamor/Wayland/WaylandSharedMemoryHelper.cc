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

#include <linux/memfd.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#include "Core/Journal.h"
#include "Core/Errors.h"
#include "Glamor/Wayland/WaylandSharedMemoryHelper.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Wayland.SharedMemoryHelper)

namespace {

// Reference: GTK+ Project (gdk/wayland/gdkdisplay-wayland.c)
int create_shared_memory_fd(const char *id)
{
    static bool force_shm_open = false;
    int ret;

#if !defined(__NR_memfd_create)
    forceShmOpen = true;
#endif

    do
    {
#if defined(__NR_memfd_create)
        if (!force_shm_open)
        {
            int options = MFD_CLOEXEC;
#if defined(MFD_ALLOW_SEALING)
            options = MFD_ALLOW_SEALING;
#endif /* MFD_ALLOW_SEALING */
            ret = memfd_create(id, options);

            if (ret < 0 && errno == ENOSYS)
                force_shm_open = true;
#if defined(F_ADD_SEALS) && defined(F_SEAL_SHRINK)
            if (ret >= 0)
                fcntl(ret, F_ADD_SEALS, F_SEAL_SHRINK);
#endif /* F_ADD_SEALS && F_SEAL_SHRINK */
        }
#endif /* __NR_memfd_create */

        if (force_shm_open)
        {
            auto name = fmt::format("/{}#{:x}", id, random());
            ret = shm_open(name.c_str(), O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, 0600);
            if (ret >= 0)
                shm_unlink(name.c_str());
            else if (errno == EEXIST)
                continue;
        }
    } while (ret < 0 && errno == EINTR);

    if (ret < 0)
    {
        QLOG(LOG_ERROR, "Creating shared memory file (using {}) failed: {}",
             force_shm_open ? "shm_open" : "memfd_create", strerror(errno));
    }

    return ret;
}

wl_shm_pool *create_shm_pool(wl_shm *shm, size_t size, void **data_ptr_out,
                             const char *id)
{
    *data_ptr_out = nullptr;

    int fd = create_shared_memory_fd(id);
    if (fd < 0)
        return nullptr;

    if (ftruncate(fd, static_cast<off_t>(size)) < 0)
    {
        QLOG(LOG_ERROR, "Truncating shared memory file failed: {}", strerror(errno));
        close(fd);
        return nullptr;
    }

    void *data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED)
    {
        QLOG(LOG_ERROR, "Mapping shared memory file failed: {}", strerror(errno));
        close(fd);
        return nullptr;
    }

    wl_shm_pool *pool = wl_shm_create_pool(shm, fd, static_cast<int32_t>(size));
    close(fd);

    *data_ptr_out = data;
    return pool;
}

const char *g_roles_id_name_map[] = {
    "cocoa-wayland-rendertarget",
    "cocoa-wayland-cursor",
    "cocoa-wayland-generic"
};

} // namespace anonymous

Shared<WaylandSharedMemoryHelper> WaylandSharedMemoryHelper::Make(wl_shm *shm,
                                                                  size_t size,
                                                                  BufferRole role)
{
    CHECK(shm);

    const char *role_id = g_roles_id_name_map[role];

    void *vma_mapped_ptr;
    wl_shm_pool *pool = create_shm_pool(shm, size, &vma_mapped_ptr, role_id);
    if (!pool)
        return nullptr;

    return std::make_shared<WaylandSharedMemoryHelper>(shm, pool, size, vma_mapped_ptr);
}

WaylandSharedMemoryHelper::WaylandSharedMemoryHelper(wl_shm *shm, wl_shm_pool *pool,
                                                     size_t size, void *ptr)
    : shm_registry_(shm)
    , shm_pool_(pool)
    , pool_size_(size)
    , vma_mapped_address_(ptr)
{
}

WaylandSharedMemoryHelper::~WaylandSharedMemoryHelper()
{
    wl_shm_pool_destroy(shm_pool_);
    munmap(vma_mapped_address_, pool_size_);
}

GLAMOR_NAMESPACE_END
