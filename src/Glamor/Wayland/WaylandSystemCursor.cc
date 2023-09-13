#include <cstdlib>

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Glamor/PresentThread.h"
#include "Glamor/Wayland/WaylandSystemCursor.h"
#include "Glamor/Wayland/WaylandDisplay.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Wayland.Cursor)

WaylandSystemCursor::WaylandSystemCursor(const Shared<CursorTheme>& theme,
                                         wl_cursor *cursor,
                                         wl_surface *cursor_surface)
    : WaylandCursor(theme, cursor_surface)
    , cursor_surface_(cursor_surface)
    , current_cursor_(cursor)
    , current_cursor_image_idx_(0)
    , animation_timer_{}
{
    auto *local_context = PresentThread::LocalContext::GetCurrent();
    uv_timer_init(local_context->GetEventLoop(), &animation_timer_);
    uv_handle_set_data((uv_handle_t*) &animation_timer_, this);
}

WaylandSystemCursor::~WaylandSystemCursor() = default;

void WaylandSystemCursor::PrepareCursorSurfaceAndAnimation()
{
    wl_cursor_image *image = current_cursor_->images[0];
    wl_buffer *buffer = wl_cursor_image_get_buffer(image);
    CHECK(buffer && "Invalid cursor image buffer");

    wl_surface_attach(cursor_surface_, buffer, 0, 0);
    wl_surface_damage(cursor_surface_, 0, 0,
                      static_cast<int32_t>(image->width),
                      static_cast<int32_t>(image->height));
    wl_surface_commit(cursor_surface_);
}

bool WaylandSystemCursor::OnHasAnimation()
{
    return current_cursor_->image_count > 1;
}

void WaylandSystemCursor::OnTryStartAnimation()
{
    if (current_cursor_->image_count == 1)
    {
        // There are no animations should be performed for on the cursor as
        // it only contains a single image.
        // In this situation, we attach the image buffer to the cursor surface
        // directly and return.
        return;
    }

    // Perform animations on the cursor
    wl_cursor_image *image = current_cursor_->images[0];
    uv_timer_start(&animation_timer_, on_animation_timer, image->delay, 0);
}

void WaylandSystemCursor::OnTryAbortAnimation()
{
    uv_timer_stop(&animation_timer_);
}

void WaylandSystemCursor::OnDispose()
{
    uv_timer_stop(&animation_timer_);
    if (cursor_surface_)
        wl_surface_destroy(cursor_surface_);

    cursor_surface_ = nullptr;
    current_cursor_ = nullptr;
    current_cursor_image_idx_ = 0;
}

SkIVector WaylandSystemCursor::OnGetHotspotVector()
{
    wl_cursor_image *image = current_cursor_->images[current_cursor_image_idx_];
    return SkIVector::Make(static_cast<int32_t>(image->hotspot_x),
                           static_cast<int32_t>(image->hotspot_y));
}

void WaylandSystemCursor::on_animation_timer(uv_timer_t *handle)
{
    CHECK(handle);
    auto *w = reinterpret_cast<WaylandSystemCursor*>(uv_handle_get_data((uv_handle_t*)handle));

    w->current_cursor_image_idx_++;
    if (w->current_cursor_image_idx_ == w->current_cursor_->image_count)
        w->current_cursor_image_idx_ = 0;

    wl_cursor_image *image = w->current_cursor_->images[w->current_cursor_image_idx_];
    wl_buffer *buffer = wl_cursor_image_get_buffer(image);
    CHECK(buffer && "Invalid cursor image buffer");

    wl_surface_attach(w->cursor_surface_, buffer, 0, 0);
    wl_surface_damage(w->cursor_surface_, 0, 0,
                      static_cast<int32_t>(image->width),
                      static_cast<int32_t>(image->height));
    wl_surface_commit(w->cursor_surface_);

    uv_timer_start(&w->animation_timer_, on_animation_timer, image->delay, 0);
}

GLAMOR_NAMESPACE_END
