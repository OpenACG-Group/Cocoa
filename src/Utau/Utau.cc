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

struct SampleFormatInfo
{
    SampleFormat fmt;
    int size_per_sample;
    bool planar;
    AVSampleFormat libav_format;
} const g_sample_format_info[] = {
    { SampleFormat::kUnknown, 0, false, AV_SAMPLE_FMT_NONE  },

    // Interleaved formats
    { SampleFormat::kU8,    1,  false, AV_SAMPLE_FMT_U8   },
    { SampleFormat::kS16,   2,  false, AV_SAMPLE_FMT_S16  },
    { SampleFormat::kS32,   4,  false, AV_SAMPLE_FMT_S32  },
    { SampleFormat::kF32,   4,  false, AV_SAMPLE_FMT_FLT  },
    { SampleFormat::kF64,   8,  false, AV_SAMPLE_FMT_DBL  },

    // Planar formats
    { SampleFormat::kU8P,   1,  true,  AV_SAMPLE_FMT_U8P  },
    { SampleFormat::kS16P,  2,  true,  AV_SAMPLE_FMT_S16P },
    { SampleFormat::kS32P,  4,  true,  AV_SAMPLE_FMT_S32P },
    { SampleFormat::kF32P,  4,  true,  AV_SAMPLE_FMT_FLTP },
    { SampleFormat::kF64P,  8,  true,  AV_SAMPLE_FMT_DBLP }
};

const SampleFormatInfo *find_sample_format_info(SampleFormat format)
{
    for (const auto& entry : g_sample_format_info)
    {
        if (entry.fmt == format)
            return &entry;
    }

    MARK_UNREACHABLE();
}

} // namespace anonymous

int GetPerSampleSize(SampleFormat fmt)
{
    return find_sample_format_info(fmt)->size_per_sample;
}

bool SampleFormatIsPlanar(SampleFormat format)
{
    return find_sample_format_info(format)->planar;
}

AVSampleFormat SampleFormatToLibavFormat(SampleFormat format)
{
    return find_sample_format_info(format)->libav_format;
}

SampleFormat LibavFormatToSampleFormat(AVSampleFormat format)
{
    for (const auto& entry : g_sample_format_info)
    {
        if (entry.libav_format == format)
            return entry.fmt;
    }
    return SampleFormat::kUnknown;
}

namespace {

thread_local char g_log_buffer[1024];

void av_log_callback(void *avcl, int level, const char *fmt, va_list arg)
{
    if (level > AV_LOG_DEBUG)
        return;

    LogType type;
    switch (level)
    {
    case AV_LOG_DEBUG:
        type = LOG_DEBUG;
        break;
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

void InitializePlatform()
{
    av_log_set_callback(av_log_callback);
    av_log_set_level(AV_LOG_INFO);
}

UTAU_NAMESPACE_END
