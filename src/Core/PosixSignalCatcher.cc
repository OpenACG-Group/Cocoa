#include <sys/mman.h>

#include <csignal>
#include <iostream>

#include "Core/Exception.h"
#include "Core/Journal.h"
#include "Core/Utils.h"
#include "Core/PosixSignalCatcher.h"

#if defined(__x86_64__)
extern "C" {
void posix_sigsegv_callee(uint64_t rip)
{
    using namespace cocoa;
    auto e = RuntimeException::Builder(__FUNCTION__)
                .append("Exception on segmentation fault, %ip = ")
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

#define DUMP_REG(reg)   std::cerr << "  %" #reg ": " \
                                  << reinterpret_cast<void*>(ctx->uc_mcontext.gregs[REG_##reg]) \
                                  << std::endl;
void posix_dump_proc_context(ucontext_t *ctx)
{
    std::cerr << "Platform registers:" << std::endl;
    DUMP_REG(RIP)
    DUMP_REG(RSP)
    DUMP_REG(RBP)
    DUMP_REG(RAX)
    DUMP_REG(RBX)
    DUMP_REG(RCX)
    DUMP_REG(RDX)
    DUMP_REG(R8)
    DUMP_REG(R9)
    DUMP_REG(R10)
    DUMP_REG(R11)
    DUMP_REG(R12)
    DUMP_REG(R13)
    DUMP_REG(R14)
    DUMP_REG(R15)
    DUMP_REG(EFL)
    DUMP_REG(CR2)
}
#undef DUMP_REG
#else
void posix_dump_proc_context(ucontext_t *ctx)
{
    /* Unsupported platform */
}
#endif // __x86_64__

void posix_signal_handle_exit(int sig)
{
    PosixSignalCatcher::Ref().setCaughtSignal(sig);
    // TODO: Do something to exit
}

void posix_signal_handle_sigsegv(siginfo_t *info, ucontext_t *ctx)
{
    std::cerr << "[=========== Segmentation fault ===========]" << std::endl;
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

} // namespace cocoa
