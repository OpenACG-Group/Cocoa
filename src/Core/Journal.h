#ifndef COCOA_JOURNAL_H
#define COCOA_JOURNAL_H

#include <mutex>
#include <string>
#include <chrono>

#include "fmt/format.h"
#include "fmt/ostream.h"
#include "Core/UniquePersistent.h"
namespace cocoa {

enum LogType
{
    LOG_DEBUG       = 0x0001,
    LOG_INFO        = 0x0002,
    LOG_WARNING     = 0x0004,
    LOG_ERROR       = 0x0008,
    LOG_EXCEPTION   = 0x0010
};

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

} // namespace cocoa

#endif //COCOA_JOURNAL_H
