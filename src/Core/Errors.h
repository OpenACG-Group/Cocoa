#ifndef COCOA_CORE_ERRORS_H
#define COCOA_CORE_ERRORS_H

namespace cocoa {

struct AssertionInfo
{
    const char *file_line;
    const char *function;
    const char *message;
};

[[noreturn]] void __fatal_assert(const AssertionInfo& info); // NOLINT
[[noreturn]] void __fatal_oom_error(); // NOLINT

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

#define LIKELY(expr) __builtin_expect(!!(expr), 1)
#define UNLIKELY(expr) __builtin_expect(!!(expr), 0)

#define CHECK_FAILED(message)                   \
    do {                                        \
        const cocoa::AssertionInfo __info = {     \
            __FILE__ ":" STRINGIFY(__LINE__),   \
            __PRETTY_FUNCTION__,                \
            #message                            \
        };                                      \
        cocoa::__fatal_assert(__info);            \
    } while (false)

#define CHECK(expr)                             \
    do {                                        \
        if (UNLIKELY(!(expr))) {                \
            CHECK_FAILED(expr);                 \
        }                                       \
    } while (false)

#define MARK_UNREACHABLE(...) \
    CHECK_FAILED("Unreachable code reached" __VA_OPT__(": ") __VA_ARGS__)

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS        0
#endif // EXIT_SUCCESS

#define EXIT_ERROR_BIT      (1 << 1)
#define EXIT_FATAL_BIT      (1 << 2)
#define EXIT_OOM_BIT        (1 << 3)

#define EXIT_STATUS_OOM     (EXIT_ERROR_BIT|EXIT_FATAL_BIT|EXIT_OOM_BIT)

} // namespace cocoa
#endif //COCOA_CORE_ERRORS_H
