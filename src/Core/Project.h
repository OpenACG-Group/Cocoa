#ifndef __PROJECT_H__
#define __PROJECT_H__

#include <type_traits>

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
#define COCOA_PLATFORM  "GNU/Linux"

#define COCOA_MAKE_VERSION(major, minor, patch)     (((major) << 24) | ((minor) << 16) | (patch))

#define COCOA_VERSION   "1.0.0-develop"
#define COCOA_LICENSE   "General Public License (GPLv3)"

#define COCOA_COPYRIGHT_YEAR    "2022"

#define g_private_api
#define g_nodiscard     [[nodiscard]]
#define g_noreturn      [[noreturn]]
#define g_inline        inline
#define g_maybe_unused  [[maybe_unused]]
#define g_async_api
#define g_sync_api

#define CO_NONCOPYABLE(T) \
    T(const T&) = delete;

#define CO_NONASSIGNABLE(T) \
    T& operator=(const T&) = delete;

namespace cocoa
{
template<typename T>
inline auto enum_underlying_value(T e)
{
    return static_cast<typename std::underlying_type<T>::type>(e);
}

} // namespace cocoa
#endif // __PROJECT_H__
