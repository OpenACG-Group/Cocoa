#ifndef __VERSION_H__
#define __VERSION_H__

#ifndef COCOA_PROJECT
#error Project.h only can be included in Cocoa Project
#endif

#define co_noncopyable(T) \
public:                   \
T(const T&) = delete;     \
T& operator=(const T&) = delete;

#define COCOA_MAJOR     1
#define COCOA_MINOR     0
#define COCOA_PATCH     0

#define COCOA_NAME      "Cocoa"

#define COCOA_MAKE_VERSION(major, minor, patch)     (((major) << 24) | ((minor) << 16) | (patch))

#define COCOA_VERSION   "1.0.0-develop"
#define COCOA_LICENSE   "General Public License (GPLv3)"

#define co_nodiscard    [[nodiscard]]

/* Compiler compatibilities */
#define _SHARP #
#if defined(__clang__)
#define COCOA_COMPILER_PRAGMA_DIAGNOSTIC_PUSH \
    clang diagnostic push
#define COCOA_COMPILER_PRAGMA_DIAGNOSTIC_POP \
    clang diagnostic pop
#define COCOA_COMPILER_PRAGMA_DIAGNOSTIC_IGNORE(ignore) \
    clang diagnostic ignore #ignore
#elif defined(__GNUC__) || defined(__GNUG__)
  #define COCOA_COMPILER_PRAGMA_DIAGNOSTIC_PUSH #pragma GCC diagnostic push
  #define COCOA_COMPILER_PRAGMA_DIAGNOSTIC_POP  #pragma GCC diagnostic pop
  #define COCOA_COMPILER_PRAGMA_DIAGNOSTIC_IGNORE(ignore) #pragma GCC diagnostic ignore #ignore
#else
  #define COCOA_COMPILER_PRAGMA_DIAGNOSTIC_PUSH
  #define COCOA_COMPILER_PRAGMA_DIAGNOSTIC_POP
  #define COCOA_COMPILER_PRAGMA_DIAGNOSTIC_IGNORE(ignore)
#endif
#undef _SHARP

#endif // __VERSION_H__
