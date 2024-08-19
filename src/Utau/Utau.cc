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

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Utau/Utau.h"
#include "Utau/ffwrappers/libavutil.h"
UTAU_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Utau)

namespace {

thread_local char g_log_buffer[1024];

void av_log_callback(void *avcl, int level, const char *fmt, va_list arg)
{
    if (level >= AV_LOG_DEBUG)
        return;

    LogType type;
    switch (level)
    {
    case AV_LOG_INFO:
        type = LOG_INFO;
        break;
    case AV_LOG_WARNING:
        type = LOG_WARNING;
        break;
    case AV_LOG_ERROR:
    case AV_LOG_FATAL:
    case AV_LOG_PANIC:
        type = LOG_ERROR;
        break;
    default:
        type = LOG_DEBUG;
        break;
    }

    AVClass *_class = avcl ? *reinterpret_cast<AVClass**>(avcl) : nullptr;
    std::string prefix;
    if (_class)
        prefix = fmt::format("%fg<cy,hl>(libav:{})%reset ", _class->item_name(avcl));
    else
        prefix = "%fg<cy,hl>(libav:unknown)%reset ";

    int len = vsnprintf(g_log_buffer, sizeof(g_log_buffer), fmt, arg);

    if (len <= 0)
        return;

    if (g_log_buffer[len - 1] == '\n')
        g_log_buffer[len - 1] = '\0';

    char *sp = &g_log_buffer[0], *q = sp;
    while (*q != '\0')
    {
        // Move `q` to next newline or string terminator
        q = sp;
        while (*q != '\n' && *q != '\0')
            q++;

        char ch = *q;
        *q = '\0';
        QLOG(type, "{}{}", prefix, sp);
        *q = ch;
        sp = q + 1;
    }
}

} // namespace anonymous

void InitializePlatform(const ContextOptions& options)
{
    GlobalContext::New(options);

    av_log_set_callback(av_log_callback);
    av_log_set_level(AV_LOG_INFO);

    QLOG(LOG_INFO, "Supported device type of hardware decoder:");
    AVHWDeviceType hw_device_type = AV_HWDEVICE_TYPE_NONE;
    while ((hw_device_type = av_hwdevice_iterate_types(hw_device_type)) != AV_HWDEVICE_TYPE_NONE) {
        QLOG(LOG_INFO, "  %fg<bl>%italic<>{}%reset",
             av_hwdevice_get_type_name(hw_device_type));
    }
}

void DisposePlatform()
{
    GlobalContext::Delete();
}

GlobalContext::GlobalContext(const ContextOptions& options)
    : options_(options)
{
}

GlobalContext::~GlobalContext()
{
}

UTAU_NAMESPACE_END
