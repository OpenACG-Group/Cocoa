#ifndef COCOA_ERRORS_H
#define COCOA_ERRORS_H

namespace cocoa {

struct AssertionInfo
{
    const char *file_line;
    const char *function;
    const char *message;
};

// NOLINTNEXTLINE
[[noreturn]] void __fatal_assert(const AssertionInfo& info);

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

} // namespace cocoa
#endif //COCOA_ERRORS_H
