#include <locale>

#include "Gsk/GskKeymap.h"
#include "Gsk/GskKey.h"
#include "Core/Utils.h"
#include "Gsk/GskKeynameTable.h"

GSK_NAMESPACE_BEGIN

GskKeymap::GskKeymap(const Weak<GskDisplay>& display)
    : fDisplay(display)
{
    fConnection = signalKeysChanged().connect([](const Handle<GskKeymap>& ptr) {
        ptr->fCache.clear();
        ptr->fCachedKeys.clear();
    });
}

GskKeymap::~GskKeymap()
{
    fConnection.disconnect();
}

void GskKeymap::getCachedEntriesForKeyval(uint32_t keyval, std::vector<GskKeymapKey>& out)
{
    size_t len;
    size_t offset;
    size_t cached;

    if (!utils::MapContains(fCache, keyval))
    {
        offset = fCachedKeys.size();

        getEntriesForKeyval(keyval, fCachedKeys);

        len = fCachedKeys.size() - offset;
        CHECK(len <= 255);

        cached = (offset << 8) | len;
        fCache[keyval] = cached;
    }
    else
    {
        cached = fCache[keyval];
        len = cached & 255;
        offset = cached >> 8;
    }

    for (auto i = offset; i < offset + len; i++)
        out.push_back(fCachedKeys[i]);
}

uint32_t KeyvalToUpper(uint32_t keyval)
{
    uint32_t result;

    KeyvalConvertCase(keyval, nullptr, &result);
    return result;
}

uint32_t KeyvalToLower(uint32_t keyval)
{
    uint32_t result;

    KeyvalConvertCase(keyval, &result, nullptr);
    return result;
}

bool KeyvalIsUpper(uint32_t keyval)
{
    if (keyval)
    {
        uint32_t upperVal = 0;
        KeyvalConvertCase(keyval, nullptr, &upperVal);
        return (upperVal == keyval);
    }
    return false;
}

bool KeyvalIsLower(uint32_t keyval)
{
    if (keyval)
    {
        uint32_t lowerVal = 0;
        KeyvalConvertCase(keyval, &lowerVal, nullptr);
        return (lowerVal == keyval);
    }
    return false;
}

#define GSK_NUM_KEYS (sizeof(keys_by_keyval_map_) / sizeof(KeyvalNameMap))

namespace {
int keyval_compare(const void *pkey, const void *pbase)
{
    return (int)((*(uint32_t*) pkey) - ((KeyvalNameMap *) pbase)->keyval);
}

int keys_name_compare(const void *pkey, const void *pbase)
{
    return std::strcmp((const char *) pkey,
                       (const char *) (keynames_ + ((const KeyvalNameMap *) pbase)->offset));
}
} // namespace anonymous

const char *KeyvalName(uint32_t keyval)
{
    static char buf[100];

    /* Check for directly encoded 24-bit UCS characters: */
    if ((keyval & 0xff000000) == 0x01000000)
    {
        std::sprintf(buf, "U+%.04X", (keyval & 0x00ffffff));
        return buf;
    }

    auto *found = reinterpret_cast<KeyvalNameMap*>(std::bsearch(&keyval, keys_by_keyval_map_, GSK_NUM_KEYS,
                                                                sizeof(KeyvalNameMap), keyval_compare));

    if (found != nullptr)
    {
        while ((found > keys_by_keyval_map_) && (found - 1)->keyval == keyval)
            found--;
        return reinterpret_cast<const char*>(keynames_ + found->offset);
    }
    else if (keyval != 0)
    {
        std::sprintf(buf, "%#x", keyval);
        return buf;
    }
    return nullptr;
}

uint32_t KeyvalFromName(std::string name)
{
    if (name.empty())
        return 0;

    if (std::strncmp(name.c_str(), "XF86", 4) == 0)
        name = name.substr(4);

    auto *found = reinterpret_cast<KeyvalNameMap*>(std::bsearch(name.c_str(), keys_by_name_map_, GSK_NUM_KEYS,
                                                                sizeof(KeyvalNameMap), keys_name_compare));

    if (found != nullptr)
        return found->keyval;
    else
        return GSK_KEY_VoidSymbol;
}

void KeyvalConvertCase(uint32_t symbol, uint32_t *lower, uint32_t *upper)
{
    uint32_t xlower, xupper;

    xlower = symbol;
    xupper = symbol;

    /* Check for directly encoded 24-bit UCS characters: */
    if ((symbol & 0xff000000) == 0x01000000)
    {
        // FIXME: This won't work on platforms which don't have en_US.UTF8 locale.
        auto& facet = std::use_facet<std::ctype<wchar_t>>(std::locale());;
        if (lower)
            *lower = UnicodeToKeyval(facet.tolower(static_cast<wchar_t>(symbol & 0x00ffffff)));
        if (upper)
            *upper = UnicodeToKeyval(facet.toupper(static_cast<wchar_t>(symbol & 0x00ffffff)));
        return;
    }

    switch (symbol >> 8)
    {
    case 0: /* Latin 1 */
        if ((symbol >= GSK_KEY_A) && (symbol <= GSK_KEY_Z))
            xlower += (GSK_KEY_a - GSK_KEY_A);
        else if ((symbol >= GSK_KEY_a) && (symbol <= GSK_KEY_z))
            xupper -= (GSK_KEY_a - GSK_KEY_A);
        else if ((symbol >= GSK_KEY_Agrave) && (symbol <= GSK_KEY_Odiaeresis))
            xlower += (GSK_KEY_agrave - GSK_KEY_Agrave);
        else if ((symbol >= GSK_KEY_agrave) && (symbol <= GSK_KEY_odiaeresis))
            xupper -= (GSK_KEY_agrave - GSK_KEY_Agrave);
        else if ((symbol >= GSK_KEY_Ooblique) && (symbol <= GSK_KEY_Thorn))
            xlower += (GSK_KEY_oslash - GSK_KEY_Ooblique);
        else if ((symbol >= GSK_KEY_oslash) && (symbol <= GSK_KEY_thorn))
            xupper -= (GSK_KEY_oslash - GSK_KEY_Ooblique);
        break;

    case 1: /* Latin 2 */
        /* Assume the KeySym is a legal value (ignore discontinuities) */
        if (symbol == GSK_KEY_Aogonek)
            xlower = GSK_KEY_aogonek;
        else if (symbol >= GSK_KEY_Lstroke && symbol <= GSK_KEY_Sacute)
            xlower += (GSK_KEY_lstroke - GSK_KEY_Lstroke);
        else if (symbol >= GSK_KEY_Scaron && symbol <= GSK_KEY_Zacute)
            xlower += (GSK_KEY_scaron - GSK_KEY_Scaron);
        else if (symbol >= GSK_KEY_Zcaron && symbol <= GSK_KEY_Zabovedot)
            xlower += (GSK_KEY_zcaron - GSK_KEY_Zcaron);
        else if (symbol == GSK_KEY_aogonek)
            xupper = GSK_KEY_Aogonek;
        else if (symbol >= GSK_KEY_lstroke && symbol <= GSK_KEY_sacute)
            xupper -= (GSK_KEY_lstroke - GSK_KEY_Lstroke);
        else if (symbol >= GSK_KEY_scaron && symbol <= GSK_KEY_zacute)
            xupper -= (GSK_KEY_scaron - GSK_KEY_Scaron);
        else if (symbol >= GSK_KEY_zcaron && symbol <= GSK_KEY_zabovedot)
            xupper -= (GSK_KEY_zcaron - GSK_KEY_Zcaron);
        else if (symbol >= GSK_KEY_Racute && symbol <= GSK_KEY_Tcedilla)
            xlower += (GSK_KEY_racute - GSK_KEY_Racute);
        else if (symbol >= GSK_KEY_racute && symbol <= GSK_KEY_tcedilla)
            xupper -= (GSK_KEY_racute - GSK_KEY_Racute);
        break;

    case 2: /* Latin 3 */
        /* Assume the KeySym is a legal value (ignore discontinuities) */
        if (symbol >= GSK_KEY_Hstroke && symbol <= GSK_KEY_Hcircumflex)
            xlower += (GSK_KEY_hstroke - GSK_KEY_Hstroke);
        else if (symbol >= GSK_KEY_Gbreve && symbol <= GSK_KEY_Jcircumflex)
            xlower += (GSK_KEY_gbreve - GSK_KEY_Gbreve);
        else if (symbol >= GSK_KEY_hstroke && symbol <= GSK_KEY_hcircumflex)
            xupper -= (GSK_KEY_hstroke - GSK_KEY_Hstroke);
        else if (symbol >= GSK_KEY_gbreve && symbol <= GSK_KEY_jcircumflex)
            xupper -= (GSK_KEY_gbreve - GSK_KEY_Gbreve);
        else if (symbol >= GSK_KEY_Cabovedot && symbol <= GSK_KEY_Scircumflex)
            xlower += (GSK_KEY_cabovedot - GSK_KEY_Cabovedot);
        else if (symbol >= GSK_KEY_cabovedot && symbol <= GSK_KEY_scircumflex)
            xupper -= (GSK_KEY_cabovedot - GSK_KEY_Cabovedot);
        break;

    case 3: /* Latin 4 */
        /* Assume the KeySym is a legal value (ignore discontinuities) */
        if (symbol >= GSK_KEY_Rcedilla && symbol <= GSK_KEY_Tslash)
            xlower += (GSK_KEY_rcedilla - GSK_KEY_Rcedilla);
        else if (symbol >= GSK_KEY_rcedilla && symbol <= GSK_KEY_tslash)
            xupper -= (GSK_KEY_rcedilla - GSK_KEY_Rcedilla);
        else if (symbol == GSK_KEY_ENG)
            xlower = GSK_KEY_eng;
        else if (symbol == GSK_KEY_eng)
            xupper = GSK_KEY_ENG;
        else if (symbol >= GSK_KEY_Amacron && symbol <= GSK_KEY_Umacron)
            xlower += (GSK_KEY_amacron - GSK_KEY_Amacron);
        else if (symbol >= GSK_KEY_amacron && symbol <= GSK_KEY_umacron)
            xupper -= (GSK_KEY_amacron - GSK_KEY_Amacron);
        break;

    case 6: /* Cyrillic */
        /* Assume the KeySym is a legal value (ignore discontinuities) */
        if (symbol >= GSK_KEY_Serbian_DJE && symbol <= GSK_KEY_Serbian_DZE)
            xlower -= (GSK_KEY_Serbian_DJE - GSK_KEY_Serbian_dje);
        else if (symbol >= GSK_KEY_Serbian_dje && symbol <= GSK_KEY_Serbian_dze)
            xupper += (GSK_KEY_Serbian_DJE - GSK_KEY_Serbian_dje);
        else if (symbol >= GSK_KEY_Cyrillic_YU && symbol <= GSK_KEY_Cyrillic_HARDSIGN)
            xlower -= (GSK_KEY_Cyrillic_YU - GSK_KEY_Cyrillic_yu);
        else if (symbol >= GSK_KEY_Cyrillic_yu && symbol <= GSK_KEY_Cyrillic_hardsign)
            xupper += (GSK_KEY_Cyrillic_YU - GSK_KEY_Cyrillic_yu);
        break;

    case 7: /* Greek */
        /* Assume the KeySym is a legal value (ignore discontinuities) */
        if (symbol >= GSK_KEY_Greek_ALPHAaccent && symbol <= GSK_KEY_Greek_OMEGAaccent)
            xlower += (GSK_KEY_Greek_alphaaccent - GSK_KEY_Greek_ALPHAaccent);
        else if (symbol >= GSK_KEY_Greek_alphaaccent && symbol <= GSK_KEY_Greek_omegaaccent &&
                 symbol != GSK_KEY_Greek_iotaaccentdieresis &&
                 symbol != GSK_KEY_Greek_upsilonaccentdieresis)
            xupper -= (GSK_KEY_Greek_alphaaccent - GSK_KEY_Greek_ALPHAaccent);
        else if (symbol >= GSK_KEY_Greek_ALPHA && symbol <= GSK_KEY_Greek_OMEGA)
            xlower += (GSK_KEY_Greek_alpha - GSK_KEY_Greek_ALPHA);
        else if (symbol == GSK_KEY_Greek_finalsmallsigma)
            xupper = GSK_KEY_Greek_SIGMA;
        else if (symbol >= GSK_KEY_Greek_alpha && symbol <= GSK_KEY_Greek_omega)
            xupper -= (GSK_KEY_Greek_alpha - GSK_KEY_Greek_ALPHA);
        break;

    default:
        break;
    }

    if (lower)
        *lower = xlower;
    if (upper)
        *upper = xupper;
}

GSK_NAMESPACE_END
