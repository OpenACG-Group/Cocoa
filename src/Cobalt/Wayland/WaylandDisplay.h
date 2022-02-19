#ifndef COCOA_COBALT_WAYLANDDISPLAY_H
#define COCOA_COBALT_WAYLANDDISPLAY_H

#include <map>

#include <wayland-client-protocol.h>

#include "Cobalt/Display.h"
#include "Cobalt/Wayland/protos/xdg-shell-client-protocol.h"
COBALT_NAMESPACE_BEGIN

class WaylandDisplay : public Display
{
public:
    static g_inline WaylandDisplay *BareCast(void *data) {
        return reinterpret_cast<WaylandDisplay*>(data);
    }

    static co_sp<WaylandDisplay> Connect(uv_loop_t *loop, const std::string& name);

    WaylandDisplay(uv_loop_t *loop, int fd);
    ~WaylandDisplay() override;

    g_nodiscard g_inline auto& GetGlobalsIdMap() {
        return globals_id_map_;
    }


    static void RegistryHandleGlobal(void *data, wl_registry *registry, uint32_t id,
                                     const char *interface, uint32_t version);
    static void RegistryHandleGlobalRemove(void *data, wl_registry *registry, uint32_t name);

private:
    void OnDispose() override;

    wl_display                     *wl_display_;
    wl_registry                    *wl_registry_;
    std::map<uint32_t, std::string> globals_id_map_;
    wl_compositor                  *wl_compositor_;
    xdg_wm_base                    *xdg_wm_base_;
};

COBALT_NAMESPACE_END
#endif //COCOA_COBALT_WAYLANDDISPLAY_H
