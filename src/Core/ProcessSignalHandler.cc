#include <initializer_list>
#include <csignal>

#define UNW_LOCAL_ONLY
#include "libunwind.h"

#include "fmt/format.h"
#include "uv.h"

#include "Core/Exception.h"
#include "Core/Errors.h"
#include "Core/ProcessSignalHandler.h"
#include "Core/EventLoop.h"
namespace cocoa {

namespace {

void primary_signal_handler_entrypoint(int signum, ::siginfo_t *siginfo, void *data)
{
    auto *ucontext = reinterpret_cast<::ucontext_t*>(data);
    fmt::print(stderr, "\033[31;1m[interrupt] Signal {} is caught by primary signal handler\n", signum);

    // NOLINTNEXTLINE
    if constexpr(std::is_same<unw_context_t, ucontext_t>::value)
    {
        fmt::print(stderr, "[interrupt] Stack backtrace:\n");

        unw_cursor_t cursor;
        unw_init_local(&cursor, ucontext);
        char proc_name[1024];
        for (int d = 0; unw_step(&cursor) > 0; d++)
        {
            unw_word_t poff;
            unw_get_proc_name(&cursor, proc_name, sizeof(proc_name), &poff);
            fmt::print(stderr, "[interrupt]  #{} {} <+{}>\n", d, proc_name, poff);
        }
    }

    fmt::print(stderr, "\033[0m\n");
    CHECK_FAILED("Fatal signal interrupted");
}

} // namespace anonymous

void InstallPrimarySignalHandler()
{
    struct ::sigaction sa{};
    ::sigemptyset(&sa.sa_mask);

    sa.sa_sigaction = primary_signal_handler_entrypoint;
    sa.sa_flags = SA_SIGINFO;

    /* Catch and process fault signals */
    for (const auto& sig : {std::make_tuple(SIGSEGV, "SIGSEGV"), std::make_tuple(SIGILL, "SIGILL"),
                            std::make_tuple(SIGFPE, "SIGFPE"), std::make_tuple(SIGBUS, "SIGBUS")})
    {
        if (::sigaction(std::get<0>(sig), &sa, nullptr) < 0)
        {
            fmt::print(stderr, "Failed to register primary signal handler for {}\n", std::get<1>(sig));
            std::exit(EXIT_ERROR_BIT | EXIT_FATAL_BIT);
        }
    }
}

void InstallSecondarySignalHandler()
{
    // TODO(important): implement this.
}

void BeforeEventLoopEntrypointHook()
{
}

} // namespace cocoa
