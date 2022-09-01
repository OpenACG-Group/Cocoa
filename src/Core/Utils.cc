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

#include <unistd.h>
#include <cxxabi.h>
#include <sys/sysinfo.h>

#include <fstream>
#include <typeinfo>
#include <atomic>

#include "Core/Utils.h"
#include "Core/Journal.h"
#include "Core/MeasuredTable.h"
#include "Core/Exception.h"
#include "Core/Filesystem.h"
namespace cocoa::utils {

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Core)

namespace {

// NOLINTNEXTLINE
std::vector<std::tuple<const char*, const std::type_info*>> cxa_symbol_simpl_typeinfo_ = {
        { "std::string", &typeid(std::string) }
};
std::vector<std::tuple<const char*, std::string>> cxa_symbol_simpl_types_tbl_;

// NOLINTNEXTLINE
void build_cxa_symbol_simpl_types_tbl()
{
    for (const auto &ti : cxa_symbol_simpl_typeinfo_)
    {
        char *sym = abi::__cxa_demangle(std::get<1>(ti)->name(), nullptr, nullptr, nullptr);
        if (sym)
        {
            cxa_symbol_simpl_types_tbl_.emplace_back(std::get<0>(ti), sym);
            std::free(sym);
        }
    }
}

std::atomic<const char*> symbol_simplify_pending_message_;

std::string try_simplify_cpp_symbol(std::string current)
{
    symbol_simplify_pending_message_.store(nullptr);

    if (cxa_symbol_simpl_types_tbl_.empty())
        build_cxa_symbol_simpl_types_tbl();

    for (const auto& tp : cxa_symbol_simpl_types_tbl_)
    {
        auto p = current.find(std::get<1>(tp));
        while (p != std::string::npos)
        {
            current.replace(p, std::get<1>(tp).size(), std::get<0>(tp));
            p = current.find(std::get<1>(tp));
        }
    }

    std::string stage_1_dump(current);

    auto p = current.find('<');
    while (p != std::string::npos)
    {
        int tpl_count = 1;
        auto pend = p + 1;
        for (; pend < current.size() && tpl_count > 0; pend++)
        {
            switch (current[pend])
            {
            case '<': tpl_count++; break;
            case '>': tpl_count--; break;
            }
        }
        if (tpl_count > 0)
        {
            symbol_simplify_pending_message_.store("Unexpected template syntax");
            return stage_1_dump;
        }

        auto old_size = current.size();
        current.replace(p, pend - p, std::string("<...>"));
        p = current.find('<', pend + (current.size() - old_size));
    }

    return current;
}

} // namespace anonymous

void SerializeException(const RuntimeException& except)
{
    QLOG(LOG_EXCEPTION, "%fg<hl>Exception: {}: {}%reset", except.who(), except.what());
    QLOG(LOG_EXCEPTION, "Stack traceback:");
    MeasuredTable table(1);
    int32_t idx = 1;
    for (const RuntimeException::Frame& f : except.frames())
    {
        std::ostringstream hdr, content;
        hdr << "  %fg<bl>#" << idx << "%reset %fg<cy>" << f.pc << "%reset %fg<gr>";
        if (f.symbol == "Unknown")
            hdr << " <...>";
        else
            hdr << " <+" << f.offset << ">";
        content << "%reset<>" << try_simplify_cpp_symbol(f.symbol) << " from " << f.file;

        table.append(hdr.str(), content.str());
        idx++;
    }
    table.flush([](const std::string& str) {
        QLOG(LOG_EXCEPTION, "{}", str);
    });

    if (symbol_simplify_pending_message_.load())
    {
        QLOG(LOG_ERROR, "(Internal.Cxa) Symbol simplification: {}",
             symbol_simplify_pending_message_.load());
    }
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

void PrintStackBacktrace(const std::string_view& title)
{
    RuntimeException except(__func__, std::string(title));
    SerializeException(except);
}

void SetThreadName(const char *name)
{
    pthread_t self = ::pthread_self();
    ::pthread_setname_np(self, name);
}

} // namespace cocoa::utils
