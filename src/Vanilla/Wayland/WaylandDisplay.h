#ifndef COCOA_WAYLANDDISPLAY_H
#define COCOA_WAYLANDDISPLAY_H

#include <wayland-client.h>
#include <ostream>
#include "Vanilla/Wayland/xdg-shell-client-protocol.h"
#include "Vanilla/Wayland/xdg-shell-unstable-v6-protocol.h"
#include "Vanilla/Wayland/primary-selection-unstable-v1-client-protocol.h"
#include "Vanilla/Wayland/pointer-gestures-unstable-v1-client-protocol.h"

#include "Core/EventSource.h"

#include "Vanilla/Display.h"
VANILLA_NS_BEGIN

class Context;
class KeyboardProxy;
class WaylandSeat;

class WaylandDisplay : public Display,
                       public PollSource,
                       public std::enable_shared_from_this<WaylandDisplay>
{
public:
    static inline WaylandDisplay *GetBareFromUserData(void *data) {
        return reinterpret_cast<WaylandDisplay*>(data);
    }

    struct WlInterfacesBatch
    {
        /* IFacePair: (pointer, version) */
        template<typename IFaceT>
        using IFacePair = std::pair<IFaceT*, uint32_t>;

        /* IFaceTriple: (pointer, id, version) */
        template<typename IFaceT>
        using IFaceTriple = std::tuple<IFaceT*, uint32_t, uint32_t>;

        std::vector<std::string> known_ifaces;
        IFacePair<::wl_compositor> compositor;
        IFacePair<::wl_shm> shm;
        IFacePair<::wl_data_device_manager> data_device_manager;
        IFaceTriple<::wl_seat> seat;
        IFacePair<::wl_subcompositor> subcompositor;
        IFaceTriple<::xdg_wm_base> xdg_wm_base{nullptr, 0, 0};
        IFaceTriple<::zxdg_shell_v6> zxdg_shell_v6{nullptr, 0, 0};
        IFacePair<::zwp_primary_selection_device_manager_v1> primary_selection_manager{nullptr, 0};
        IFacePair<::zwp_pointer_gestures_v1> pointer_gestures{nullptr, 0};
    };

    enum class WaylandShellVariant
    {
        kXDGShell,
        kZXDGShellV6,
        kPending
    };

    WaylandDisplay(const Handle<Context>& ctx,
                   ::wl_display *display,
                   int32_t poll_fd);
    ~WaylandDisplay() override;

    va_nodiscard inline std::unique_ptr<WlInterfacesBatch>& getInterfacesBatch() {
        return fIFacesBatch;
    }

    va_nodiscard inline WaylandShellVariant getShellVariant() const {
        return fShellVariant;
    }

    va_nodiscard Handle<WaylandSeat> getSeat() const {
        return fSeat;
    }

    int32_t height() override {}
    int32_t width() override {}

    void dispose() override;
    void flush() override;

    KeyboardProxy *keyboardProxy() override {}

    Handle<Window> onCreateWindow(vec::float2 size, vec::float2 pos, Handle<Window> parent) override {}

    ::wl_callback *sync();
    void asyncRoundtrip();
    bool waitForAsynchronousRoundtrips();

    inline void setShellVariant(WaylandShellVariant v) {
        fShellVariant = v;
    }

    void createSeatFromIFacesBatch();

    static void AsyncRoundtripCallback(void *data, ::wl_callback *cb, uint32_t time);

private:
    void disposeNonVirtualOverride();
    KeepInLoop pollDispatch(int status, int events) override;

    bool                                fDisposed;
    ::wl_display                       *fWlDisplay;
    std::unique_ptr<WlInterfacesBatch>  fIFacesBatch;
    std::list<::wl_callback*>           fAsyncRoundtripList;
    WaylandShellVariant                 fShellVariant;
    Handle<WaylandSeat>                 fSeat;
};

VANILLA_NS_END
#endif //COCOA_WAYLANDDISPLAY_H
