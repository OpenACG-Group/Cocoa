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

ScopeEpilogue::ScopeEpilogue(std::function<void()> func)
    : fFunction(std::move(func))
{
}

ScopeEpilogue::~ScopeEpilogue()
{
    fFunction();
}

void ScopeEpilogue::abolish()
{
    fFunction = []() -> void {};
}

RuntimeException::Builder::Builder(std::string who)
    : fWho(std::move(who))
{
}

// ---------------------------------------------------------------------------------

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

    while (unw_step(&cursor) > 0)
    {
        Frame frame;

        /* Program counter register (%pc) */
        unw_word_t pc;
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        if (pc == 0)
            break;

        frame.pc = reinterpret_cast<void*>(pc);
        frame.file = "<Unknown>";
        frame.symbol = "<Unknown>";
        frame.procAddress = nullptr;

        /* Get symbol information by dynamic linker */
        Dl_info dynLinkerInfo;
        if (dladdr(reinterpret_cast<const void*>(pc), &dynLinkerInfo) == 0)
        {
            fFrames->push_back(frame);
            continue;
        }

        if (dynLinkerInfo.dli_saddr == nullptr)
        {
            unw_word_t sp;
            unw_get_reg(&cursor, UNW_REG_SP, &sp);
        }

        frame.file = dynLinkerInfo.dli_fname;
        frame.symbol = demangle_cpp_symbol(dynLinkerInfo.dli_sname);
        frame.procAddress = dynLinkerInfo.dli_saddr;
        frame.offset = static_cast<off64_t>(reinterpret_cast<uint64_t>(frame.pc)
                        - reinterpret_cast<uint64_t>(frame.procAddress));
        fFrames->push_back(frame);
    }
}

} // namespace cocoa
