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

#ifndef COCOA_GLAMOR_WAYLAND_SCREENCAST_PIPEWIRE_H
#define COCOA_GLAMOR_WAYLAND_SCREENCAST_PIPEWIRE_H

#include <pipewire/pipewire.h>
#include <spa/support/loop.h>
#include <spa/param/video/format.h>

#include "Screencast.h"
#include "MemoryTexture.h"
SCREENCAST_NAMESPACE_BEGIN

struct PipewireCursor
{
    bool visible;
    bool valid;
    int32_t x, y;
    int32_t hotspot_x;
    int32_t hotspot_y;

    // Pipewire delivers a bitmap of current cursor, and we just copy the contents
    // of that bitmap into this texture memory. Although the pixels will be sent
    // to the host process through the shared memory, we still cannot write pixels
    // to the shared memory directly. A simple reason of that is the racing condition,
    // which means that the host process may manipulate the contents of the shared memory
    // while we are writing pixels, and that will cause unexpected errors.
    // Consequently, we just store pixels here and when the host process tells us it is
    // ready to receive the cursor texture, which means it will not access the shared
    // memory until the transferring has been finished, we write pixels into the shared memory.
    std::shared_ptr<MemoryTexture> texture;
};

struct VideoCrop
{
    bool valid;
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
};

struct VideoTexture
{
    /* CPU buffer */
    std::shared_ptr<MemoryTexture> memory_texture;

    /* DMA buffer */
    constexpr static size_t kMaxDmaBufPlanes = 4;
    uint32_t  dma_width;
    uint32_t  dma_height;
    uint32_t  dma_drm_format;
    uint32_t  dma_n_planes;
    int32_t   dma_fds[kMaxDmaBufPlanes];
    uint32_t  dma_offsets[kMaxDmaBufPlanes];
    uint32_t  dma_strides[kMaxDmaBufPlanes];
    uint64_t  dma_modifiers[kMaxDmaBufPlanes];
};

class Pipewire
{
public:
    struct VersionTriple
    {
        g_nodiscard g_inline bool Check(int major_, int minor_, int micro_) const {
            if (major != major_)
                return major > major_;
            if (minor != minor_)
                return minor > minor_;
            return micro >= micro_;
        }

        int major;
        int minor;
        int micro;
    };

    Pipewire();
    ~Pipewire();

    g_nodiscard static std::shared_ptr<Pipewire> Make(int pipewire_fd,
                                                      uint32_t pipewire_node);

    g_nodiscard g_inline int GetServerVersionSync() const {
        return server_version_sync_;
    }

    g_nodiscard g_inline pw_thread_loop *GetThreadLoop() const {
        return thread_loop_;
    }

    g_nodiscard g_inline VersionTriple& GetServerVersion() {
        return server_version_;
    }

    g_nodiscard g_inline pw_stream *GetVideoStream() const {
        return video_stream_;
    }

    g_nodiscard g_inline spa_video_info& GetVideoInfo() {
        return video_info_;
    }

    g_nodiscard g_inline PipewireCursor& GetCursor() {
        return cursor_;
    }

    g_nodiscard g_inline VideoCrop& GetVideoCrop() {
        return video_crop_;
    }

    g_nodiscard g_inline VideoTexture& GetVideoTexture() {
        return video_texture_;
    }

    g_private_api bool UploadVideoTextureWithMetadata();

    void Activate();
    void Deactivate();

private:
    int                  pipewire_fd_;
    pw_thread_loop      *thread_loop_;
    pw_context          *context_;
    pw_core             *core_;
    spa_source          *renegotiate_;
    VersionTriple        server_version_;
    int                  server_version_sync_;
    pw_stream           *video_stream_;
    spa_hook             core_listener_;
    spa_hook             stream_listener_;
    spa_video_info       video_info_;
    PipewireCursor       cursor_;
    VideoCrop            video_crop_;
    VideoTexture         video_texture_;
};

SCREENCAST_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_SCREENCAST_PIPEWIRE_H
