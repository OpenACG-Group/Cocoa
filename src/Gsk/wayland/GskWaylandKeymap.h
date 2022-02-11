#ifndef COCOA_GSKWAYLANDKEYMAP_H
#define COCOA_GSKWAYLANDKEYMAP_H

#include <xkbcommon/xkbcommon.h>

#include "Gsk/GskKeymap.h"
GSK_NAMESPACE_BEGIN

class GskWaylandKeymap : public GskKeymap
{
public:
    explicit GskWaylandKeymap(const Weak<GskDisplay>& display);
    ~GskWaylandKeymap() override;

    TextDirection getDirection() override;
    bool hasBidiLayouts() override;
    bool getNumLockState() override;
    bool getCapsLockState() override;
    bool getScrollLockState() override;
    bool getEntriesForKeyval(uint32_t keyval, std::vector<GskKeymapKey> &out) override;
    bool getEntriesForKeycode(uint32_t hardwareKeycode, std::vector<std::pair<GskKeymapKey, uint32_t>> &out) override;
    uint32_t lookupKey(const GskKeymapKey *key) override;
    bool translateKeyboardState(uint32_t hardwareKeycode,
                                Bitfield<GskModifierType> state,
                                int group,
                                uint32_t& keyval,
                                int& effectiveGroup,
                                int& level,
                                Bitfield<GskModifierType>& consumedModifiers) override;
    Bitfield<GskModifierType> getModifierState() override;

    Bitfield<GskModifierType> getGskModifiers(uint32_t mods);
    void updateFromFd(uint32_t format, uint32_t fd, uint32_t size);
    bool keyIsModifier(uint32_t keycode);

    g_nodiscard g_inline xkb_keymap *getXKBKeymap() const {
        return fXKBKeymap;
    }

    g_nodiscard g_inline xkb_state *getXKBState() const {
        return fXKBState;
    }

private:
    void updateDirection();
    uint32_t getXKBModifiers(Bitfield<GskModifierType> state);

    xkb_keymap      *fXKBKeymap;
    xkb_state       *fXKBState;
    std::vector<TextDirection> fDirection;
    bool             fBidi;
};

GSK_NAMESPACE_END
#endif //COCOA_GSKWAYLANDKEYMAP_H
