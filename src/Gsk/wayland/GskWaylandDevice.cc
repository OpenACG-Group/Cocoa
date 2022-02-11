#include "Gsk/wayland/GskWaylandDevice.h"

#include <utility>
#include "Gsk/wayland/GskWaylandSeat.h"
#include "Gsk/wayland/GskWaylandDisplay.h"
#include "Core/EventSource.h"
#include "Core/EventLoop.h"
GSK_NAMESPACE_BEGIN

class CursorUpdateTimer;

uint32_t update_timer_id_cnt_ = 0;
std::map<uint32_t, CursorUpdateTimer*> update_timers_;

class CursorUpdateTimer : public TimerSource
{
public:
    using Callback = std::function<KeepInLoop()>;

    static uint32_t Add(int64_t interval, const Callback& cb) {
        uint32_t id = ++update_timer_id_cnt_;
        update_timers_[id] = new CursorUpdateTimer(interval, cb);
        return id;
    }

    static void Remove(uint32_t id) {
        if (update_timers_.count(id) > 0)
        {
            delete update_timers_[id];
            update_timers_.erase(id);
        }
    }

    CursorUpdateTimer(int64_t interval, Callback cb)
    : TimerSource(EventLoop::Instance())
    , fCallback(std::move(cb))
    {
        startTimer(interval, interval);
    }

    KeepInLoop timerDispatch() override {
        return fCallback();
    }

private:
    Callback    fCallback;
};

namespace {

void pointer_stop_cursor_animation(GskWaylandPointerData *pointer)
{
    if (pointer->cursorTimeoutId > 0)
    {
        CursorUpdateTimer::Remove(pointer->cursorTimeoutId);
        pointer->cursorTimeoutId = 0;
        pointer->cursorImageDelay = 0;
    }
    pointer->cursorImageIndex = 0;
}

}

GskWaylandDevice::GskWaylandDevice(const Weak<GskDisplay>& display)
    : GskDevice(display)
{
}

GskWaylandDevice::~GskWaylandDevice()
{
}

KeepInLoop GskWaylandDevice::updateSurfaceCursor()
{
    auto seat = std::dynamic_pointer_cast<GskWaylandSeat>(GskDevice::getSeat());
    auto display = std::dynamic_pointer_cast<GskWaylandDisplay>(getDisplay());

    GskWaylandTabletData *tablet = seat->findTablet(shared_from_this());

    wl_buffer *buffer;
    Vec2i pos;
    SkISize size(SkISize::MakeEmpty());
    int scale;

    if (fPointer->cursor)
    {
        buffer = display->cursorGetBuffer(fPointer->cursor,
                                          fPointer->currentOutputScale,
                                          fPointer->cursorImageIndex,
                                          pos,
                                          size,
                                          scale);
    }
    else
    {
        CursorUpdateTimer::Remove(fPointer->cursorTimeoutId);
        fPointer->cursorTimeoutId = 0;
        return KeepInLoop::kDeleted;
    }

    if (tablet)
    {
        if (!tablet->currentTool)
        {
            CursorUpdateTimer::Remove(fPointer->cursorTimeoutId);
            fPointer->cursorTimeoutId = 0;
            return KeepInLoop::kDeleted;
        }

        zwp_tablet_tool_v2_set_cursor(tablet->currentTool->wpTabletTool,
                                      fPointer->enterSerial,
                                      fPointer->pointerSurface,
                                      pos[0], pos[1]);
    }
    else if (seat->fWlPointer)
    {
        wl_pointer_set_cursor(seat->fWlPointer,
                              fPointer->enterSerial,
                              fPointer->pointerSurface,
                              pos[0], pos[1]);
    }
    else
    {
        CursorUpdateTimer::Remove(fPointer->cursorTimeoutId);
        fPointer->cursorTimeoutId = 0;
        return KeepInLoop::kDeleted;
    }

    if (buffer)
    {
        wl_surface_attach(fPointer->pointerSurface, buffer, 0, 0);
        wl_surface_set_buffer_scale(fPointer->pointerSurface, scale);
        wl_surface_damage(fPointer->pointerSurface, 0, 0, size.width(), size.height());
        wl_surface_commit(fPointer->pointerSurface);
    }
    else
    {
        wl_surface_attach(fPointer->pointerSurface, nullptr, 0, 0);
        wl_surface_commit(fPointer->pointerSurface);
    }


}

GSK_NAMESPACE_END
