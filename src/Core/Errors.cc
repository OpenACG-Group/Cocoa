#include <cstdio>
#include <cstdlib>

#include "Core/Errors.h"

namespace cocoa {

[[noreturn]] void __fatal_assert(const AssertionInfo& info)
{
    fprintf(stderr,
            "%s:\n  %s%s\n    Assertion `%s' failed.\n",
            info.file_line,
            info.function,
            *info.function ? ":" : "",
            info.message);
    fflush(stderr);
    std::abort();
}

[[noreturn]] void __fatal_oom_error()
{
    fprintf(stderr, "Exited with EXIT_STATUS_OOM[%d]\n", EXIT_STATUS_OOM);
    std::exit(EXIT_STATUS_OOM);
}

}
