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

void DumpRuntimeException(const RuntimeException& except)
{
    LOGF(LOG_EXCEPTION, "{}:", except.what())
    LOGW(LOG_EXCEPTION, "Traceback (most recent call last):")

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
        LOGF(LOG_EXCEPTION, "{}", str)
    });
}

namespace {

void dumpPropertiesObjectNode(const std::shared_ptr<PropertyObjectNode>& node, const std::string& name, int depth);
void dumpPropertiesDataNode(const std::shared_ptr<PropertyDataNode>& node, const std::string& name, int depth);
void dumpPropertiesArrayNode(const std::shared_ptr<PropertyArrayNode>& node, const std::string& name, int depth);

void dumpPropertiesNode(const std::shared_ptr<PropertyNode>& node, const std::string& name, int depth)
{
    switch (node->kind())
    {
    case PropertyNode::Kind::kObject:
        dumpPropertiesObjectNode(prop::Cast<PropertyObjectNode>(node), name, depth);
        break;
    case PropertyNode::Kind::kArray:
        dumpPropertiesArrayNode(prop::Cast<PropertyArrayNode>(node), name, depth);
        break;
    case PropertyNode::Kind::kData:
        dumpPropertiesDataNode(prop::Cast<PropertyDataNode>(node), name, depth);
        break;
    }
}

std::string genOutSpacing(int depth)
{
    std::string ret;
    for (int i = 0; i < depth * 2; i++)
        ret.push_back(' ');
    return ret;
}

std::ostream& operator<<(std::ostream& os, PropertyNode::Protection prot)
{
    switch (prot)
    {
    case PropertyNode::Protection::kPublic:
        os << "public";
        break;
    case PropertyNode::Protection::kProtected:
        os << "protected";
        break;
    case PropertyNode::Protection::kPrivate:
        os << "private";
        break;
    }
    return os;
}

std::string genOutHeader(PropertyNode::Protection prot, const std::string& name, int depth)
{
    std::ostringstream ret;
    for (int i = 0; i < depth * 2; i++)
        ret << ' ';
    ret << "%fg<re>" << prot << "%reset %fg<ye>" << name << "%reset<>: ";
    return ret.str();
}

void dumpPropertiesDataNode(const std::shared_ptr<PropertyDataNode>& node, const std::string& name, int depth)
{
    auto s = genOutHeader(node->protection(), name, depth);
#define T_(t)  (node->type() == typeid(t))
    if (T_(int8_t) || T_(int16_t) || T_(int32_t) || T_(int64_t))
    {
        LOGF(LOG_INFO, "{}%fg<ma>{}%reset", s, node->extract<int64_t>())
    }
    else if (T_(uint8_t))
    {
        LOGF(LOG_INFO, "{}%fg<ma>0x{:02x}%reset", s, node->extract<uint8_t>())
    }
    else if (T_(uint16_t) || T_(uint32_t) || T_(uint64_t))
    {
        LOGF(LOG_INFO, "{}%fg<ma>{}U%reset", s, node->extract<uint64_t>())
    }
    else if (T_(float))
    {
        LOGF(LOG_INFO, "{}%fg<ma>{}f%reset", s, node->extract<float>())
    }
    else if (T_(double))
    {
        LOGF(LOG_INFO, "{}%fg<ma>{}%reset", s, node->extract<double>())
    }
    else if (T_(long double))
    {
        LOGF(LOG_INFO, "{}%fg<ma>{}%reset", s, node->extract<long double>())
    }
    else if (T_(const char*))
    {
        /* Disable the syntax log while printing a string to avoid replacements in the string. */
        LOGF(LOG_INFO, "{}%fg<gr>%disable\"{}\"%enable%reset", s, node->extract<const char*>())
    }
    else if (T_(std::string))
    {
        LOGF(LOG_INFO, "{}%fg<gr>%disable\"{}\"%enable%reset", s, node->extract<std::string&>())
    }
    else
    {
        char *demangledType = abi::__cxa_demangle(node->type().name(),
                                                  nullptr, nullptr, nullptr);
        if (!demangledType)
        {
            LOGF(LOG_INFO, "{}%fg<re><Corrupted typeinfo>%reset", s)
        }
        else
        {
            LOGF(LOG_INFO, "{}%fg<re><%fg<bl>(ptr)%fg<re> {}>%reset", s, demangledType)
            std::free(demangledType);
        }
    }
#undef T_
}

void dumpPropertiesObjectNode(const std::shared_ptr<PropertyObjectNode>& node, const std::string& name, int depth)
{
    auto s = genOutHeader(node->protection(), name, depth);
    LOGF(LOG_INFO, "{}{{", s)

    for (const auto& child : *node)
        dumpPropertiesNode(child.second, child.first, depth + 1);

    LOGF(LOG_INFO, "{}}}", genOutSpacing(depth))
}

void dumpPropertiesArrayNode(const std::shared_ptr<PropertyArrayNode>& node, const std::string& name, int depth)
{
    auto s = genOutHeader(node->protection(), name, depth);
    LOGF(LOG_INFO, "{}[", s)

    int32_t idx = 0;
    for (const auto& subscript : *node)
        dumpPropertiesNode(subscript, fmt::format("[{}]", idx++), depth + 1);

    LOGF(LOG_INFO, "{}]", genOutSpacing(depth))
}

} // namespace anonymous

void DumpProperties(const std::shared_ptr<PropertyNode>& root)
{
    dumpPropertiesNode(root, "<root>", 0);
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
        if (linebuf.starts_with("model name"))
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
    struct sysinfo info;
    if (sysinfo(&info) < 0)
        return 0;
    return info.totalram;
#else
#error Unsupported platform
#endif
}

} // namespace cocoa::utils
