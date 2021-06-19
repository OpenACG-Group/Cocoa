#ifndef __LOG_H__
#define __LOG_H__

#include <string>
#include <sstream>
#include <ostream>
#include <chrono>
#include <cstdint>
#include <mutex>

#include "Core/Project.h"
#include "Core/UniquePersistent.h"

namespace cocoa
{

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

struct __endl_struct {};
using endl_t = struct __endl_struct *;

#define log_endl        static_cast<cocoa::endl_t>(nullptr)
#define log_write(t)    cocoa::Journal::Instance()->stream(t)

class Journal;

class StreamHolder
{
public:
    explicit StreamHolder(Journal *logObject);
    ~StreamHolder();

    template<typename T>
    StreamHolder& operator<<(T&& val)
    {
        mBuffer << std::forward<T>(val);
        return *this;
    }

    template<>
    StreamHolder& operator<<(endl_t&& val)
    {
        mBuffer << '\n';
        commitBuffer();
        mBuffer.str("");
        return *this;
    }

private:
    void commitBuffer();

private:
    std::ostringstream  mBuffer;
    Journal            *mLogger;
};

class Journal : public UniquePersistent<Journal>
{
public:
    Journal(int fd, int filter, bool color);
    Journal(char const *file, int filter, bool color);
    ~Journal();

    void write(char const *str);
    StreamHolder& stream(int type);
    bool textShader() const;
    void setTextShader(bool enable);

private:
    void welcome();
    void redirectToFile(char const *path);

private:
    StreamHolder    *mStream;
    int              mFilter;
    int              mCurType;
    std::mutex       mOutMutex;
    bool             mColor;
    int              mFd;
    std::chrono::steady_clock::time_point mStartTime;
};

} // namespace cocoa

#endif // __LOG_H__
