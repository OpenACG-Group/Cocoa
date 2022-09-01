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

#ifndef COCOA_CORE_JOURNAL_H
#define COCOA_CORE_JOURNAL_H

#ifndef COCOA_JOURNAL_DISABLED
#include <mutex>
#include <string>
#include <chrono>
#endif /* COCOA_JOURNAL_DISABLED */

#include "fmt/format.h"

#ifndef COCOA_JOURNAL_DISABLED
#include "Core/UniquePersistent.h"
#endif /* COCOA_JOURNAL_DISABLED */

namespace cocoa {

#define COCOA_MODULE_NAME(name)  "Cocoa." #name

enum LogType
{
    LOG_DEBUG       = 0x0001,
    LOG_INFO        = 0x0002,
    LOG_WARNING     = 0x0004,
    LOG_ERROR       = 0x0008,
    LOG_EXCEPTION   = 0x0010
};

#ifndef COCOA_JOURNAL_DISABLED

enum LogLevel
{
    LOG_LEVEL_DEBUG     = LOG_DEBUG | LOG_INFO | LOG_WARNING | LOG_ERROR | LOG_EXCEPTION,
    LOG_LEVEL_NORMAL    = LOG_INFO | LOG_WARNING | LOG_ERROR | LOG_EXCEPTION,
    LOG_LEVEL_QUIET     = LOG_WARNING | LOG_ERROR | LOG_EXCEPTION,
    LOG_LEVEL_SILENT    = LOG_ERROR | LOG_EXCEPTION,
    LOG_LEVEL_DISABLED  = 0x0000
};

class Journal : public UniquePersistent<Journal>
{
public:
    enum class OutputDevice
    {
        kStandardError,
        kStandardOut,
        kFile
    };

    Journal(LogLevel level, OutputDevice output, bool enableColor, char const *file = nullptr);
    ~Journal();

    template<typename...ArgsT>
    void operator()(LogType type, fmt::format_string<ArgsT...> format, ArgsT&&...args)
    {
        if (!this->filter(type))
            return;
        this->commit(type, fmt::format(format, std::forward<ArgsT>(args)...));
    }

private:
    void commit(LogType type, const std::string& str);
    bool filter(LogType type);

    bool                                    fEnableColor;
    std::mutex                              fWriteMutex;
    LogLevel                                fLevel;
    int                                     fOutputFd;
    std::chrono::steady_clock::time_point   fStartTime;
};

#define QLOG(level, fmt, ...)                                                       \
    do {                                                                            \
        Journal::Ref()(level, "%fg<bl><{}>%reset " fmt,                             \
                       THIS_FILE_MODULE __VA_OPT__(,) __VA_ARGS__);                 \
    } while (false)

#else  /* COCOA_JOURNAL_DISABLED is defined */

#define QLOG(level, fmt, ...) fmt::print("[" THIS_FILE_MODULE "]" fmt __VA_OPT__(,) __VA_ARGS__);

#endif /* !COCOA_JOURNAL_DISABLED */

} // namespace cocoa
#endif //COCOA_CORE_JOURNAL_H
