#ifndef COCOA_GSKKEY_H
#define COCOA_GSKKEY_H

#include "Gsk/Gsk.h"
#include "Gsk/GskKeySymbols.h"
GSK_NAMESPACE_BEGIN

struct KeyvalNameMap
{
    uint32_t  keyval;
    uint32_t  offset;
};

/* A `GskKeymapKey` is a hardware key that can be mapped to a keyval. */
struct GskKeymapKey
{
    GskKeymapKey(const GskKeymapKey& other) = default;

    /* The hardware keycode. This is an identifying number for a physical key. */
    uint32_t keycode;
    /* Indicates movement in a horizontal direction.Usually groups are used for two
     * different languages. In group 0, a key might have two English characters, and in group
     * 1 it might have two Hebrew characters. The Hebrew characters will be printed on the
     * key next to the English characters. */
    uint32_t group;
    /* Indicates which symbol on the key will be used, in a vertical direction.
     * So on a standard US keyboard, the key with the number "1" on it also has the
     * exclamation point ("!") character on it. The level indicates whether to use
     * the "1" or the "!" symbol. The letter keys are considered to have a lowercase
     * letter at level 0, and an uppercase letter at level 1, though only the uppercase
     * letter is printed. */
    uint32_t level;
};

const char *KeyvalName(uint32_t keyval);
uint32_t KeyvalFromName(std::string name);

void KeyvalConvertCase(uint32_t symbol, uint32_t *lower, uint32_t *upper);

uint32_t KeyvalToUpper(uint32_t keyval);
uint32_t KeyvalToLower(uint32_t keyval);

bool KeyvalIsUpper(uint32_t keyval);
bool KeyvalIsLower(uint32_t keyval);

uint32_t KeyvalToUnicode(uint32_t keyval);
uint32_t UnicodeToKeyval(uint32_t wc);

GSK_NAMESPACE_END
#endif //COCOA_GSKKEY_H
