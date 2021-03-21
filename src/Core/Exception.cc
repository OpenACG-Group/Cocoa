#ifndef UNW_LOCAL_ONLY
#define UNW_LOCAL_ONLY
#endif // UNW_LOCAL_ONLY

#include <libunwind.h>
#include <dlfcn.h>
#include <cxxabi.h>

#include <string>
#include <memory>
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

BeforeLeaveScope::BeforeLeaveScope(std::function<void()> func)
    : fFunction(std::move(func))
{
}

BeforeLeaveScope::~BeforeLeaveScope()
{
    fFunction();
}

void BeforeLeaveScope::cancel()
{
    fFunction = []() -> void {};
}

RuntimeException::Builder::Builder(const std::string& who)
    : fWho(who)
{
}

RuntimeException::FrameIterable::FrameIterable(Frames::const_iterator begin,
                                               Frames::const_iterator end)
    : fBegin(std::move(begin)),
      fEnd(std::move(end))
{
}

RuntimeException::Frames::const_iterator RuntimeException::FrameIterable::begin() const noexcept
{
    return fBegin;
}

RuntimeException::Frames::const_iterator RuntimeException::FrameIterable::end() const noexcept
{
    return fEnd;
}

// ---------------------------------------------------------------------------------

RuntimeException::RuntimeException(const std::string& who, const std::string& what)
    : fWho(who), fWhat(what)
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
    return FrameIterable(fFrames->cbegin(), fFrames->cend());
}

void RuntimeException::recordFrames()
{
    fFrames = std::make_shared<Frames>();

    unw_cursor_t    cursor;
    unw_context_t   context;
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    for (int i = 0; i < 2 && unw_step(&cursor) > 0; i++)
        ;

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
        frame.offset = reinterpret_cast<uint64_t>(frame.pc) - reinterpret_cast<uint64_t>(frame.procAddress);

        fFrames->push_back(frame);
    }
}

} // namespace cocoa
