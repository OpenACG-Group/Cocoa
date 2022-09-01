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

#ifndef UNW_LOCAL_ONLY
#define UNW_LOCAL_ONLY
#endif // UNW_LOCAL_ONLY

#include <libunwind.h>
#include <dlfcn.h>
#include <cxxabi.h>

#include <string>
#include <memory>
#include <utility>
#include <vector>

#include "Core/Exception.h"

namespace {

std::string demangle_cpp_symbol(char const *sym)
{
    if (sym == nullptr)
        return "Unknown";

    char *demangled = abi::__cxa_demangle(sym, nullptr, nullptr, nullptr);
    if (demangled == nullptr)
        return sym;

    std::string ret(demangled);
    std::free(demangled);
    return ret;
}

} // namespace anonymous

namespace cocoa {

ScopeExitAutoInvoker::ScopeExitAutoInvoker(std::function<void()> func)
    : fFunction(std::move(func))
{
}

ScopeExitAutoInvoker::~ScopeExitAutoInvoker()
{
    fFunction();
}

void ScopeExitAutoInvoker::cancel()
{
    fFunction = []() -> void {};
}

RuntimeException::Builder::Builder(std::string who)
    : fWho(std::move(who))
{
}


RuntimeException::RuntimeException(std::string who, std::string what)
    : fWho(std::move(who))
    , fWhat(std::move(what))
{
    recordFrames();
}

RuntimeException::RuntimeException(const RuntimeException& other)
    : fFrames(other.fFrames),
      fWho(other.fWho),
      fWhat(other.fWhat)
{
}

const char *RuntimeException::who() const noexcept
{
    return fWho.c_str();
}

const char *RuntimeException::what() const noexcept
{
    return fWhat.c_str();
}

RuntimeException::FrameIterable RuntimeException::frames() const noexcept
{
    return FrameIterable(fFrames);
}

void RuntimeException::recordFrames()
{
    fFrames = std::make_shared<Frames>();

    unw_cursor_t    cursor;
    unw_context_t   context;
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    char proc_name[1024];
    unw_proc_info_t proc_info;
    unw_word_t offset;
    Dl_info dyn_linker_info;

    while (unw_step(&cursor) > 0)
    {
        Frame frame;

        /* Program counter register (%pc) */
        unw_word_t pc;
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        if (pc == 0)
            break;

        unw_get_proc_name(&cursor, proc_name, sizeof(proc_name), &offset);
        unw_get_proc_info(&cursor, &proc_info);
        frame.pc = reinterpret_cast<void *>(pc);
        frame.symbol = demangle_cpp_symbol(proc_name);
        frame.procAddress = reinterpret_cast<void *>(proc_info.start_ip);
        frame.offset = static_cast<off64_t>(offset);
        frame.file = "Unknown";

        /* Get symbol information by dynamic linker */
        if (dladdr(reinterpret_cast<const void*>(pc), &dyn_linker_info) != 0)
        {
            frame.file = dyn_linker_info.dli_fname;
        }

        fFrames->push_back(frame);
    }
}

} // namespace cocoa
