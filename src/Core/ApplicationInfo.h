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

#ifndef COCOA_CORE_APPLICATIONINFO_H
#define COCOA_CORE_APPLICATIONINFO_H

#include <string>
#include <vector>

#include "Core/Project.h"
#include "Core/UniquePersistent.h"
COCOA_BEGIN_NAMESPACE

class ApplicationInfo : public UniquePersistent<ApplicationInfo>
{
public:
    ApplicationInfo();
    ~ApplicationInfo();

    using StringV = std::vector<std::string>;
    using String = std::string;

    static bool Setup();

    /* Cocoa path table */
    String     working_dir;
    String     program_file_path;
    String     program_path;

    /* System path table */
    String     HOME;
    String     XDG_DATA_HOME;
    String     XDG_CONFIG_HOME;
    StringV    XDG_DATA_DIRS;
    StringV    XDG_CONFIG_DIRS;
    String     XDG_CACHE_HOME;
    String     XDG_RUNTIME_DIR;

    /* JavaScript arguments */
    StringV    js_arguments;
    StringV    js_native_preloads;
    StringV    js_native_preloads_blacklist;

    String     js_first_script_name;
};

COCOA_END_NAMESPACE
#endif //COCOA_CORE_APPLICATIONINFO_H
