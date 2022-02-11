#ifndef COCOA_GSKWAYLANDDISPLAY_H
#define COCOA_GSKWAYLANDDISPLAY_H

#include <wayland-client-protocol.h>
#include "Gsk/wayland/cursor/wayland-cursor.h"
#include <xkbcommon/xkbcommon.h>
#include "Gsk/wayland/protos/xdg-shell-client-protocol.h"
#include "Gsk/wayland/protos/xdg-shell-unstable-v6-protocol.h"
#include "Gsk/wayland/protos/primary-selection-unstable-v1-client-protocol.h"
#include "Gsk/wayland/protos/pointer-gestures-unstable-v1-client-protocol.h"
#include "Gsk/wayland/protos/tablet-unstable-v2-client-protocol.h"
#include "Gsk/wayland/protos/xdg-output-unstable-v1-protocol.h"

#include "include/core/SkSurface.h"

#include "Core/EventSource.h"

#include "Gsk/Gsk.h"
#include "Gsk/GskDisplay.h"
#include "Gsk/GskCursor.h"
#include "Gsk/wayland/GskSkSurfaceWrapper.h"
GSK_NAMESPACE_BEGIN

class GskWaylandKeymap;
class GskWaylandDisplay;

class GskWaylandDisplay : public GskDisplay,
                          public PollSource,
                          public PrepareSource
{
public:
    struct GlobalsSet
    {
        std::vector<std::string> known_globals;

        wl_compositor                   *compositor{nullptr};
        wl_shm                          *shm{nullptr};
        wl_data_device_manager          *dataDeviceManager{nullptr};
        wl_seat                         *seat{nullptr};
        wl_subcompositor                *subCompositor{nullptr};
        xdg_wm_base                     *wmBase{nullptr};
        zxdg_shell_v6                   *zxdgShell{nullptr};
        zxdg_output_manager_v1          *outputManager;
        zwp_primary_selection_device_manager_v1
                                        *primarySelectionManager{nullptr};
        zwp_pointer_gestures_v1         *pointerGestures{nullptr};
        zwp_tablet_manager_v2           *tabletManager{nullptr};

        uint32_t compositorVersion;
        uint32_t wmBaseId = 0;
        uint32_t wmBaseVersion;
        uint32_t zxdgShellId = 0;
        uint32_t dataDeviceManagerVersion;
        uint32_t outputManagerVersion;
    };

    struct NativePODs
    {
        wl_display *display{nullptr};
        wl_registry *registry{nullptr};
        std::list<wl_event_queue*> queues;
        wl_cursor_theme *cursorTheme;
    };

    enum class ShellVariant
    {
        kXDG,
        kZXDG,
        kPending
    };

    struct ServerBarrier
    {
        GskWaylandDisplay *display;
        std::function<void()> cb;
        wl_callback *wlc;
    };

    explicit GskWaylandDisplay(int32_t fd);
    ~GskWaylandDisplay() override;

    static Handle<GskWaylandDisplay> Make(const std::string& displayName);
    g_nodiscard g_inline static GskWaylandDisplay *CastBare(void *data) {
        return reinterpret_cast<GskWaylandDisplay*>(data);
    }

    g_nodiscard g_inline Unique<GlobalsSet>& getGlobalsSet() {
        return fGlobalsSet;
    }

    g_nodiscard g_inline ShellVariant getShellVariant() const {
        return fShellVariant;
    }

    /**
     * Checks whether the Wayland compositor prefers to draw the window
     * decorations or if it leaves decorations to the application.
     * @return true if the compositor prefers server-side decorations.
     */
    bool prefersSSD();

    /**
     * A server barrier is a special request that can make sure
     * that all the requests before it are processed by Wayland
     * compositor.
     * This function returns immediately and you can wait for
     * inserted barriers by `waitForAllServerBarriers`, or they
     * will be processed in event loop later.
     * When a barrier reaches, the `cb` callback function
     * associated with it will be invoked.
     */
    void insertServerBarrier(const std::function<void()>& cb);
    bool waitForAllServerBarriers();
    static void ServerBarrierCallback(void *data, wl_callback *cb, uint32_t time);

    g_nodiscard g_inline const std::list<Handle<GskSurface>>& getToplevelSurfaces() const {
        return fToplevels;
    }

    g_nodiscard g_inline wl_cursor_theme *getCursorTheme() {
        return fPods->cursorTheme;
    }

    g_nodiscard g_inline ulong getSerial() const {
        return fSerial;
    }

    g_inline void updateSerial(ulong serial) {
        fSerial = serial;
    }

    /* Returns a SkSurface associated with a data pointer. Never delete that pointer
     * because it will be deleted automatically when SkSurface is destroyed. */
    g_private_api GskSkSurfaceWrapper *createRasterShmSurface(const Vec2i& size, int scale);

    g_private_api void removeMaybeOutput(uint32_t id);
    g_private_api void updateScale();
    g_private_api wl_buffer *cursorGetBuffer(const Handle<GskCursor>& cursor,
                                             uint32_t desiredScale,
                                             uint32_t imageIndex,
                                             Vec2i& hotspot,
                                             SkISize& size,
                                             int& scale);
    g_private_api uint32_t cursorGetNextImageIndex(const Handle<GskCursor>& cursor,
                                                   uint32_t scale,
                                                   uint32_t currentIndex,
                                                   uint32_t& nextImageDelay);
    g_private_api void removeCursorSurfaceCache(GskSkSurfaceWrapper *wrapped);

private:
    void onFlush() override;
    void onSync() override;
    void onEnqueueEvents(Unique<GskEventQueue> &queue) override;
    void onDispose() override;
    void onMakeDefault() override;
    bool onHasPending() override;
    uint32_t onGetNextSerial() override;
    Handle<GskSurface> onCreateSurface(SurfaceType type,
                                       const Handle<GskSurface> &parent,
                                       Vec2i pos,
                                       Vec2i size) override;
    Handle<GskKeymap> onGetKeymap() override;
    Handle<GskMonitor> onGetMonitorAtSurface(const Handle<GskSurface> &surface) override;
    void onSetCursorTheme(const std::string &name, int size) override;

    void dispatchEventsFromQueue();
    KeepInLoop pollDispatch(int status, int events) override;
    KeepInLoop prepareDispatch() override;

    Unique<NativePODs>          fPods;
    Unique<GlobalsSet>          fGlobalsSet;
    ShellVariant                fShellVariant;
    std::list<ServerBarrier*>   fServerBarriers;

    std::list<Handle<GskSurface>>   fToplevels;
    /* Most recent serial */
    uint32_t                        fSerial;

    std::list<Handle<GskSurface>>   fCurrentPopups;
    std::list<Handle<GskSurface>>   fCurrentGrabbingPopups;

    wl_cursor_theme                *fCursorTheme;
    std::string                     fCursorThemeName;
    int                             fCursorThemeSize;
    std::unordered_map<Handle<GskCursor>, GskSkSurfaceWrapper*,
                       GskCursor::HashFunctor, GskCursor::EqualFunctor>
                                    fCursorSkSurfaceCache;

    xkb_context                    *fXKBContext;
    Handle<GskWaylandKeymap>        fTmpKeymap;
};

GSK_NAMESPACE_END
#endif //COCOA_GSKWAYLANDDISPLAY_H
