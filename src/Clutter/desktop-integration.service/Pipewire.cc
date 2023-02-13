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

#include <fcntl.h>
#include <pipewire/pipewire.h>
#include <spa/utils/type.h>
#include <spa/pod/builder.h>
#include <spa/param/format.h>
#include <spa/param/format-utils.h>
#include <spa/param/video/format-utils.h>
#include <spa/param/video/format.h>
#include <spa/debug/types.h>
#include <libdrm/drm_fourcc.h>

#include <unordered_map>
#include <vector>

#include <glib.h>

#include "fmt/format.h"

#include "Screencast.h"
#include "Pipewire.h"
#include "Errors.h"
SCREENCAST_NAMESPACE_BEGIN

#define CURSOR_META_SIZE(width, height)                                    \
	    (sizeof(struct spa_meta_cursor) + sizeof(struct spa_meta_bitmap) + \
         (width) * (height) * 4)

namespace {

spa_video_format texture_format_to_spa(TextureFormat fmt)
{
    switch (fmt)
    {
    case TextureFormat::kUnknown:
        CHECK_FAILED("Invalid texture format");
    case TextureFormat::kBGRA:
        return SPA_VIDEO_FORMAT_BGRA;
    case TextureFormat::kRGBA:
        return SPA_VIDEO_FORMAT_RGBA;
    case TextureFormat::kBGRX:
        return SPA_VIDEO_FORMAT_BGRx;
    case TextureFormat::kRGBX:
        return SPA_VIDEO_FORMAT_RGBx;
    }
}

TextureFormat spa_format_to_texture_format(spa_video_format fmt)
{
    switch (fmt)
    {
    case SPA_VIDEO_FORMAT_BGRA:
        return TextureFormat::kBGRA;
    case SPA_VIDEO_FORMAT_RGBA:
        return TextureFormat::kRGBA;
    case SPA_VIDEO_FORMAT_BGRx:
        return TextureFormat::kBGRX;
    case SPA_VIDEO_FORMAT_RGBx:
        return TextureFormat::kRGBX;
    default:
        return TextureFormat::kUnknown;
    }
}

uint32_t texture_format_to_drm_format(TextureFormat fmt)
{
    switch (fmt)
    {
    case TextureFormat::kUnknown:
        CHECK_FAILED("Invalid texture format");
    case TextureFormat::kBGRA:
        return DRM_FORMAT_ARGB8888;
    case TextureFormat::kRGBA:
        return DRM_FORMAT_ABGR8888;
    case TextureFormat::kBGRX:
        return DRM_FORMAT_XRGB8888;
    case TextureFormat::kRGBX:
        return DRM_FORMAT_XBGR8888;
    }
}

spa_pod *build_format(spa_pod_builder *b,
                      TextureFormat format,
                      const std::vector<uint64_t>& modifiers)
{
    spa_pod_frame format_frame{};

    /* Make an object of type SPA_TYPE_OBJECT_Format and id SPA_PARAM_EnumFormat.
	 * The object type is important because it defines the properties that are
	 * acceptable. The id gives more context about what the object is meant to
	 * contain. In this case we enumerate supported formats. */
    spa_pod_builder_push_object(b, &format_frame, SPA_TYPE_OBJECT_Format,
                                SPA_PARAM_EnumFormat);
    /* add media type and media subtype properties */
    spa_pod_builder_add(b, SPA_FORMAT_mediaType,
                        SPA_POD_Id(SPA_MEDIA_TYPE_video), 0);
    spa_pod_builder_add(b, SPA_FORMAT_mediaSubtype,
                        SPA_POD_Id(SPA_MEDIA_SUBTYPE_raw), 0);

    /* formats */
    spa_video_format spa_format = texture_format_to_spa(format);
    spa_pod_builder_add(b, SPA_FORMAT_VIDEO_format, SPA_POD_Id(spa_format), 0);

    /* modifier */
    if (!modifiers.empty())
    {
        spa_pod_frame modifier_frame{};

        /* build an enumeration of modifiers */
        spa_pod_builder_prop(b, SPA_FORMAT_VIDEO_modifier,
                             SPA_POD_PROP_FLAG_MANDATORY |
                             SPA_POD_PROP_FLAG_DONT_FIXATE);

        spa_pod_builder_push_choice(b, &modifier_frame, SPA_CHOICE_Enum, 0);

        /* The first element of choice pods is the preferred value. Here
         * we arbitrarily pick the first modifier as the preferred one.
         */
        spa_pod_builder_long(b, static_cast<int64_t>(modifiers[0]));

        /* modifiers from  an array */
        for (unsigned long modifier : modifiers)
            spa_pod_builder_long(b, static_cast<int64_t>(modifier));

        spa_pod_builder_pop(b, &modifier_frame);
    }

    /* add size and framerate ranges */
    auto range_rect_def = SPA_RECTANGLE(320, 240); // Arbitrary
    auto range_rect_min = SPA_RECTANGLE(1, 1);
    auto range_rect_max = SPA_RECTANGLE(8192, 4320);
    auto range_frac_def = SPA_FRACTION(g_host_params->fps_num, g_host_params->fps_den);
    auto range_frac_min = SPA_FRACTION(0, 1);
    auto range_frac_max = SPA_FRACTION(360, 1);
    spa_pod_builder_add(b, SPA_FORMAT_VIDEO_size,
                        SPA_POD_CHOICE_RANGE_Rectangle(
                                &range_rect_def,
                                &range_rect_min,
                                &range_rect_max),
                        SPA_FORMAT_VIDEO_framerate,
                        SPA_POD_CHOICE_RANGE_Fraction(
                                &range_frac_def,
                                &range_frac_min,
                                &range_frac_max),
                        0);

    return reinterpret_cast<spa_pod*>(spa_pod_builder_pop(b, &format_frame));
}

std::vector<spa_pod*> build_format_params(Pipewire *pw, spa_pod_builder *pod_builder)
{
    std::vector<spa_pod*> params;

    if (pw->GetServerVersion().Check(0, 3, 33))
    {
        for (const auto& pair : g_host_params->drm_formats)
        {
            if (pair.second.empty())
                continue;
            params.push_back(build_format(pod_builder, pair.first, pair.second));
        }
    }

    for (const auto& pair : g_host_params->drm_formats)
    {
        params.push_back(build_format(pod_builder, pair.first, {}));
    }

    return params;
}

void on_core_info(void *user_data, const pw_core_info *info)
{
    auto *pw = reinterpret_cast<Pipewire*>(user_data);

    fmt::print("[pipewire] Pipewire server version: {}\n", info->version);

    auto& triple = pw->GetServerVersion();
    // NOLINTNEXTLINE
    int n_matches = sscanf(info->version, "%d.%d.%d", &triple.major, &triple.minor, &triple.micro);

    if (n_matches != 3)
        fmt::print("[pipewire] Failed to parse server version\n");
}

void on_core_error(void *user_data, uint32_t id, int seq, int res, const char *message)
{
    auto *pw = reinterpret_cast<Pipewire*>(user_data);

    fmt::print("[pipewire] Error id:{} seq:{} res:{} ({}): {}\n",
               id, seq, res, g_strerror(res), message);

    pw_thread_loop_signal(pw->GetThreadLoop(), false);
}

void on_core_done(void *user_data, uint32_t id, int seq)
{
    auto *pw = reinterpret_cast<Pipewire*>(user_data);

    if (id == PW_ID_CORE && pw->GetServerVersionSync() == seq)
        pw_thread_loop_signal(pw->GetThreadLoop(), false);
}

const pw_core_events g_core_listener = {
    .version = PW_VERSION_CORE_EVENTS,
    .info = on_core_info,
    .done = on_core_done,
    .error = on_core_error
};

std::shared_ptr<MemoryTexture> update_memory_texture_from_bitmap(spa_meta_bitmap *bitmap,
                                                                 const std::shared_ptr<MemoryTexture>& old)
{
    auto format = spa_format_to_texture_format((spa_video_format) bitmap->format);
    CHECK(format != TextureFormat::kUnknown);

    TextureInfo current_info(format,
                             static_cast<int32_t>(bitmap->size.width),
                             static_cast<int32_t>(bitmap->size.height));

    std::shared_ptr<MemoryTexture> result = old;

    if (!old || old->GetInfo() != current_info)
    {
        // Recreate a memory texture with new dimensions and format
        result = MemoryTexture::Allocate(current_info);
        CHECK(result);
    }

    // Copy pixels into the memory texture
    const uint8_t *pixels = SPA_PTROFF(bitmap, bitmap->offset, uint8_t);
    std::memcpy(result->GetStorage(), pixels, current_info.ComputeMinByteSize());

    return result;
}

std::shared_ptr<MemoryTexture> update_memory_texture_from_video_info(const spa_video_info& info,
                                                                     const uint8_t *pixels,
                                                                     const std::shared_ptr<MemoryTexture>& old)
{
    CHECK(pixels);

    auto format = spa_format_to_texture_format(info.info.raw.format);
    CHECK(format != TextureFormat::kUnknown);

    TextureInfo current_info(format,
                             static_cast<int32_t>(info.info.raw.size.width),
                             static_cast<int32_t>(info.info.raw.size.height));

    std::shared_ptr<MemoryTexture> result = old;

    if (!old || old->GetInfo() != current_info)
    {
        // Recreate a memory texture with new dimensions and format
        result = MemoryTexture::Allocate(current_info);
        CHECK(result);
    }

    // Copy pixels into the memory texture
    std::memcpy(result->GetStorage(), pixels, current_info.ComputeMinByteSize());

    return result;
}

void on_stream_process(void *user_data)
{
    auto *pw = reinterpret_cast<Pipewire*>(user_data);

    // Find the most recent buffer
    pw_buffer *b = nullptr;
    while (true)
    {
        pw_buffer *aux = pw_stream_dequeue_buffer(pw->GetVideoStream());
        if (!aux)
            break;
        if (b)
            pw_stream_queue_buffer(pw->GetVideoStream(), b);
        b = aux;
    }

    if (!b)
    {
        fmt::print("[pipewire] Out of buffers\n");
        return;
    }
    spa_buffer *buffer = b->buffer;

    // Receive and extract the cursor texture from the optional metadata
    auto *cursor = reinterpret_cast<spa_meta_cursor*>(
            spa_buffer_find_meta_data(buffer, SPA_META_Cursor, sizeof(spa_meta_cursor)));
    pw->GetCursor().valid = cursor && spa_meta_cursor_is_valid(cursor);

    if (pw->GetCursor().visible && pw->GetCursor().valid)
    {
        spa_meta_bitmap *bitmap = nullptr;
        if (cursor->bitmap_offset)
        {
            // `SPA_MEMBER` has been deprecated
            bitmap = SPA_PTROFF(cursor, cursor->bitmap_offset, spa_meta_bitmap);
        }

        // Make sure it is a supported bitmap
        if (bitmap && bitmap->size.width > 0 && bitmap->size.height > 0 &&
            spa_format_to_texture_format((spa_video_format)bitmap->format) != TextureFormat::kUnknown)
        {
            pw->GetCursor().hotspot_x = cursor->hotspot.x;
            pw->GetCursor().hotspot_y = cursor->hotspot.y;
            pw->GetCursor().texture = update_memory_texture_from_bitmap(bitmap, pw->GetCursor().texture);
        }

        pw->GetCursor().x = cursor->position.x;
        pw->GetCursor().y = cursor->position.y;
    }

    // Prepare to receive and extract video buffers (dmabuf or memory-based buffers)
    if (!buffer->datas[0].chunk->size)
    {
        // No video buffers are provided
        pw_stream_queue_buffer(pw->GetVideoStream(), b);
        return;
    }

    // Receive and extract video crop information
    auto *region = (spa_meta_region *) spa_buffer_find_meta_data(buffer, SPA_META_VideoCrop,
                                                                 sizeof(spa_meta_region));
    if (region && spa_meta_region_is_valid(region))
    {
        VideoCrop& crop = pw->GetVideoCrop();
        crop.x = region->region.position.x;
        crop.y = region->region.position.y;
        crop.width = static_cast<int32_t>(region->region.size.width);
        crop.height = static_cast<int32_t>(region->region.size.height);
        crop.valid = true;
    }
    else
    {
        pw->GetVideoCrop().valid = false;
    }

    // Process video buffers now
    VideoTexture& video_texture = pw->GetVideoTexture();
    if (buffer->datas[0].type == SPA_DATA_DmaBuf)
    {
        spa_video_info& video_info = pw->GetVideoInfo();
        auto format = spa_format_to_texture_format(video_info.info.raw.format);
        if (format == TextureFormat::kUnknown)
        {
            // Unsupported pixel format in memory
            pw_stream_queue_buffer(pw->GetVideoStream(), b);
            return;
        }

        video_texture.dma_width = video_info.info.raw.size.width;
        video_texture.dma_height = video_info.info.raw.size.height;
        video_texture.dma_n_planes = buffer->n_datas;
        video_texture.dma_drm_format = texture_format_to_drm_format(format);

        for (uint32_t i = 0; i < video_texture.dma_n_planes; i++)
        {
            video_texture.dma_fds[i] = static_cast<int32_t>(buffer->datas[i].fd);
            video_texture.dma_offsets[i] = buffer->datas[i].chunk->offset;
            video_texture.dma_strides[i] = buffer->datas[i].chunk->stride;
            video_texture.dma_modifiers[i] = video_info.info.raw.modifier;
        }

        if (!g_host_params->host_accept_dmabuf)
        {
            // Host process did not accept DMA buffer with file descriptors,
            // and we should copy pixels from DMA buffers into the CPU memory
            // so that the pixels can be provided as a memory texture.
            fmt::print("[pipewire] warning: pipewire provided us with DMA buffers "
                       ", but the host process did NOT accept DMA buffers.\n");
            fmt::print("[pipewire] warning: screencast failing back to the shared memory mechanism"
                       ", which is very slow\n");

            // TODO: Map buffers and copy data
        }
    }
    else
    {
        // Memory-based texture
        spa_video_info& video_info = pw->GetVideoInfo();
        auto format = spa_format_to_texture_format(video_info.info.raw.format);
        if (format == TextureFormat::kUnknown)
        {
            // Unsupported pixel format in memory
            pw_stream_queue_buffer(pw->GetVideoStream(), b);
            return;
        }

        auto *pixels = reinterpret_cast<uint8_t*>(buffer->datas[0].data);
        video_texture.memory_texture = update_memory_texture_from_video_info(video_info,
                                                                             pixels,
                                                                             video_texture.memory_texture);
    }

    // Finally, upload the video texture and metadata to the host process.
    pw->UploadVideoTextureWithMetadata();
    pw_stream_queue_buffer(pw->GetVideoStream(), b);
}

void on_stream_param_changed(void *user_data, uint32_t id, const spa_pod *param)
{
    auto *pw = reinterpret_cast<Pipewire*>(user_data);

    int result = spa_format_parse(param,
                                  &pw->GetVideoInfo().media_type,
                                  &pw->GetVideoInfo().media_subtype);
    if (result < 0)
        return;

    if (pw->GetVideoInfo().media_type != SPA_MEDIA_TYPE_video ||
        pw->GetVideoInfo().media_subtype != SPA_MEDIA_SUBTYPE_raw)
        return;

    spa_format_video_raw_parse(param, &pw->GetVideoInfo().info.raw);

    uint32_t buffer_types = 1 << SPA_DATA_MemPtr;
    bool has_modifier = spa_pod_find_prop(param, nullptr, SPA_FORMAT_VIDEO_modifier);
    if (has_modifier || pw->GetServerVersion().Check(0, 3, 24))
        buffer_types |= 1 << SPA_DATA_DmaBuf;

    fmt::print("[pipewire] Negotiated format:\n");
    fmt::print("[pipewire]   format: {} ({})\n",
               static_cast<int>(pw->GetVideoInfo().info.raw.format),
               spa_debug_type_find_name(spa_type_video_format,
                                        pw->GetVideoInfo().info.raw.format));

    if (has_modifier)
        fmt::print("[pipewire]   modifier: 0x{:x}\n", pw->GetVideoInfo().info.raw.modifier);

    fmt::print("[pipewire]   size: {}x{}\n",
               pw->GetVideoInfo().info.raw.size.width,
               pw->GetVideoInfo().info.raw.size.height);

    fmt::print("[pipewire]   framerate: {}/{}\n",
               pw->GetVideoInfo().info.raw.framerate.num,
               pw->GetVideoInfo().info.raw.framerate.denom);

    spa_pod_builder pod_builder{};
    const struct spa_pod *params[3];
    uint8_t params_buffer[1024];

    /* Video crop */
    pod_builder = SPA_POD_BUILDER_INIT(params_buffer, sizeof(params_buffer));

    // NOLINTNEXTLINE
    params[0] = (spa_pod*) spa_pod_builder_add_object(&pod_builder,
                                                      SPA_TYPE_OBJECT_ParamMeta,
                                                      SPA_PARAM_Meta,
                                                      SPA_PARAM_META_type,
                                                      SPA_POD_Id(SPA_META_VideoCrop),
                                                      SPA_PARAM_META_size,
                                                      SPA_POD_Int(sizeof(spa_meta_region)));

    /* Cursor */

    // NOLINTNEXTLINE
    params[1] = (spa_pod*) spa_pod_builder_add_object(&pod_builder,
                                                      SPA_TYPE_OBJECT_ParamMeta,
                                                      SPA_PARAM_Meta,
                                                      SPA_PARAM_META_type,
                                                      SPA_POD_Id(SPA_META_Cursor),
                                                      SPA_PARAM_META_size,
                                                      SPA_POD_CHOICE_RANGE_Int(CURSOR_META_SIZE(64, 64),
                                                                               CURSOR_META_SIZE(1, 1),
                                                                               CURSOR_META_SIZE(1024, 1024)));

    /* Buffer options */

    // NOLINTNEXTLINE
    params[2] = (spa_pod*) spa_pod_builder_add_object(&pod_builder,
                                                      SPA_TYPE_OBJECT_ParamBuffers,
                                                      SPA_PARAM_Buffers,
                                                      SPA_PARAM_BUFFERS_dataType,
                                                      SPA_POD_Int(buffer_types));

    pw_stream_update_params(pw->GetVideoStream(), params, 3);
}

void on_stream_state_changed(void *user_data, g_maybe_unused pw_stream_state old,
                             pw_stream_state state, const char *error)
{
    auto *pw = reinterpret_cast<Pipewire*>(user_data);

    fmt::print("[pipewire] Stream {} state: \"{}\" (error: {})\n",
               fmt::ptr(pw->GetVideoStream()), pw_stream_state_as_string(state),
               error ? error : "none");
}

const pw_stream_events g_stream_listener = {
    .version = PW_VERSION_STREAM_EVENTS,
    .state_changed = on_stream_state_changed,
    .param_changed = on_stream_param_changed,
    .process = on_stream_process
};

void renegotiate_format(void *data, g_maybe_unused uint64_t expirations)
{
    auto *pw = reinterpret_cast<Pipewire*>(data);

    fmt::print("[pipewire] Renegotiating stream\n");

    pw_thread_loop_lock(pw->GetThreadLoop());

    uint8_t params_buffer[2048];
    spa_pod_builder pod_builder = SPA_POD_BUILDER_INIT(params_buffer, sizeof(params_buffer));

    std::vector<spa_pod*> params = build_format_params(pw, &pod_builder);
    if (params.empty())
    {
        // FIXME(sora): error handling
        pw_thread_loop_unlock(pw->GetThreadLoop());
        fmt::print("[pipewire] Failed to renegotiate stream: failed to build format params\n");
    }

    pw_stream_update_params(pw->GetVideoStream(),
                            const_cast<const spa_pod**>(params.data()),
                            params.size());
    pw_thread_loop_unlock(pw->GetThreadLoop());
}


} // namespace anonymous

std::shared_ptr<Pipewire> Pipewire::Make(int pipewire_fd, uint32_t pipewire_node)
{
    auto pw = std::make_shared<Pipewire>();
    pw->pipewire_fd_ = pipewire_fd;

    pw_init(nullptr, nullptr);

    pw->thread_loop_ = pw_thread_loop_new("PipeWire", nullptr);
    pw->context_ = pw_context_new(pw_thread_loop_get_loop(pw->thread_loop_),
                                  nullptr, 0);

    if (pw_thread_loop_start(pw->thread_loop_) < 0)
    {
        fmt::print("[pipewire] Error starting threaded mainloop\n");
        return nullptr;
    }

    pw_thread_loop_lock(pw->thread_loop_);

    // Core
    int dump_fd = fcntl(static_cast<int>(pipewire_fd), F_DUPFD_CLOEXEC, 5);
    pw->core_ = pw_context_connect_fd(pw->context_, dump_fd, nullptr, 0);

    if (!pw->core_)
    {
        fmt::print("[pipewire] Error creating PipeWire core: {}\n", strerror(errno));
        pw_thread_loop_unlock(pw->thread_loop_);
        return nullptr;
    }

    pw_core_add_listener(pw->core_, &pw->core_listener_, &g_core_listener, pw.get());

    // Signal to renegotiate
    pw->renegotiate_ = pw_loop_add_event(pw_thread_loop_get_loop(pw->thread_loop_),
                                         renegotiate_format, pw.get());
    fmt::print("[pipewire] Registered renegotiation event {}\n", fmt::ptr(pw->renegotiate_));

    // Dispatch to receive the info core event
    pw->server_version_sync_ = pw_core_sync(pw->core_, PW_ID_CORE, pw->server_version_sync_);
    pw_thread_loop_wait(pw->thread_loop_);

    // Stream
    pw->video_stream_ = pw_stream_new(pw->core_,
                                      "Cocoa Screencast (PipeWire)",
                                      pw_properties_new(PW_KEY_MEDIA_TYPE, "Video",
                                                        PW_KEY_MEDIA_CATEGORY, "Capture",
                                                        PW_KEY_MEDIA_ROLE, "Screen",
                                                        nullptr));
    pw_stream_add_listener(pw->video_stream_, &pw->stream_listener_, &g_stream_listener, pw.get());
    fmt::print("[pipewire] Created stream {}\n", fmt::ptr(pw->video_stream_));

    // Stream parameters
    uint8_t params_buffer[2048];
    spa_pod_builder pod_builder = SPA_POD_BUILDER_INIT(params_buffer, sizeof(params_buffer));

    std::vector<spa_pod*> params = build_format_params(pw.get(), &pod_builder);
    if (params.empty())
    {
        pw_thread_loop_unlock(pw->thread_loop_);
        return nullptr;
    }

    auto flags = static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS);
    int result = pw_stream_connect(pw->video_stream_,
                                   PW_DIRECTION_INPUT,
                                   pipewire_node,
                                   flags,
                                   const_cast<const spa_pod **>(params.data()),
                                   params.size());
    if (result < 0)
    {
        fmt::print("[pipewire] Failed to connect stream\n");
        pw_thread_loop_unlock(pw->thread_loop_);
        return nullptr;
    }

    fmt::print("[pipewire] Playing stream {}\n", fmt::ptr(pw->video_stream_));

    pw_thread_loop_unlock(pw->thread_loop_);

    return pw;
}

Pipewire::Pipewire()
    : pipewire_fd_(-1)
    , thread_loop_(nullptr)
    , context_(nullptr)
    , core_(nullptr)
    , renegotiate_(nullptr)
    , server_version_{}
    , server_version_sync_(0)
    , video_stream_(nullptr)
    , core_listener_{}
    , stream_listener_{}
    , video_info_{}
    , cursor_{}
    , video_crop_()
    , video_texture_{}
{
    // Cursor is visible by default.
    cursor_.valid = false;
    cursor_.visible = true;

    video_crop_.valid = false;
}

Pipewire::~Pipewire()
{
    if (thread_loop_)
    {
        pw_thread_loop_wait(thread_loop_);
        pw_thread_loop_stop(thread_loop_);
    }

    if (video_stream_)
    {
        pw_stream_disconnect(video_stream_);
        pw_stream_destroy(video_stream_);
    }

    if (context_)
        pw_context_destroy(context_);

    if (thread_loop_)
        pw_thread_loop_destroy(thread_loop_);

    if (pipewire_fd_ > 0)
        close(pipewire_fd_);
}

void Pipewire::Activate()
{
    pw_stream_set_active(video_stream_, true);
}

void Pipewire::Deactivate()
{
    pw_stream_set_active(video_stream_, false);
}

bool Pipewire::UploadVideoTextureWithMetadata()
{
}

SCREENCAST_NAMESPACE_END
