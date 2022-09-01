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
