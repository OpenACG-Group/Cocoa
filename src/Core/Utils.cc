#include <unistd.h>

#include "Core/Utils.h"
#include "Core/Journal.h"
#include "Core/MeasuredTable.h"
#include "Core/Exception.h"
namespace cocoa::utils {

void ChangeWorkDirectory(const std::string& dir)
{
    int ret = ::chdir(dir.c_str());
    if (ret < 0)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Failed to change working directory to \"")
                .append(dir)
                .append("\": ")
                .append(strerror(errno))
                .make<RuntimeException>();
    }
}

void DumpRuntimeException(const RuntimeException& except)
{
    Journal::Ref()(LOG_EXCEPTION, "{}:", except.what());
    Journal::Ref()(LOG_EXCEPTION, "Traceback (most recent call last):");

    MeasuredTable table(1);
    for (const RuntimeException::Frame& f : except.frames())
    {
        std::ostringstream hdr, content;
        if (f.symbol == "Unknown")
            hdr << "  " << f.pc << " <...>";
        else
            hdr << "  " << f.pc << " <+" << f.offset << ">";
        content << f.symbol << " from " << f.file;

        table.append(hdr.str(), content.str());
    }

    table.flush([](const std::string& str) -> void {
        Journal::Ref()(LOG_EXCEPTION, "{}", str);
    });
}

std::string GetAbsoluteDirectory(const std::string& dir)
{
    char buf[PATH_MAX];
    realpath(dir.c_str(), buf);

    return buf;
}

std::string GetExecutablePath()
{
    typedef std::vector<char> char_vector;
    typedef std::vector<char>::size_type size_type;
    char_vector buf(1024, 0);
    size_type size = buf.size();
    bool havePath = false;
    bool shouldContinue = true;
    do
    {
        ssize_t result = readlink("/proc/self/exe", &buf[0], size);
        if (result < 0)
            shouldContinue = false;
        else if (static_cast<size_type>(result) < size)
        {
            havePath = true;
            shouldContinue = false;
            size = result;
        }
        else
        {
            size *= 2;
            buf.resize(size);
            std::fill(std::begin(buf), std::end(buf), 0);
        }
    } while (shouldContinue);
    if (!havePath)
        throw std::runtime_error("Failed to get executable path");
    return std::string(&buf[0], size);
}

} // namespace cocoa::utils
