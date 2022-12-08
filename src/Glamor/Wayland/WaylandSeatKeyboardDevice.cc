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

#include <sys/mman.h>
#include <linux/input-event-codes.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon-compose.h>

#include "fmt/format.h"

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Core/Exception.h"
#include "Glamor/Wayland/WaylandDisplay.h"
#include "Glamor/Wayland/WaylandSeat.h"
#include "Glamor/Wayland/WaylandSeatKeyboardDevice.h"
#include "Glamor/Wayland/WaylandSurface.h"
#include "Glamor/Wayland/WaylandInputContext.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Wayland.WaylandSeatKeyboardDevice)

#define LISTENER(ptr)   reinterpret_cast<WaylandSeatKeyboardDevice*>(ptr)

namespace {

struct KeycodeMap
{
    static constexpr size_t kMapSize = 256;
    static constexpr size_t kReverseMapSize =
            static_cast<uint16_t>(KeyboardKey::kLast) + 1;

    // scancode -----> keycode
    KeyboardKey keycodes[kMapSize];

    // keycode -----> scancode
    int16_t scancodes[kReverseMapSize];

    static std::unique_ptr<KeycodeMap> Create();
};

std::unique_ptr<KeycodeMap> KeycodeMap::Create()
{
    auto map = std::make_unique<KeycodeMap>();

    for (auto & keycode : map->keycodes)
    {
        keycode = KeyboardKey::kPlaceholder;
    }
    std::memset(map->scancodes, -1, sizeof(map->scancodes));
    
    using T = KeyboardKey;

    map->keycodes[KEY_GRAVE]      = T::kKey_GRAVE_ACCENT;
    map->keycodes[KEY_1]          = T::kKey_1;
    map->keycodes[KEY_2]          = T::kKey_2;
    map->keycodes[KEY_3]          = T::kKey_3;
    map->keycodes[KEY_4]          = T::kKey_4;
    map->keycodes[KEY_5]          = T::kKey_5;
    map->keycodes[KEY_6]          = T::kKey_6;
    map->keycodes[KEY_7]          = T::kKey_7;
    map->keycodes[KEY_8]          = T::kKey_8;
    map->keycodes[KEY_9]          = T::kKey_9;
    map->keycodes[KEY_0]          = T::kKey_0;
    map->keycodes[KEY_SPACE]      = T::kKey_SPACE;
    map->keycodes[KEY_MINUS]      = T::kKey_MINUS;
    map->keycodes[KEY_EQUAL]      = T::kKey_EQUAL;
    map->keycodes[KEY_Q]          = T::kKey_Q;
    map->keycodes[KEY_W]          = T::kKey_W;
    map->keycodes[KEY_E]          = T::kKey_E;
    map->keycodes[KEY_R]          = T::kKey_R;
    map->keycodes[KEY_T]          = T::kKey_T;
    map->keycodes[KEY_Y]          = T::kKey_Y;
    map->keycodes[KEY_U]          = T::kKey_U;
    map->keycodes[KEY_I]          = T::kKey_I;
    map->keycodes[KEY_O]          = T::kKey_O;
    map->keycodes[KEY_P]          = T::kKey_P;
    map->keycodes[KEY_LEFTBRACE]  = T::kKey_LEFT_BRACKET;
    map->keycodes[KEY_RIGHTBRACE] = T::kKey_RIGHT_BRACKET;
    map->keycodes[KEY_A]          = T::kKey_A;
    map->keycodes[KEY_S]          = T::kKey_S;
    map->keycodes[KEY_D]          = T::kKey_D;
    map->keycodes[KEY_F]          = T::kKey_F;
    map->keycodes[KEY_G]          = T::kKey_G;
    map->keycodes[KEY_H]          = T::kKey_H;
    map->keycodes[KEY_J]          = T::kKey_J;
    map->keycodes[KEY_K]          = T::kKey_K;
    map->keycodes[KEY_L]          = T::kKey_L;
    map->keycodes[KEY_SEMICOLON]  = T::kKey_SEMICOLON;
    map->keycodes[KEY_APOSTROPHE] = T::kKey_APOSTROPHE;
    map->keycodes[KEY_Z]          = T::kKey_Z;
    map->keycodes[KEY_X]          = T::kKey_X;
    map->keycodes[KEY_C]          = T::kKey_C;
    map->keycodes[KEY_V]          = T::kKey_V;
    map->keycodes[KEY_B]          = T::kKey_B;
    map->keycodes[KEY_N]          = T::kKey_N;
    map->keycodes[KEY_M]          = T::kKey_M;
    map->keycodes[KEY_COMMA]      = T::kKey_COMMA;
    map->keycodes[KEY_DOT]        = T::kKey_PERIOD;
    map->keycodes[KEY_SLASH]      = T::kKey_SLASH;
    map->keycodes[KEY_BACKSLASH]  = T::kKey_BACKSLASH;
    map->keycodes[KEY_ESC]        = T::kKey_ESCAPE;
    map->keycodes[KEY_TAB]        = T::kKey_TAB;
    map->keycodes[KEY_LEFTSHIFT]  = T::kKey_LEFT_SHIFT;
    map->keycodes[KEY_RIGHTSHIFT] = T::kKey_RIGHT_SHIFT;
    map->keycodes[KEY_LEFTCTRL]   = T::kKey_LEFT_CONTROL;
    map->keycodes[KEY_RIGHTCTRL]  = T::kKey_RIGHT_CONTROL;
    map->keycodes[KEY_LEFTALT]    = T::kKey_LEFT_ALT;
    map->keycodes[KEY_RIGHTALT]   = T::kKey_RIGHT_ALT;
    map->keycodes[KEY_LEFTMETA]   = T::kKey_LEFT_SUPER;
    map->keycodes[KEY_RIGHTMETA]  = T::kKey_RIGHT_SUPER;
    map->keycodes[KEY_COMPOSE]    = T::kKey_MENU;
    map->keycodes[KEY_NUMLOCK]    = T::kKey_NUM_LOCK;
    map->keycodes[KEY_CAPSLOCK]   = T::kKey_CAPS_LOCK;
    map->keycodes[KEY_PRINT]      = T::kKey_PRINT_SCREEN;
    map->keycodes[KEY_SCROLLLOCK] = T::kKey_SCROLL_LOCK;
    map->keycodes[KEY_PAUSE]      = T::kKey_PAUSE;
    map->keycodes[KEY_DELETE]     = T::kKey_DELETE;
    map->keycodes[KEY_BACKSPACE]  = T::kKey_BACKSPACE;
    map->keycodes[KEY_ENTER]      = T::kKey_ENTER;
    map->keycodes[KEY_HOME]       = T::kKey_HOME;
    map->keycodes[KEY_END]        = T::kKey_END;
    map->keycodes[KEY_PAGEUP]     = T::kKey_PAGE_UP;
    map->keycodes[KEY_PAGEDOWN]   = T::kKey_PAGE_DOWN;
    map->keycodes[KEY_INSERT]     = T::kKey_INSERT;
    map->keycodes[KEY_LEFT]       = T::kKey_LEFT;
    map->keycodes[KEY_RIGHT]      = T::kKey_RIGHT;
    map->keycodes[KEY_DOWN]       = T::kKey_DOWN;
    map->keycodes[KEY_UP]         = T::kKey_UP;
    map->keycodes[KEY_F1]         = T::kKey_F1;
    map->keycodes[KEY_F2]         = T::kKey_F2;
    map->keycodes[KEY_F3]         = T::kKey_F3;
    map->keycodes[KEY_F4]         = T::kKey_F4;
    map->keycodes[KEY_F5]         = T::kKey_F5;
    map->keycodes[KEY_F6]         = T::kKey_F6;
    map->keycodes[KEY_F7]         = T::kKey_F7;
    map->keycodes[KEY_F8]         = T::kKey_F8;
    map->keycodes[KEY_F9]         = T::kKey_F9;
    map->keycodes[KEY_F10]        = T::kKey_F10;
    map->keycodes[KEY_F11]        = T::kKey_F11;
    map->keycodes[KEY_F12]        = T::kKey_F12;
    map->keycodes[KEY_F13]        = T::kKey_F13;
    map->keycodes[KEY_F14]        = T::kKey_F14;
    map->keycodes[KEY_F15]        = T::kKey_F15;
    map->keycodes[KEY_F16]        = T::kKey_F16;
    map->keycodes[KEY_F17]        = T::kKey_F17;
    map->keycodes[KEY_F18]        = T::kKey_F18;
    map->keycodes[KEY_F19]        = T::kKey_F19;
    map->keycodes[KEY_F20]        = T::kKey_F20;
    map->keycodes[KEY_F21]        = T::kKey_F21;
    map->keycodes[KEY_F22]        = T::kKey_F22;
    map->keycodes[KEY_F23]        = T::kKey_F23;
    map->keycodes[KEY_F24]        = T::kKey_F24;
    map->keycodes[KEY_KPSLASH]    = T::kKey_KP_DIVIDE;
    map->keycodes[KEY_KPASTERISK] = T::kKey_KP_MULTIPLY;
    map->keycodes[KEY_KPMINUS]    = T::kKey_KP_SUBTRACT;
    map->keycodes[KEY_KPPLUS]     = T::kKey_KP_ADD;
    map->keycodes[KEY_KP0]        = T::kKey_KP_0;
    map->keycodes[KEY_KP1]        = T::kKey_KP_1;
    map->keycodes[KEY_KP2]        = T::kKey_KP_2;
    map->keycodes[KEY_KP3]        = T::kKey_KP_3;
    map->keycodes[KEY_KP4]        = T::kKey_KP_4;
    map->keycodes[KEY_KP5]        = T::kKey_KP_5;
    map->keycodes[KEY_KP6]        = T::kKey_KP_6;
    map->keycodes[KEY_KP7]        = T::kKey_KP_7;
    map->keycodes[KEY_KP8]        = T::kKey_KP_8;
    map->keycodes[KEY_KP9]        = T::kKey_KP_9;
    map->keycodes[KEY_KPDOT]      = T::kKey_KP_DECIMAL;
    map->keycodes[KEY_KPEQUAL]    = T::kKey_KP_EQUAL;
    map->keycodes[KEY_KPENTER]    = T::kKey_KP_ENTER;
    map->keycodes[KEY_102ND]      = T::kKey_WORLD_2;

    // Make a reverse map
    for (int16_t scancode = 0; scancode < 256; scancode++)
    {
        if (map->keycodes[scancode] != KeyboardKey::kPlaceholder)
            map->scancodes[static_cast<int16_t>(map->keycodes[scancode])] = scancode;
    }

    return map;
}

std::unique_ptr<KeycodeMap> g_keycode_map;

wl_keyboard_listener g_keyboard_listener = {
    .keymap = &WaylandSeatKeyboardDevice::on_keymap,
    .enter = &WaylandSeatKeyboardDevice::on_enter,
    .leave = &WaylandSeatKeyboardDevice::on_leave,
    .key = &WaylandSeatKeyboardDevice::on_key,
    .modifiers = &WaylandSeatKeyboardDevice::on_modifiers,
    .repeat_info = &WaylandSeatKeyboardDevice::on_repeat_info
};

Shared<WaylandSurface> extract_surface_from_keyboard(void *data, wl_keyboard *keyboard)
{
    CHECK(data);
    auto *listener = LISTENER(data);

    auto display = listener->GetSeat()->GetDisplay();
    return display->GetKeyboardEnteredSurface(keyboard);
}

} // namespace anonymous

WaylandSeatKeyboardDevice::WaylandSeatKeyboardDevice(WaylandSeat *seat,
                                                     wl_keyboard *keyboard)
    : seat_(seat)
    , keyboard_device_(keyboard)
    , keymap_(nullptr)
    , state_(nullptr)
    , compose_state_(nullptr)
    , key_repeat_delay_ms_(0)
    , key_repeat_rate_(0)
    , key_is_repeating_(false)
    , key_repeat_timer_{}
    , repeating_key_(KeyboardKey::kPlaceholder)
    , key_repeat_first_fire_(true)
{
    CHECK(seat_ && keyboard_device_);

    uv_loop_t *event_loop = seat->GetDisplay()->GetEventLoop();
    uv_timer_init(event_loop, &key_repeat_timer_);
    uv_handle_set_data(reinterpret_cast<uv_handle_t*>(&key_repeat_timer_), this);

    if (!g_keycode_map)
        g_keycode_map = KeycodeMap::Create();
}

WaylandSeatKeyboardDevice::~WaylandSeatKeyboardDevice()
{
    TryStopKeyRepeat();

    if (compose_state_)
        xkb_compose_state_unref(compose_state_);

    if (state_)
        xkb_state_unref(state_);

    if (keymap_)
        xkb_keymap_unref(keymap_);

    CHECK(keyboard_device_);
    wl_keyboard_destroy(keyboard_device_);
}

Unique<WaylandSeatKeyboardDevice>
WaylandSeatKeyboardDevice::MakeFromKeyboardDevice(WaylandSeat *seat, wl_keyboard *keyboard)
{
    CHECK(seat && keyboard);

    auto device = std::make_unique<WaylandSeatKeyboardDevice>(seat, keyboard);
    wl_keyboard_add_listener(keyboard, &g_keyboard_listener, device.get());

    return device;
}

void WaylandSeatKeyboardDevice::on_keymap(void *data,
                                          g_maybe_unused wl_keyboard *keyboard,
                                          uint32_t format,
                                          int32_t fd,
                                          uint32_t size)
{
    auto *listener = LISTENER(data);
    CHECK(listener);

    ScopeExitAutoInvoker closer([fd]() {
        ::close(fd);
    });

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
    {
        QLOG(LOG_WARNING, "Compositor reported that no XKB keymaps are available");
        return;
    }

    char *mapped_str = reinterpret_cast<char*>(
            mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (mapped_str == MAP_FAILED)
    {
        QLOG(LOG_ERROR, "Failed to map keymap descriptor");
        return;
    }

    WaylandInputContext *input_context =
            listener->seat_->GetDisplay()->GetInputContext();
    CHECK(input_context);

    xkb_keymap *keymap = xkb_keymap_new_from_string(input_context->GetXkbContext(),
                                                    mapped_str,
                                                    XKB_KEYMAP_FORMAT_TEXT_V1,
                                                    XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(mapped_str, size);

    if (!keymap)
    {
        QLOG(LOG_ERROR, "Failed to compile keymap given by Wayland compositor");
        return;
    }

    xkb_state *state = xkb_state_new(keymap);
    if (!state)
    {
        QLOG(LOG_ERROR, "Failed to create XKB state");
        xkb_keymap_unref(keymap);
        return;
    }

    // Look up the preferred locale, falling back to "C" as default
    const char *locale = getenv("LC_ALL");
    if (!locale)
        locale = getenv("LC_CTYPE");
    if (!locale)
        locale = getenv("LANG");
    if (!locale)
        locale = "C";

    xkb_compose_state *compose_state;
    xkb_compose_table *compose_table =
            xkb_compose_table_new_from_locale(input_context->GetXkbContext(),
                                              locale, XKB_COMPOSE_COMPILE_NO_FLAGS);
    if (compose_table)
    {
        compose_state = xkb_compose_state_new(compose_table, XKB_COMPOSE_STATE_NO_FLAGS);
        xkb_compose_table_unref(compose_table);
        if (compose_state)
            listener->compose_state_ = compose_state;
        else
            QLOG(LOG_ERROR, "Failed to create XKB compose state");
    }
    else
    {
        QLOG(LOG_ERROR, "Failed to create XKB compose table");
    }

    listener->keymap_ = keymap;
    listener->state_ = state;

    listener->mod_indices.control   = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_CTRL);
    listener->mod_indices.alt       = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_ALT);
    listener->mod_indices.shift     = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_SHIFT);
    listener->mod_indices.super     = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_LOGO);
    listener->mod_indices.caps_lock = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_CAPS);
    listener->mod_indices.num_lock  = xkb_keymap_mod_get_index(keymap, "Mod2");
    listener->mod_indices.meta      = xkb_keymap_mod_get_index(keymap, "Meta");
}

void WaylandSeatKeyboardDevice::on_repeat_info(void *data,
                                               g_maybe_unused wl_keyboard *keyboard,
                                               int32_t rate,
                                               int32_t delay)
{
    auto *listener = LISTENER(data);
    CHECK(listener);

    // New parameters will be applied when next `key` event is delivered
    listener->key_repeat_delay_ms_ = delay;
    listener->key_repeat_rate_ = rate;
}

void WaylandSeatKeyboardDevice::on_enter(void *data,
                                         wl_keyboard *keyboard,
                                         g_maybe_unused uint32_t serial,
                                         wl_surface *surface,
                                         g_maybe_unused wl_array *keys)
{
    auto *listener = LISTENER(data);
    CHECK(listener);

    auto surface_object = listener->seat_->FindSurfaceByNativeHandle(surface);
    if (!surface_object)
    {
        QLOG(LOG_ERROR, "Compositor notified us the keyboard focused on a surface"
                        " which is not in the surfaces list");
        return;
    }

    surface_object->SetKeyboardEntered(keyboard);

    RenderClientEmitterInfo emit;
    emit.EmplaceBack<bool>(true);
    surface_object->Emit(GLSI_SURFACE_KEYBOARD_FOCUS, std::move(emit));
}

void WaylandSeatKeyboardDevice::on_leave(void *data,
                                         g_maybe_unused wl_keyboard *keyboard,
                                         g_maybe_unused uint32_t serial,
                                         wl_surface *surface)
{
    auto *listener = LISTENER(data);
    CHECK(listener);

    auto surface_object = listener->seat_->FindSurfaceByNativeHandle(surface);
    if (!surface_object)
    {
        QLOG(LOG_ERROR, "Compositor notified us the keyboard unfocused on a surface"
                        " which is not in the surfaces list");
        return;
    }

    surface_object->SetKeyboardEntered(nullptr);

    listener->TryStopKeyRepeat();

    RenderClientEmitterInfo emit;
    emit.EmplaceBack<bool>(false);
    surface_object->Emit(GLSI_SURFACE_KEYBOARD_FOCUS, std::move(emit));
}

void WaylandSeatKeyboardDevice::on_modifiers(void *data,
                                             wl_keyboard *keyboard,
                                             g_maybe_unused uint32_t serial,
                                             uint32_t mods_depressed,
                                             uint32_t mods_latched,
                                             uint32_t mods_locked,
                                             uint32_t group)
{
    auto *listener = LISTENER(data);
    CHECK(listener);

    if (!listener->keymap_)
        return;

    xkb_state_update_mask(listener->state_,
                          mods_depressed,
                          mods_latched,
                          mods_locked,
                          0,
                          0,
                          group);

    listener->modifiers_.clear();

    struct {
        xkb_mod_index_t   index;
        KeyboardModifiers bit;
    } modifiers_map[] = {
        { listener->mod_indices.control,   KeyboardModifiers::kControl  },
        { listener->mod_indices.alt,       KeyboardModifiers::kAlt      },
        { listener->mod_indices.shift,     KeyboardModifiers::kShift    },
        { listener->mod_indices.super,     KeyboardModifiers::kSuper    },
        { listener->mod_indices.caps_lock, KeyboardModifiers::kCapsLock },
        { listener->mod_indices.num_lock,  KeyboardModifiers::kNumLock  },
        { listener->mod_indices.meta,      KeyboardModifiers::kMeta     }
    };

    for (const auto& entry : modifiers_map)
    {
        if (xkb_state_mod_index_is_active(listener->state_,
                                          entry.index,
                                          XKB_STATE_MODS_EFFECTIVE) == 1)
        {
            listener->modifiers_ |= entry.bit;
        }
    }
}

void WaylandSeatKeyboardDevice::on_key(void *data,
                                       wl_keyboard *keyboard,
                                       g_maybe_unused uint32_t serial,
                                       g_maybe_unused uint32_t time,
                                       uint32_t scancode,
                                       uint32_t state)
{
    auto *listener = LISTENER(data);
    CHECK(listener);

    auto surface = extract_surface_from_keyboard(data, keyboard);
    if (!surface)
    {
        QLOG(LOG_ERROR, "Compositor notified us the change of key states, "
                        "but there is no surface on which the keyboard has focused");
        return;
    }

    bool pressed = (state == WL_KEYBOARD_KEY_STATE_PRESSED);

    if (scancode >= KeycodeMap::kMapSize)
    {
        QLOG(LOG_ERROR, "Invalid key scancode 0x{:x}", scancode);
        return;
    }
    CHECK(g_keycode_map);
    KeyboardKey key = g_keycode_map->keycodes[scancode];

    listener->UpdateKeyRepeat(key, pressed);

    RenderClientEmitterInfo emit;
    emit.EmplaceBack<KeyboardKey>(key);
    emit.EmplaceBack<Bitfield<KeyboardModifiers>>(listener->modifiers_);
    emit.EmplaceBack<bool>(pressed);
    surface->Emit(GLSI_SURFACE_KEYBOARD_KEY, std::move(emit));
}

void WaylandSeatKeyboardDevice::UpdateKeyRepeat(KeyboardKey key, bool pressed)
{
    CHECK(key != KeyboardKey::kPlaceholder);

    if (key_repeat_rate_ == 0)
        return;

    // Filter keys
    int16_t scancode = g_keycode_map->scancodes[static_cast<int16_t>(key)];
    CHECK(scancode > 0);
    const xkb_keycode_t keycode = scancode + 8;
    if (!xkb_keymap_key_repeats(keymap_, keycode))
        return;

    if (key_is_repeating_)
        TryStopKeyRepeat();

    if (!pressed)
        return;

    // Otherwise, replace the repeating key and restart key-repeating
    repeating_key_ = key;
    key_repeat_first_fire_ = true;
    uv_timer_start(&key_repeat_timer_, &repeat_timer_callback, key_repeat_delay_ms_,
                   1000 / key_repeat_rate_);
    key_is_repeating_ = true;
}

void WaylandSeatKeyboardDevice::TryStopKeyRepeat()
{
    if (!key_is_repeating_)
        return;

    uv_timer_stop(&key_repeat_timer_);
    key_is_repeating_ = false;
    repeating_key_ = KeyboardKey::kPlaceholder;
}

void WaylandSeatKeyboardDevice::repeat_timer_callback(uv_timer_t *timer)
{
    auto *listener = LISTENER(
            uv_handle_get_data(reinterpret_cast<uv_handle_t*>(timer)));
    CHECK(listener);

    CHECK(listener->repeating_key_ != KeyboardKey::kPlaceholder);

    Shared<WaylandSurface> surface = listener->seat_->GetDisplay()
            ->GetKeyboardEnteredSurface(listener->keyboard_device_);
    if (!surface)
    {
        // Lost keyboard focus
        listener->TryStopKeyRepeat();
    }

    std::vector<bool> sequence;
    if (listener->key_repeat_first_fire_)
    {
        listener->key_repeat_first_fire_ = false;
        sequence.push_back(false);
    }
    sequence.push_back(true);
    sequence.push_back(false);

    for (bool state : sequence)
    {
        RenderClientEmitterInfo emit;
        emit.PushBack(listener->repeating_key_);
        emit.PushBack(listener->modifiers_);
        emit.PushBack(state);
        surface->Emit(GLSI_SURFACE_KEYBOARD_KEY, std::move(emit));
    }
}

GLAMOR_NAMESPACE_END
