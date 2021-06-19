#include <iostream>
#include <cmath>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "Core/Journal.h"
#include "Core/Exception.h"
#include "Core/Configurator.h"

namespace {

enum Identifier
{
    /* catch_block tokens */
    MAYBE_TIMESTAMP_OR_LEVEL_OR_DECLARATIVE,
    MAYBE_MODULE_NAME,
    MAYBE_NUMBER,
    MAYBE_CONTENT_OR_DEFINITION,
    /* Final tokens */
    TIMESTAMP,
    LEVEL_TRACE,
    LEVEL_DEBUG,
    LEVEL_INFO,
    LEVEL_WARN,
    LEVEL_ERROR,
    LEVEL_FATAL,
    LEVEL_OFF,
    DECLARATIVE,
    DEFINITION,
    MODULE_NAME,
    NUMBER,
    STRING,
    SPACE,
    OPERATOR,
    CONTENT
};

enum Colors
{
    NONE = 0,
    GREEN = 0x001,
    RED = 0x002,
    YELLOW = 0x004,
    BLUE = 0x008,
    PURPLE = 0x010,
    DEEP_GREEN = 0x020,
    WHITE = 0x040,
    GRAY = 0x080,
    HIGHLIGHT = 0x100,
    DISABLE = 0x200
};

typedef struct
{
    int color;
    char const *attribute;
} ColorAttributeMap;

typedef struct
{
    Identifier ident;
    int color;
} IdentColorMap;

typedef struct
{
    char const *begin;
    char const *end;
    Identifier ident;
} catch_block;

typedef struct
{
    Identifier level;
    char const *str;
} LevelStringMap;

const ColorAttributeMap text_shader_camap[] = {
        {GREEN,      "\033[32m"},
        {RED,        "\033[31m"},
        {YELLOW,     "\033[33m"},
        {BLUE,       "\033[34m"},
        {PURPLE,     "\033[35m"},
        {DEEP_GREEN, "\033[36m"},
        {WHITE,      "\033[37m"},
        {GRAY,       "\033[38m"},
        {HIGHLIGHT,  "\033[1m"},
        {DISABLE,    "\033[0m"},
        {NONE,       ""}
};

const IdentColorMap text_shader_icmap[] = {
        {TIMESTAMP,   PURPLE},
        {LEVEL_TRACE, GRAY | HIGHLIGHT},
        {LEVEL_DEBUG, PURPLE},
        {LEVEL_INFO,  GREEN},
        {LEVEL_WARN,  YELLOW},
        {LEVEL_ERROR, RED},
        {LEVEL_FATAL, RED | HIGHLIGHT},
        {LEVEL_OFF,   WHITE | HIGHLIGHT},
        {MODULE_NAME, WHITE | HIGHLIGHT},
        {NUMBER,      DEEP_GREEN},
        {STRING,      YELLOW},
        {SPACE,       NONE},
        {CONTENT,     NONE},
        {OPERATOR,    RED},
        {DECLARATIVE, BLUE | HIGHLIGHT},
        {DEFINITION,  DEEP_GREEN}
};

const LevelStringMap text_shader_lsmap[] = {
        {LEVEL_TRACE, "trace"},
        {LEVEL_DEBUG, "debug"},
        {LEVEL_INFO,  "info"},
        {LEVEL_WARN,  "warn"},
        {LEVEL_ERROR, "error"},
        {LEVEL_FATAL, "fatal"},
        {LEVEL_OFF,   "off"}
};

bool text_shader_matches_catch_block(char const *str, catch_block *catch_block)
{
    bool expects = false;
    bool expectExclude = false;
    char expect;

    if (*str == '\0')
        return false;

    while (*str != '\0')
    {
        if (expects)
        {
            if (expect != *str)
                goto next;
            catch_block->end = str;
            if (expectExclude)
                catch_block->end--;
            break;
        }

        catch_block->begin = str;
        if (*str == '[')
        {
            catch_block->ident = MAYBE_TIMESTAMP_OR_LEVEL_OR_DECLARATIVE;
            expects = true;
            expect = ']';
        } else if (*str == '<')
        {
            catch_block->ident = MAYBE_MODULE_NAME;
            expects = true;
            expect = '>';
        } else if (*str >= '0' && *str <= '9')
        {
            catch_block->ident = MAYBE_NUMBER;
            expects = true;
            expectExclude = true;
            expect = ' ';
        } else if (*str == '\"' || *str == '\'')
        {
            catch_block->ident = STRING;
            expects = true;
            expect = *str;
        } else if (*str == ' ')
        {
            catch_block->begin = str;
            catch_block->end = str;
            catch_block->ident = SPACE;
            break;
        } else if (*str == '=' || *str == '+' || *str == '-'
                   || *str == '*' || *str == '/' || *str == '>'
                   || *str == '<')
        {
            catch_block->end = str;
            catch_block->ident = OPERATOR;
            break;
        } else if (*str == '\033')
        {
            catch_block->ident = CONTENT;
            catch_block->end = str + std::strlen(str) - 1;
            break;
        } else
        {
            catch_block->ident = MAYBE_CONTENT_OR_DEFINITION;
            expects = true;
            expect = ' ';
            expectExclude = true;
        }

        next:
        str++;
    }

    if (expects && expect == ' ' && *str == '\0')
        catch_block->end = str - 1;
    return true;
}

inline bool text_shader_is_number(char ch)
{ return (ch >= '0' && ch <= '9'); }

inline bool text_shader_is_upper(char ch)
{ return (ch >= 'A' && ch <= 'Z'); }

inline bool text_shader_is_lower(char ch)
{ return (ch >= 'a' && ch <= 'z'); }

inline char text_shader_to_lower(char ch)
{
    if (text_shader_is_lower(ch))
        return ch;
    else
    {
        if (text_shader_is_upper(ch))
            return char('a' + (ch - 'A'));
    }
    return ch;
}

bool text_shader_case_insensitive_compare_equal(const char *begin, const char *end, char const *str)
{
    if (begin > end)
        return false;
    if (static_cast<size_t>(end - begin + 1) != strlen(str))
        return false;

    char const *p0 = begin, *p1 = str;
    while (p0 <= end && *p1 != '\0')
    {
        char c0 = text_shader_to_lower(*p0);
        char c1 = text_shader_to_lower(*p1);
        if (c0 != c1)
            return false;
        p0++;
        p1++;
    }
    return true;
}

bool text_shader_is_integer(char const *begin, char const *end)
{
    if (begin > end)
        return false;
    while (begin <= end)
    {
        if (!text_shader_is_number(*begin))
            return false;
        begin++;
    }
    return true;
}

bool text_shader_is_float(char const *begin, char const *end)
{
    if (begin > end)
        return false;

    bool point = false;
    while (begin <= end)
    {
        if (*begin == '.')
        {
            if (point || begin == end)
                return false;
            point = true;
        } else if (!text_shader_is_number(*begin))
            return false;
        begin++;
    }
    return true;
}

bool text_shader_check_number(catch_block *cb)
{
    if (text_shader_is_integer(cb->begin, cb->end)
        || text_shader_is_float(cb->begin, cb->end))
    {
        cb->ident = NUMBER;
        return true;
    }
    return false;
}

/* There's no regex engine, so we must handle it manually... */
bool text_shader_check_timestamp(catch_block *cb)
{
    const char *ptr = cb->begin + 1;
    while (*ptr++ == ' ');

    if (text_shader_is_float(ptr, cb->end - 1))
    {
        cb->ident = TIMESTAMP;
        return true;
    }
    return false;
}

#define ARRAY_SIZE(a)   (sizeof(a) / sizeof(a[0]))

bool text_shader_check_level(catch_block *cb)
{
    for (auto i : text_shader_lsmap)
    {
        if (text_shader_case_insensitive_compare_equal(cb->begin + 1, cb->end - 1,
                                                     i.str))
        {
            cb->ident = i.level;
            return true;
        }
    }
    return false;
}

bool text_shader_is_llegal_identifier(char const *begin, char const *end)
{
    if (begin > end)
        return 0;

    bool allowNumber = 0;
    while (begin <= end)
    {
        if ((text_shader_is_number(*begin) && !allowNumber) ||
            (!text_shader_is_lower(*begin) && !text_shader_is_upper(*begin) &&
             !text_shader_is_number(*begin) && *begin != '_'))
            return false;
        allowNumber = true;
        begin++;
    }
    return true;
}

bool text_shader_check_modulename(catch_block *cb)
{
    /* A module name is like: org.sora.xxx */
    char const *p = cb->begin;
    char const *pIdBegin = cb->begin + 1, *pIdEnd;
    while (p <= cb->end)
    {
        if (*p == '.' || p == cb->end)
        {
            pIdEnd = p - 1;
            if (!text_shader_is_llegal_identifier(pIdBegin, pIdEnd))
                return false;
            pIdBegin = p + 1;
        }
        p++;
    }
    cb->ident = MODULE_NAME;
    return true;
}

bool text_shader_check_definition(catch_block *cb)
{
    if (!text_shader_is_llegal_identifier(cb->begin, cb->end))
        return false;

    char const *p = cb->begin;
    while (p <= cb->end)
    {
        if (!text_shader_is_upper(*p) && *p != '_' && !text_shader_is_number(*p))
            return false;
        p++;
    }
    cb->ident = DEFINITION;
    return true;
}

void text_shader_catch_block_tochecked(catch_block *cb)
{
    switch (cb->ident)
    {
    case MAYBE_TIMESTAMP_OR_LEVEL_OR_DECLARATIVE:
        if (!text_shader_check_timestamp(cb) && !text_shader_check_level(cb))
            cb->ident = DECLARATIVE;
        break;
    case MAYBE_NUMBER:
        if (!text_shader_check_number(cb))
            cb->ident = CONTENT;
        break;
    case MAYBE_MODULE_NAME:
        if (!text_shader_check_modulename(cb))
            cb->ident = CONTENT;
        break;
    case MAYBE_CONTENT_OR_DEFINITION:
        if (!text_shader_check_definition(cb))
            cb->ident = CONTENT;
        break;
    default:
        break;
    }
}

void text_shader_apply(int fd, catch_block *cb)
{
    int color = NONE;
    for (auto i : text_shader_icmap)
    {
        if (i.ident == cb->ident)
        {
            color = i.color;
            break;
        }
    }

    char const *disable_attribute;
    for (auto i : text_shader_camap)
    {
        if (color & i.color)
        {
            ::write(fd, i.attribute, std::strlen(i.attribute));
        }
        if (i.color == DISABLE)
            disable_attribute = i.attribute;
    }

    size_t siz = cb->end - cb->begin + 1;
    char buffer[siz + 1];
    std::memcpy(buffer, cb->begin, siz);
    buffer[siz] = '\0';

    ::write(fd, buffer, std::strlen(buffer));
    ::write(fd, disable_attribute, std::strlen(disable_attribute));
}

void text_shader_commit(int fd, char const *str)
{
    catch_block catch_block;
    while (text_shader_matches_catch_block(str, &catch_block))
    {
        text_shader_catch_block_tochecked(&catch_block);
        text_shader_apply(fd, &catch_block);
        str = catch_block.end + 1;
    }
}

} // namespace anonymous

namespace cocoa {

StreamHolder::StreamHolder(Journal *obj)
    : mLogger(obj)
{
}

StreamHolder::~StreamHolder()
{
}

void StreamHolder::commitBuffer()
{
    std::string str = mBuffer.str();
    mLogger->write(str.c_str());
}

static const char *infoPrompt[] = {
    "[Debug]",
    "[Info]",
    "[Warn]",
    "[Error]",
    "[Fatal]"
};

Journal::Journal(int fd, int filter, bool color)
    : mStream(new StreamHolder(this)),
      mColor(color),
      mStartTime(std::chrono::steady_clock::now())
{
    mFd = fd;
    mFilter = filter;

    welcome();
}

Journal::Journal(char const *file, int filter, bool color)
    : mStream(new StreamHolder(this)),
      mColor(color),
      mStartTime(std::chrono::steady_clock::now())
{
    BeforeLeaveScope beforeLeaveScope([&]() -> void { delete mStream; });
    redirectToFile(file);
    beforeLeaveScope.cancel();

    mFilter = filter;

    welcome();
}

Journal::~Journal()
{
    mOutMutex.lock();
    delete mStream;
    mOutMutex.unlock();
}

void Journal::welcome()
{
    stream(LOG_INFO) << "Cocoa 2D Rendering Engine version " << COCOA_VERSION << log_endl;
    stream(LOG_INFO) << "This software is under " << COCOA_LICENSE << log_endl;
    stream(LOG_INFO) << log_endl;
}

StreamHolder& Journal::stream(int type)
{
    mOutMutex.lock();
    mCurType = type;
    return *mStream;
}

void Journal::write(char const *str)
{
    if (!(mCurType & mFilter))
    {
        mOutMutex.unlock();
        return ;
    }
    
    int idx = std::log2(mCurType);
    const char *header =  infoPrompt[idx];
    
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - mStartTime);
    double dt = static_cast<double>(duration.count()) * std::chrono::microseconds::period::num
        / std::chrono::microseconds::period::den;

    std::ostringstream ss;
    ss.setf(std::ios_base::right | std::ios_base::fixed);
    ss.precision(6);
    ss << '[' << std::setw(12) << dt << "] ";
    
    ss << header << " " << str;
    std::string res = ss.str();
    // ::write(mFd, res.c_str(), res.size());
    if (mColor)
        text_shader_commit(mFd, res.c_str());
    else
        ::write(mFd, res.c_str(), res.length());

    mOutMutex.unlock();
}

void Journal::redirectToFile(const char *path)
{
    int fd = ::open(path, O_RDWR);
    if (fd >= 0)
    {
        ::close(fd);
        char *p = new char[std::strlen(path) + 5];
        sprintf(p, "%s.old", path);
        ::rename(path, p);
        delete[] p;
    }

    fd = ::open(path, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);
    if (fd < 0)
        throw std::runtime_error("Could not open log file");

    mFd = fd;
}

bool Journal::textShader() const
{
    return mColor;
}

void Journal::setTextShader(bool enable)
{
    mColor = enable;
}

} // namespace cocoa
