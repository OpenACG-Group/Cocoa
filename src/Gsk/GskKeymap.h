#ifndef COCOA_GSKKEYMAP_H
#define COCOA_GSKKEYMAP_H

#include <map>
#include <sigc++/sigc++.h>

#include "Core/Errors.h"
#include "Core/EnumClassBitfield.h"
#include "Gsk/Gsk.h"
#include "Gsk/GskKey.h"
#include "Gsk/GskEnumerations.h"
GSK_NAMESPACE_BEGIN

class GskDisplay;

class GskKeymap
{
public:
    explicit GskKeymap(const Weak<GskDisplay>& display);
    virtual ~GskKeymap();

    g_inline Handle<GskDisplay> getDisplay() const {
        CHECK(!fDisplay.expired());
        return fDisplay.lock();
    }

    virtual TextDirection getDirection() = 0;
    virtual bool hasBidiLayouts() = 0;
    virtual bool getCapsLockState() = 0;
    virtual bool getNumLockState() = 0;
    virtual bool getScrollLockState() = 0;
    virtual bool getEntriesForKeyval(uint32_t keyval, std::vector<GskKeymapKey>& out) = 0;
    virtual bool getEntriesForKeycode(uint32_t hardwareKeycode,
                                      std::vector<std::pair<GskKeymapKey, uint32_t>>& out) = 0;
    virtual uint32_t lookupKey(const GskKeymapKey *key) = 0;
    virtual bool translateKeyboardState(uint32_t hardwareKeycode,
                                        Bitfield<GskModifierType> state,
                                        int group,
                                        uint32_t& keyval,
                                        int& effectiveGroup,
                                        int& level,
                                        Bitfield<GskModifierType>& consumedModifiers) = 0;
    virtual Bitfield<GskModifierType> getModifierState() = 0;

    void getCachedEntriesForKeyval(uint32_t keyval, std::vector<GskKeymapKey>& out);


    g_signal_getter(DirectionChanged);
    g_signal_getter(KeysChanged);
    g_signal_getter(StateChanged);

protected:
    g_signal_fields(
        g_signal_signature(void(const Handle<GskKeymap>&), DirectionChanged)
        g_signal_signature(void(const Handle<GskKeymap>&), KeysChanged)
        g_signal_signature(void(const Handle<GskKeymap>&), StateChanged)
    )

private:
    Weak<GskDisplay>    fDisplay;
    std::vector<GskKeymapKey>       fCachedKeys;
    std::map<uint32_t, uint32_t>    fCache;
    sigc::connection     fConnection;
};

GSK_NAMESPACE_END
#endif //COCOA_GSKKEYMAP_H
