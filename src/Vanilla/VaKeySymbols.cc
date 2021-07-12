#include <cstring>
#include <xkbcommon/xkbcommon.h>

#include "Vanilla/Base.h"
#include "Vanilla/VaKeySymbols.h"
VANILLA_NS_BEGIN

namespace {
thread_local char localNameBuffer[64];
} // namespace anonymous

const char *GetKeySymbolName(KeySymbol sym)
{
    std::memset(localNameBuffer, '\0', sizeof(localNameBuffer));
    int ret = xkb_keysym_get_name(va_translate_from_key_symbol(sym),
                                  localNameBuffer,
                                  sizeof(localNameBuffer));
    if (ret < 0)
        return nullptr;
    return localNameBuffer;
}

VANILLA_NS_END
