/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef COCOA_CORE_PROJECT_H
#define COCOA_CORE_PROJECT_H

#include <type_traits>

#ifndef COCOA_PROJECT
#error Project.h only can be included in Cocoa Project
#endif

// Project versions
#define COCOA_MAJOR     1
#define COCOA_MINOR     0
#define COCOA_PATCH     0

// Project basic information
#define COCOA_NAME                  "Cocoa"
#define COCOA_FREEDESKTOP_APPID     "org.OpenACG.Cocoa"
#define COCOA_PLATFORM              "GNU/Linux"

#define COCOA_MAKE_VERSION(major, minor, patch)     (((major) << 24) | ((minor) << 16) | (patch))

#define COCOA_VERSION   "1.0.0-develop"
#define COCOA_LICENSE   "General Public License (GPLv3)"

#define COCOA_COPYRIGHT_YEAR    "2022"

// Property specifiers for functions, methods, member variables.
// Can be used as function signature.
#define g_private_api
#define g_nodiscard     [[nodiscard]]
#define g_noreturn      [[noreturn]]
#define g_inline        inline
#define g_maybe_unused  [[maybe_unused]]
#define g_async_api
#define g_sync_api
#define g_locked_sync_api

// Property specifiers for classes
#define CO_NONCOPYABLE(T) \
    T(const T&) = delete;

#define CO_NONASSIGNABLE(T) \
    T& operator=(const T&) = delete;

#define CO_CLASS_HAS_EXTRACT_VALUE \
    constexpr static bool kHasExtractValue = true;


// Environment variables to control the behaviour of Cocoa
#define ENV_GL_XCURSOR_THEME        "XCURSOR_THEME"
#define ENV_GL_XCURSOR_SIZE         "XCURSOR_SIZE"

#endif // COCOA_CORE_PROJECT_H
