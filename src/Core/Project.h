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

#define COCOA_MAKE_VERSION(major, minor, patch)     (((major) << 24) | ((minor) << 16) | (patch))

#define COCOA_VERSION   "1.0.0-develop"
#define COCOA_LICENSE   "General Public License (GPLv3)"

#define COCOA_COPYRIGHT_YEAR    "2022"

#define co_nodiscard    [[nodiscard]]
#define co_cdecl_begin  extern "C" {
#define co_cdecl_end    }

namespace cocoa
{
template<typename T>
inline auto enum_underlying_value(T e)
{
    return static_cast<typename std::underlying_type<T>::type>(e);
}

} // namespace cocoa
#endif // __PROJECT_H__
