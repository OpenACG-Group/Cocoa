#include <fribidi/fribidi.h>

#include "Core/Journal.h"
#include "Core/Filesystem.h"
#include "Core/Exception.h"
#include "Gsk/wayland/GskWaylandKeymap.h"
GSK_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gsk.Wayland.Keymap)

namespace {

TextDirection unicodeDirection(uint32_t ch)
{
    static_assert(sizeof(FriBidiChar) == sizeof(uint32_t));

    FriBidiCharType bidiChar = fribidi_get_bidi_type(ch);
    if (!FRIBIDI_IS_STRONG(bidiChar))
        return TextDirection::kNeutral;
    else if (FRIBIDI_IS_RTL(bidiChar))
        return TextDirection::kRTL;
    else
        return TextDirection::kLTR;
}

}

GskWaylandKeymap::GskWaylandKeymap(const Weak<GskDisplay>& display)
    : GskKeymap(display)
    , fXKBKeymap(nullptr)
    , fXKBState(nullptr)
    , fBidi(false)
{
    xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    xkb_rule_names names{};
    names.rules = "evdev";
    names.model = "pc105";
    names.layout = "us";
    names.variant = "";
    names.options = "";
    fXKBKeymap = xkb_keymap_new_from_names(context, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
    fXKBState = xkb_state_new(fXKBKeymap);
    xkb_context_unref(context);

    updateDirection();
}

GskWaylandKeymap::~GskWaylandKeymap()
{
    xkb_keymap_unref(fXKBKeymap);
    xkb_state_unref(fXKBState);
}

void GskWaylandKeymap::updateDirection()
{
    fDirection.clear();

    xkb_layout_index_t numLayouts = xkb_keymap_num_layouts(fXKBKeymap);
    std::vector<int> rtl(numLayouts, 0);

    xkb_keycode_t minKeycode = xkb_keymap_min_keycode(fXKBKeymap);
    xkb_keycode_t maxKeycode = xkb_keymap_max_keycode(fXKBKeymap);
    for (uint32_t key = minKeycode; key < maxKeycode; key++)
    {
        xkb_layout_index_t layouts = xkb_keymap_num_layouts_for_key(fXKBKeymap, key);
        CHECK(layouts <= numLayouts);

        for (xkb_layout_index_t layout = 0; layout < layouts; layout++)
        {
            const xkb_keysym_t *syms;
            int numSyms = xkb_keymap_key_get_syms_by_level(fXKBKeymap, key, layout, 0, &syms);

            for (int sym = 0; sym < numSyms; sym++)
            {
                TextDirection dir;
                uint32_t unicode = xkb_keysym_to_utf32(syms[sym]);

                switch (unicodeDirection(unicode))
                {
                case TextDirection::kLTR:
                    rtl[layout]++;
                    break;
                case TextDirection::kRTL:
                    rtl[layout]--;
                    break;
                case TextDirection::kWeakLTR:
                case TextDirection::kWeakRTL:
                case TextDirection::kNeutral:
                    break;
                }
            }
        }
    }

    bool haveRTL = false, haveLTR = false;
    for (int i = 0; i < numLayouts; i++)
    {
        if (rtl[i] > 0)
        {
            fDirection[i] = TextDirection::kNeutral;
            haveRTL = true;
        }
        else
        {
            fDirection[i] = TextDirection::kLTR;
            haveLTR = true;
        }
    }
    if (haveRTL && haveLTR)
        fBidi = true;
}

TextDirection GskWaylandKeymap::getDirection()
{
    for (int i = 0; i < xkb_keymap_num_layouts(fXKBKeymap); i++)
    {
        if (xkb_state_layout_index_is_active(fXKBState, i, XKB_STATE_LAYOUT_EFFECTIVE))
            return fDirection[i];
    }
    return TextDirection::kNeutral;
}

bool GskWaylandKeymap::hasBidiLayouts()
{
    return fBidi;
}

bool GskWaylandKeymap::getNumLockState()
{
    return xkb_state_led_name_is_active(fXKBState, XKB_LED_NAME_NUM);
}

bool GskWaylandKeymap::getCapsLockState()
{
    return xkb_state_led_name_is_active(fXKBState, XKB_LED_NAME_CAPS);
}

bool GskWaylandKeymap::getScrollLockState()
{
    return xkb_state_led_name_is_active(fXKBState, XKB_LED_NAME_SCROLL);
}

bool GskWaylandKeymap::getEntriesForKeyval(uint32_t keyval, std::vector<GskKeymapKey>& out)
{
    size_t len = out.size();

    xkb_keycode_t minKeycode = xkb_keymap_min_keycode(fXKBKeymap);
    xkb_keycode_t maxKeycode = xkb_keymap_max_keycode(fXKBKeymap);
    for (xkb_keycode_t keycode = minKeycode; keycode < maxKeycode; keycode++)
    {
        xkb_layout_index_t numLayouts = xkb_keymap_num_layouts_for_key(fXKBKeymap, keycode);
        for (xkb_layout_index_t layout = 0; layout < numLayouts; layout++)
        {
            xkb_level_index_t numLevels = xkb_keymap_num_levels_for_key(fXKBKeymap, keycode, layout);
            for (xkb_level_index_t level = 0; level < numLevels; level++)
            {
                const xkb_keysym_t *syms;
                int numSyms = xkb_keymap_key_get_syms_by_level(fXKBKeymap, keycode, layout, level, &syms);
                for (int sym = 0; sym < numSyms; sym++)
                {
                    if (syms[sym] == keyval)
                    {
                        out.push_back({
                            .keycode = keycode,
                            .group = layout,
                            .level = level
                        });
                    }
                }
            }
        }
    }

    return out.size() > len;
}

bool GskWaylandKeymap::getEntriesForKeycode(uint32_t hardwareKeycode,
                                            std::vector<std::pair<GskKeymapKey, uint32_t>>& out)
{
    xkb_layout_index_t numLayouts = xkb_keymap_num_layouts_for_key(fXKBKeymap, hardwareKeycode);
    int numEntries = 0;
    for (xkb_layout_index_t layout = 0; layout < numLayouts; layout++)
        numEntries += static_cast<int>(xkb_keymap_num_levels_for_key(fXKBKeymap, hardwareKeycode, layout));

    int i = 0;
    for (xkb_layout_index_t layout = 0; layout < numLayouts; layout++)
    {
        xkb_level_index_t numLevels = xkb_keymap_num_levels_for_key(fXKBKeymap, hardwareKeycode, layout);
        for (xkb_level_index_t level = 0; level < numLevels; level++)
        {
            const xkb_keysym_t *syms;
            int numSyms = xkb_keymap_key_get_syms_by_level(fXKBKeymap, hardwareKeycode, layout, 0, &syms);
            out.emplace_back(std::make_pair(GskKeymapKey{ hardwareKeycode, layout, level },
                                            numSyms > 0 ? syms[0] : 0));
            i++;
        }
    }

    return numEntries > 0;
}

uint32_t GskWaylandKeymap::lookupKey(const GskKeymapKey *key)
{
    const xkb_keysym_t *syms;
    int numSyms = xkb_keymap_key_get_syms_by_level(fXKBKeymap,
                                                   key->keycode,
                                                   key->group,
                                                   key->level,
                                                   &syms);
    if (numSyms > 0)
        return syms[0];
    else
        return XKB_KEY_NoSymbol;
}

uint32_t GskWaylandKeymap::getXKBModifiers(Bitfield<GskModifierType> state)
{
    uint32_t mods = 0;
    if (state & GskModifierType::kShiftMask)
        mods |= 1 << xkb_keymap_mod_get_index (fXKBKeymap, XKB_MOD_NAME_SHIFT);
    if (state & GskModifierType::kLockMask)
        mods |= 1 << xkb_keymap_mod_get_index (fXKBKeymap, XKB_MOD_NAME_CAPS);
    if (state & GskModifierType::kControlMask)
        mods |= 1 << xkb_keymap_mod_get_index (fXKBKeymap, XKB_MOD_NAME_CTRL);
    if (state & GskModifierType::kAltMask)
        mods |= 1 << xkb_keymap_mod_get_index (fXKBKeymap, XKB_MOD_NAME_ALT);
    if (state & GskModifierType::kSuperMask)
        mods |= 1 << xkb_keymap_mod_get_index (fXKBKeymap, "Super");
    if (state & GskModifierType::kHyperMask)
        mods |= 1 << xkb_keymap_mod_get_index (fXKBKeymap, "Hyper");
    if (state & GskModifierType::kMetaMask)
        mods |= 1 << xkb_keymap_mod_get_index (fXKBKeymap, "Meta");

    return mods;
}

Bitfield<GskModifierType> GskWaylandKeymap::getGskModifiers(uint32_t mods)
{
    Bitfield<GskModifierType> state;

    if (mods & (1 << xkb_keymap_mod_get_index (fXKBKeymap, XKB_MOD_NAME_SHIFT)))
        state |= GskModifierType::kShiftMask;
    if (mods & (1 << xkb_keymap_mod_get_index (fXKBKeymap, XKB_MOD_NAME_CAPS)))
        state |= GskModifierType::kLockMask;
    if (mods & (1 << xkb_keymap_mod_get_index (fXKBKeymap, XKB_MOD_NAME_CTRL)))
        state |= GskModifierType::kControlMask;
    if (mods & (1 << xkb_keymap_mod_get_index (fXKBKeymap, XKB_MOD_NAME_ALT)))
        state |= GskModifierType::kAltMask;
    if (mods & (1 << xkb_keymap_mod_get_index (fXKBKeymap, "Super")))
        state |= GskModifierType::kSuperMask;
    if (mods & (1 << xkb_keymap_mod_get_index (fXKBKeymap, "Hyper")))
        state |= GskModifierType::kHyperMask;

    /* GSK treats MOD1 as a synonym for Alt, and does not expect it to
     * be mapped around, so we should avoid adding GskModifierType::kMetaMask
     * if MOD1 is already included to avoid confusing GSK and applications that
     * rely on that behavior.
     */
    if (mods & (1 << xkb_keymap_mod_get_index (fXKBKeymap, "Meta")) &&
        (state & GskModifierType::kAltMask) == 0)
        state |= GskModifierType::kMetaMask;

    return state;
}

bool GskWaylandKeymap::translateKeyboardState(uint32_t hardwareKeycode,
                                              Bitfield<GskModifierType> state,
                                              int group,
                                              uint32_t& keyval,
                                              int& effectiveGroup,
                                              int& effectiveLevel,
                                              Bitfield<GskModifierType>& consumedModifiers)
{
    if (group >= 4)
        return false;

    uint32_t modifiers = getXKBModifiers(state);

    xkb_state *xkbState = xkb_state_new(fXKBKeymap);

    xkb_state_update_mask(xkbState, modifiers, 0, 0, group, 0, 0);

    xkb_layout_index_t layout = xkb_state_key_get_layout(xkbState, hardwareKeycode);
    xkb_level_index_t level = xkb_state_key_get_level(xkbState, hardwareKeycode, layout);
    xkb_keysym_t sym = xkb_state_key_get_one_sym(xkbState, hardwareKeycode);
    uint32_t consumed = modifiers & !xkb_state_mod_mask_remove_consumed(xkbState, hardwareKeycode, modifiers);

    xkb_state_unref(xkbState);

    keyval = sym;
    effectiveGroup = static_cast<int>(layout);
    effectiveLevel = static_cast<int>(level);
    consumedModifiers = getGskModifiers(consumed);

    return (sym != XKB_KEY_NoSymbol);
}

Bitfield<GskModifierType> GskWaylandKeymap::getModifierState()
{
    xkb_mod_mask_t mods = xkb_state_serialize_mods(fXKBState, XKB_STATE_MODS_EFFECTIVE);
    return getGskModifiers(mods);
}

void GskWaylandKeymap::updateFromFd(uint32_t format, uint32_t fd, uint32_t size)
{
    xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    ScopeEpilogue scope([context] {
        xkb_context_unref(context);
    });

    char *map_ptr = (char *) vfs::MemMap(static_cast<int32_t>(fd),
                                         nullptr,
                                         {vfs::MapProtection::kRead},
                                         {vfs::MapFlags::kPrivate},
                                         size,
                                         0);
    if (vfs::MemMapHasFailed(map_ptr))
    {
        vfs::Close(static_cast<int32_t>(fd));
        return;
    }

    xkb_keymap *keymap = xkb_keymap_new_from_string(context,
                                                    map_ptr,
                                                    static_cast<xkb_keymap_format>(format),
                                                    XKB_KEYMAP_COMPILE_NO_FLAGS);
    vfs::MemUnmap(map_ptr, size);
    vfs::Close(static_cast<int32_t>(fd));

    if (!keymap)
    {
        QLOG(LOG_WARNING, "Got invalid keymap from compositor, keeping previous/default one");
        return;
    }

    xkb_keymap_unref(fXKBKeymap);
    fXKBKeymap = keymap;

    xkb_state_unref(fXKBState);
    fXKBState = xkb_state_new(fXKBKeymap);

    updateDirection();
}

bool GskWaylandKeymap::keyIsModifier(uint32_t keycode)
{
    xkb_state *state = xkb_state_new(fXKBKeymap);
    ScopeEpilogue scope([state] { xkb_state_unref(state); });
    return (xkb_state_update_key(state, keycode, XKB_KEY_DOWN) & XKB_STATE_MODS_EFFECTIVE);
}

GSK_NAMESPACE_END
