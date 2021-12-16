#include "Core/Errors.h"
#include <xkbcommon/xkbcommon.h>

#include "Core/Journal.h"
#include "Vanilla/Base.h"
#include "Vanilla/KeyboardProxy.h"
VANILLA_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Vanilla)

namespace {

thread_local char local_fmt_buffer[512];
void xkb_journal_forwarder(xkb_context *pCtx, enum xkb_log_level level, const char *fmt, va_list args)
{
    LogType jLevel;
    switch (level)
    {
    case XKB_LOG_LEVEL_INFO:
    case XKB_LOG_LEVEL_CRITICAL:
        jLevel = LOG_INFO;
        break;
    case XKB_LOG_LEVEL_DEBUG:
        jLevel = LOG_DEBUG;
        break;
    case XKB_LOG_LEVEL_ERROR:
        jLevel = LOG_ERROR;
        break;
    case XKB_LOG_LEVEL_WARNING:
        jLevel = LOG_WARNING;
        break;
    }

    char *buffer = local_fmt_buffer;
    size_t bufferSize = 512;
    size_t requireSize = std::vsnprintf(nullptr, 0, fmt, args);

    ScopeEpilogue leave([buffer]() -> void {
        delete[] buffer;
    });
    if (requireSize > bufferSize)
    {
        buffer = new char[requireSize + 1];
        bufferSize = requireSize + 1;
    }
    else
        leave.abolish();

    std::vsnprintf(buffer, bufferSize, fmt, args);
    QLOG(jLevel, "XKB@proxy:{}: {}", fmt::ptr(xkb_context_get_user_data(pCtx)), buffer);
}

} // namespace anonymous

KeyboardProxy::KeyboardProxy(xkb_context *pCtx,
                                 xkb_keymap *pKeymap,
                                 xkb_state *pState)
    : fXkbContext(pCtx),
      fXkbKeymap(pKeymap),
      fXkbState(pState)
{
    xkb_context_set_user_data(fXkbContext, this);
    xkb_context_set_log_level(fXkbContext, XKB_LOG_LEVEL_INFO);
    xkb_context_set_log_fn(fXkbContext, xkb_journal_forwarder);
}

KeyboardProxy::~KeyboardProxy()
{
    xkb_context_set_user_data(fXkbContext, nullptr);
}

Bitfield<KeyModifier> KeyboardProxy::activeMods()
{
    Bitfield<KeyModifier> result;
    if (xkb_state_mod_name_is_active(fXkbState, XKB_MOD_NAME_ALT, XKB_STATE_MODS_EFFECTIVE))
        result |= KeyModifier::kAlt;
    if (xkb_state_mod_name_is_active(fXkbState, XKB_MOD_NAME_CAPS, XKB_STATE_MODS_EFFECTIVE))
        result |= KeyModifier::kCaps;
    if (xkb_state_mod_name_is_active(fXkbState, XKB_MOD_NAME_CTRL, XKB_STATE_MODS_EFFECTIVE))
        result |= KeyModifier::kCtrl;
    if (xkb_state_mod_name_is_active(fXkbState, XKB_MOD_NAME_LOGO, XKB_STATE_MODS_EFFECTIVE))
        result |= KeyModifier::kLogo;
    if (xkb_state_mod_name_is_active(fXkbState, XKB_MOD_NAME_NUM, XKB_STATE_MODS_EFFECTIVE))
        result |= KeyModifier::kNum;
    if (xkb_state_mod_name_is_active(fXkbState, XKB_MOD_NAME_SHIFT, XKB_STATE_MODS_EFFECTIVE))
        result |= KeyModifier::kShift;

    return result;
}

Bitfield<KeyLed> KeyboardProxy::activeLeds()
{
    Bitfield<KeyLed> result;
    if (xkb_state_led_name_is_active(fXkbState, XKB_LED_NAME_CAPS))
        result |= KeyLed::kCapsLock;
    if (xkb_state_led_name_is_active(fXkbState, XKB_LED_NAME_NUM))
        result |= KeyLed::kNumLock;
    if (xkb_state_led_name_is_active(fXkbState, XKB_LED_NAME_SCROLL))
        result |= KeyLed::kScrollLock;

    return result;
}

VANILLA_NS_END
