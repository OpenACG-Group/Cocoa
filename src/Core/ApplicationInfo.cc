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
#include <algorithm>

#include "fmt/format.h"

#include "Core/ApplicationInfo.h"
#include "Core/Filesystem.h"
#include "Core/Utils.h"
COCOA_BEGIN_NAMESPACE

namespace {

const char *test_directory_env_variable(const char *name, bool report_if_unset)
{
    const char *value = ::getenv(name);
    if (!value || std::strlen(value) == 0)
    {
        if (report_if_unset)
            fmt::print(stderr, "Error: Environment variable ${} not set or empty\n", name);
        return nullptr;
    }

    if (value[0] != '/')
    {
        fmt::print(stderr, "Error: Environment variable ${} points to a relative directory\n", name);
        return nullptr;
    }

    if (!vfs::IsDirectory(value))
    {
        fmt::print(stderr, "Error: Environment variable ${} points to an invalid directory\n", name);
        return nullptr;
    }
    return value;
}

std::vector<std::string> test_directory_list_env_variable(const char *name)
{
    const char *value = ::getenv(name);
    if (!value || std::strlen(value) == 0)
        return {};

    std::string value_str = value;
    auto vec = utils::SplitString(value_str, ':');
    for (const std::string_view& sv : vec)
    {
        if (sv[0] != '/')
        {
            fmt::print(stderr, "Error: Environment variable ${} contains a relative directory {}\n",
                       name, sv);
            return {};
        }
        /* We do NOT check whether the directory exists. */
    }

    std::vector<std::string> result(vec.size());
    std::transform(vec.begin(), vec.end(), result.begin(), [](const std::string_view& sv) -> std::string {
        return std::string(sv);
    });
    return result;
}

/**
 * See also: XDG Base Directory Specification from Freedesktop.org
 * https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
 */
bool initialize_system_path_table(ApplicationInfo *app)
{
    /* Necessary system and user directories */
    const char *home = test_directory_env_variable("HOME", true);
    if (!home)
        return false;
    app->HOME = home;

    const char *runtime_dir_env = test_directory_env_variable("XDG_RUNTIME_DIR", true);
    if (!runtime_dir_env)
        return false;
    app->XDG_RUNTIME_DIR = runtime_dir_env;

    /* User directories */
    const char *user_data_env = test_directory_env_variable("XDG_DATA_HOME", false);
    app->XDG_DATA_HOME = user_data_env
                        ? user_data_env
                        : vfs::Realpath(fmt::format("{}/.local/share", home));

    const char *user_config_env = test_directory_env_variable("XDG_CONFIG_HOME", false);
    app->XDG_CONFIG_HOME = user_config_env
                           ? user_config_env
                           : vfs::Realpath(fmt::format("{}/.config", home));

    const char *user_cache_env = test_directory_env_variable("XDG_CACHE_HOME", false);
    app->XDG_CACHE_HOME = user_cache_env
                          ? user_cache_env
                          : vfs::Realpath(fmt::format("{}/.cache", home));

    /* System directories */
    app->XDG_DATA_DIRS = test_directory_list_env_variable("XDG_DATA_DIRS");
    if (app->XDG_DATA_DIRS.empty())
    {
        app->XDG_DATA_DIRS.emplace_back("/usr/local/share");
        app->XDG_DATA_DIRS.emplace_back("/usr/share");
    }

    app->XDG_CONFIG_DIRS = test_directory_list_env_variable("XDG_CONFIG_DIRS");
    if (app->XDG_CONFIG_DIRS.empty())
        app->XDG_CONFIG_DIRS.emplace_back("/etc/xdg");

    return true;
}

void initialize_internal_path_table(ApplicationInfo *app)
{
    app->program_file_path = utils::GetExecutablePath();
    app->program_path = app->program_path.substr(
            0, app->program_path.find_last_of('/') + 1);

    app->working_dir = utils::GetAbsoluteDirectory(".");
}

} // namespace anonymous

ApplicationInfo::ApplicationInfo() = default;
ApplicationInfo::~ApplicationInfo() = default;

bool ApplicationInfo::Setup()
{
    ApplicationInfo::New();

    ApplicationInfo *app = ApplicationInfo::Instance();

    initialize_internal_path_table(app);

    if (!initialize_system_path_table(app))
    {
        ApplicationInfo::Delete();
        return false;
    }

    return true;
}

COCOA_END_NAMESPACE
