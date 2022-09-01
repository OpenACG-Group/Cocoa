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

#include "Core/Properties.h"
#include "Core/Errors.h"
#include "Core/Utils.h"
#include "Gallium/UnixPathTools.h"

GALLIUM_NS_BEGIN
namespace unixpath
{

std::string SolveShortestPathRepresentation(const std::string& path)
{
    CHECK(utils::StrStartsWith(path, '/') && "Path should be an absolute path");
    auto cwd = prop::Cast<PropertyDataNode>(prop::Get()
                ->next("Runtime")
                ->next("CurrentPath"))
                ->extract<std::string>();
    CHECK(utils::StrStartsWith(path, '/') && "Runtime.CurrentPath should be an absolute path");
    if (cwd != "/" && cwd[cwd.length() - 1] != '/')
        cwd.push_back('/');

    /* Find the longest common prefix (LCP) [0, cp) of `path` and `cwd` */
    int32_t cp = 0;
    while (cp < path.length() && cp < cwd.length() && path[cp] == cwd[cp])
        cp++;

    std::string result;
    for (int32_t i = cp; i < cwd.length(); i++)
    {
        if (cwd[i] == '/')
            result += "../";
    }
    result += path.substr(cp);
    return (result.length() < path.length()) ? result : path;
}

} // namespace unixpath
GALLIUM_NS_END
