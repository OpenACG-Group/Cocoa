#include <sys/mman.h>

#include <csignal>
#include <iostream>

#include "Core/Exception.h"
#include "Core/Journal.h"
#include "Core/Utils.h"
#include "Core/PosixSignalCatcher.h"
#include "Core/EventDispatcher.h"

#if defined(__x86_64__)
extern "C" {
void posix_sigsegv_callee(uint64_t rip)
{
    using namespace cocoa;
    auto e = RuntimeException::Builder(__FUNCTION__)
                .append("Segmentation fault, %ip = ")
                .append(reinterpret_cast<void*>(rip))
                .make<RuntimeException>();

    utils::DumpRuntimeException(e, false, [](const std::string& str) -> void {
        std::cerr << str << std::endl;
    });
}
extern void posix_sigsegv_caller();
extern void posix_sigsegv_caller_save_rip(uint64_t rip);
}
#endif

namespace cocoa {

namespace
{

#ifdef __x86_64__

void posix_dump_proc_context(ucontext_t *ctx)
{
}

#else
void posix_dump_proc_context(ucontext_t *ctx)
{
    /* Unsupported platform */
}
#endif // __x86_64__

void posix_signal_handle_exit(int sig)
{
    PosixSignalCatcher::Ref().setCaughtSignal(sig);
    EventDispatcher::Ref().wakeup(PosixSignalCatcher::Instance());
}

void posix_signal_handle_sigsegv(siginfo_t *info, ucontext_t *ctx)
{
    posix_dump_proc_context(ctx);
#if defined(__x86_64__)
    posix_sigsegv_caller_save_rip(ctx->uc_mcontext.gregs[REG_RIP]);
    ctx->uc_mcontext.gregs[REG_RIP] = (uint64_t) posix_sigsegv_caller;
#elif defined(__i386__)
    std::cerr << "Segmentation fault" << std::endl;
    std::exit(EXIT_FAILURE);
#endif // __x86_64__
}

void posix_signal_handler(int sig, siginfo_t *info, void *ucontext)
{
    if (sig == SIGINT || sig == SIGTERM)
        posix_signal_handle_exit(sig);
    else if (sig == SIGSEGV)
        posix_signal_handle_sigsegv(info, reinterpret_cast<ucontext_t *>(ucontext));
    else
    {
        std::cerr << "SystemError: Unexpected UNIX signal" << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

} // namespace anonymous

PosixSignalCatcher::PosixSignalCatcher()
    : fCaughtSignal(-1),
      fOldAction{},
      fOldSigStack{},
      fAlternateStack(nullptr)
{
    fAlternateStack = mmap(nullptr,
                           SIGSTKSZ,
                           PROT_WRITE | PROT_READ,
                           MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN,
                           -1, 0);
    log_write(LOG_DEBUG) << "Unix signals will be handled on stack "
                         << fAlternateStack << log_endl;
    stack_t st{
        .ss_sp = fAlternateStack,
        .ss_flags = 0,
        .ss_size = SIGSTKSZ
    };
    sigaltstack(&st, &fOldSigStack);

    struct sigaction act{};
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_ONSTACK | SA_SIGINFO;
    act.sa_sigaction = posix_signal_handler;

    sigaction(SIGINT, &act, &fOldAction);
    sigaction(SIGSEGV, &act, nullptr);
    sigaction(SIGTERM, &act, nullptr);
}

PosixSignalCatcher::~PosixSignalCatcher()
{
    sigaction(SIGINT, &fOldAction, nullptr);
    sigaction(SIGSEGV, &fOldAction, nullptr);
    sigaction(SIGTERM, &fOldAction, nullptr);

    sigaltstack(&fOldSigStack, nullptr);
    if (fAlternateStack)
        munmap(fAlternateStack, SIGSTKSZ);
}

void PosixSignalCatcher::setCaughtSignal(int signum)
{
    fCaughtSignal = signum;
}

void PosixSignalCatcher::processEvent()
{
    if (fCaughtSignal == SIGINT ||
        fCaughtSignal == SIGTERM)
    {
        log_write(LOG_INFO) << "Terminated by signal" << log_endl;
        EventDispatcher::Ref().dispose();
    }
    else
    {
        log_write(LOG_ERROR) << "Unknown UNIX signal: " << fCaughtSignal << log_endl;
    }
}

} // namespace cocoa
