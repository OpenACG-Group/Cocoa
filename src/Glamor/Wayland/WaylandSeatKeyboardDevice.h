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

#ifndef COCOA_GLAMOR_WAYLAND_WAYLANDSEATKEYBOARDDEVICE_H
#define COCOA_GLAMOR_WAYLAND_WAYLANDSEATKEYBOARDDEVICE_H

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>

#include "uv.h"

#include "Core/EnumClassBitfield.h"
#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

class WaylandSeat;

class WaylandSeatKeyboardDevice
{
public:
    WaylandSeatKeyboardDevice(WaylandSeat *seat, wl_keyboard *keyboard);
    ~WaylandSeatKeyboardDevice();

    static std::unique_ptr<WaylandSeatKeyboardDevice>
    MakeFromKeyboardDevice(WaylandSeat *seat, wl_keyboard *keyboard);

    g_nodiscard g_inline WaylandSeat *GetSeat() const {
        return seat_;
    }

    void UpdateKeyRepeat(KeyboardKey key, bool pressed);
    void TryStopKeyRepeat();

    // Keyboard events
    static void on_keymap(void *data,
                          wl_keyboard *keyboard,
                          uint32_t format,
                          int32_t fd,
                          uint32_t size);
    
    static void on_enter(void *data,
                         wl_keyboard *keyboard,
                         uint32_t serial,
                         wl_surface *surface,
                         wl_array *keys);
    
    static void on_leave(void *data,
                         wl_keyboard *keyboard,
                         uint32_t serial,
                         wl_surface *surface);
    
    static void on_key(void *data,
                       wl_keyboard *keyboard,
                       uint32_t serial,
                       uint32_t time,
                       uint32_t key,
                       uint32_t state);
    
    static void on_modifiers(void *data,
                             wl_keyboard *keyboard,
                             uint32_t serial,
                             uint32_t mods_depressed,
                             uint32_t mods_latched,
                             uint32_t mods_locked,
                             uint32_t group);
    
    static void on_repeat_info(void *data,
                               wl_keyboard *keyboard,
                               int32_t rate,
                               int32_t delay);

private:
    static void repeat_timer_callback(uv_timer_t *timer);

    struct ModIndices
    {
        xkb_mod_index_t control = 0;
        xkb_mod_index_t alt = 0;
        xkb_mod_index_t shift = 0;
        xkb_mod_index_t super = 0;
        xkb_mod_index_t caps_lock = 0;
        xkb_mod_index_t num_lock = 0;
        xkb_mod_index_t meta = 0;
    };

    WaylandSeat         *seat_;
    wl_keyboard         *keyboard_device_;
    xkb_keymap          *keymap_;
    xkb_state           *state_;
    xkb_compose_state   *compose_state_;
    ModIndices          mod_indices;
    Bitfield<KeyboardModifiers> modifiers_;

    int32_t             key_repeat_delay_ms_;
    int32_t             key_repeat_rate_;
    bool                key_is_repeating_;
    uv_timer_t          key_repeat_timer_;
    KeyboardKey         repeating_key_;
    bool                key_repeat_first_fire_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_WAYLANDSEATKEYBOARDDEVICE_H
