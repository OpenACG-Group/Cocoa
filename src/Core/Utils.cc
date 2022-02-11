#include <unistd.h>
#include <cxxabi.h>
#include <sys/sysinfo.h>

#include <fstream>

#include "Core/Utils.h"
#include "Core/Journal.h"
#include "Core/MeasuredTable.h"
#include "Core/Exception.h"
#include "Core/Filesystem.h"
namespace cocoa::utils {

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Core)

void SerializeException(const RuntimeException& except)
{
    QLOG(LOG_EXCEPTION, "%fg<hl>Exception: {}: {}%reset", except.who(), except.what());
    QLOG(LOG_EXCEPTION, "Traceback (most recent call last):");
    MeasuredTable table(1);
    int32_t idx = 1;
    for (const RuntimeException::Frame& f : except.frames())
    {
        std::ostringstream hdr, content;
        hdr << "  #" << idx << " " << f.pc;
        if (f.symbol == "Unknown")
            hdr << " <...>";
        else
            hdr << " <+" << f.offset << ">";
        content << f.symbol << " from " << f.file;

        table.append(hdr.str(), content.str());
        idx++;
    }
    table.flush([](const std::string& str) {
        QLOG(LOG_EXCEPTION, "{}", str);
    });
}

std::string GetAbsoluteDirectory(const std::string& dir)
{
    return vfs::Realpath(dir);
}

std::string GetExecutablePath()
{
#ifdef __linux__
    return vfs::ReadLink("/proc/self/exe");
#else
#error Unsupported platform
#endif
}

size_t GetMemPageSize()
{
    return getpagesize();
}

std::string GetCpuModel()
{
#ifdef __linux__
    std::ifstream fs("/proc/cpuinfo");
    if (!fs.is_open())
        return "<Unknown>";

    std::string linebuf;
    linebuf.resize(512);
    while (fs.getline(linebuf.data(), static_cast<std::streamsize>(linebuf.size())))
    {
        if (StrStartsWith(linebuf, "model name"))
        {
            auto pos = linebuf.find_first_of(':');
            if (pos == std::string::npos || pos + 2 >= linebuf.length())
                return "<Unknown>";

            std::string copy = linebuf.substr(pos + 2);
            copy.resize(std::strlen(copy.data()));
            return copy;
        }
    }
    return "<Unknown>";
#else
#error Unsupported platform
#endif // __linux__
}

size_t GetMemTotalSize()
{
#ifdef __linux__
    struct sysinfo info{};
    if (sysinfo(&info) < 0)
        return 0;
    return info.totalram;
#else
#error Unsupported platform
#endif
}

std::vector<std::string_view> SplitString(const std::string& str, std::string::value_type delimiter)
{
    std::vector<std::string_view> result;
    size_t p = 0;
    int64_t last_p = -1;
    while ((p = str.find(delimiter, p + 1)) != std::string::npos)
    {
        std::string_view view(str);
        view.remove_prefix(last_p + 1);
        view.remove_suffix(str.size() - p);
        result.emplace_back(view);
        last_p = static_cast<int64_t>(p);
    }
    std::string_view view(str);
    view.remove_prefix(last_p + 1);
    result.emplace_back(view);

    return result;
}

} // namespace cocoa::utils
