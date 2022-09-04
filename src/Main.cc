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

#include <iostream>
#include <optional>
#include <vector>
#include <string_view>

#include "fmt/format.h"
#include "fmt/ostream.h"

#include "Core/Project.h"
#include "Core/Properties.h"
#include "Core/Errors.h"
#include "Core/Utils.h"
#include "Core/MeasuredTable.h"
#include "Core/Journal.h"
#include "Core/Exception.h"
#include "Core/EventLoop.h"
#include "Core/Filesystem.h"
#include "Core/QResource.h"
#include "Core/ProcessSignalHandler.h"
#include "Core/subprocess/SubprocessHost.h"

#include "Gallium/Runtime.h"
#include "Gallium/BindingManager.h"

#include "Glamor/Glamor.h"

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Main)

namespace cocoa {
namespace cmd {

enum class ValueType
{
    kString,
    kInteger,
    kFloat,
    kBoolean
};

struct Template
{
    enum class RequireValue
    {
        kEmpty,
        kNecessary,
        kOptional
    };

    const char                 *long_name = nullptr;
    std::optional<char>         short_name;
    RequireValue                has_value = RequireValue::kEmpty;
    std::optional<ValueType>    value_type;
    const char                 *desc = nullptr;
};

enum class ParseState
{
    kExit,
    kSuccess,
    kError,
    kJustInitialize
};

struct ParseResult
{
    struct Option
    {
        struct Value
        {
            std::string     v_str;
            int32_t         v_int;
            float           v_float;
            bool            v_bool;
        };
        const Template          *matched_template = nullptr;
        std::string              origin;
        std::optional<Value>     value;
    };

    std::vector<const char*> orphans;
    std::vector<Option>      options;
};

const Template g_templates[] = {
        {
            .long_name = "help",
            .short_name = 'h',
            .desc = "Display available options"
        },
        {
            .long_name = "version",
            .short_name = 'v',
            .desc = "Display version information"
        },
        {
            .long_name = "log-file",
            .short_name = 'o',
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Specify a file to print logs"
        },
        {
            .long_name = "log-stderr",
            .desc = "Print logs to standard error"
        },
        {
            .long_name = "log-level",
            .short_name = 'L',
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Specify log level. Valid arguments: debug|normal|quiet|silent|disabled"
        },
        {
            .long_name = "disable-log-decoration",
            .desc = "Don't print logs with colors through ANSI escape code"
        },
        {
            .long_name = "initialize-only",
            .desc = "Exit immediately after finishing all the initialization steps (not running script)"
        },
        {
            .long_name = "disable-traceback-symbol-folding",
            .desc = "Do not fold symbols of traceback information in exception report"
        },
        {
            .long_name = "v8-concurrent-workers",
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kInteger,
            .desc = "Specify the number of worker threads to Allocate for background jobs for V8"
        },
        {
            .long_name = "v8-options",
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Pass the comma separated arguments to V8"
        },
        {
            .long_name = "runtime-blacklist",
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Specify a comma separated blacklist of language bindings"
        },
        {
            .long_name = "runtime-preload",
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Specify a path of a dynamic library to load it as language bindings"
        },
        {
            .long_name = "runtime-allow-override",
            .has_value = Template::RequireValue::kEmpty,
            .desc = "Language bindings with the same name can override each other"
        },
        {
            .long_name = "runtime-expose-introspect",
            .has_value = Template::RequireValue::kOptional,
            .value_type = ValueType::kBoolean,
            .desc = "Specify whether VM expose 'introspect' global object to JavaScript land"
        },
        {
            .long_name = "introspect-policy",
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Enable/disable functions in 'introspect' global object"
        },
        {
            .long_name = "pass",
            .short_name = 'A',
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Specify a delimiter separated list passed to JavaScript"
        },
        {
            .long_name = "pass-delimiter",
            .short_name = 'D',
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Specify a character as delimiter. Default is ','"
        },
        {
            .long_name = "startup",
            .short_name = 's',
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Specify a JavaScript file to run. (index.js for default)"
        },
        {
            .long_name = "gl-transfer-queue-profile",
            .has_value = Template::RequireValue::kEmpty,
            .desc = "Enable profiling on RenderHost's message queue"
        },
        {
            .long_name = "gl-use-jit",
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kBoolean,
            .desc = "Use JIT to accelerate CPU-bound operations while rendering (true by default)"
        },
        {
            .long_name = "gl-concurrent-workers",
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kInteger,
            .desc = "Specify the number of worker threads for tile rendering, rasterization, etc."
        },
        {
            .long_name = "gl-show-tile-boundaries",
            .has_value = Template::RequireValue::kEmpty,
            .desc = "Draw tile boundaries if tiled rendering is available"
        },
        {
            .long_name = "gl-disable-hwcompose",
            .has_value = Template::RequireValue::kEmpty,
            .desc = "Disable Vulkan-based hardware acceleration (disable HWCompose surfaces)"
        },
        {
            .long_name = "gl-hwcompose-enable-vkdbg",
            .has_value = Template::RequireValue::kEmpty,
            .desc = "Enable Vulkan debug utils to generate detailed Vulkan logs"
        },
        {
            .long_name = "gl-hwcompose-vkdbg-severities",
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Specify a comma separated list of allowed message severities for Vulkan debug utils"
        },
        {
            .long_name = "gl-hwcompose-vkdbg-levels",
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Specify a comma separated list of allowed message types for Vulkan debug utils"
        }
};

namespace {

const Template *match_template(const std::string_view& longOpt)
{
    for (const Template& t : g_templates)
    {
        if (longOpt == t.long_name)
            return &t;
    }
    return nullptr;
}

const Template *match_template(char shortOpt)
{
    for (const Template& t : g_templates)
    {
        if (t.short_name.has_value() && shortOpt == t.short_name)
            return &t;
    }
    return nullptr;
}

bool interpret_and_set_option_value(ParseResult::Option& opt, const std::string_view& str)
{
    std::string stored(str);

    switch (opt.matched_template->value_type.value())
    {
    case ValueType::kString:
        opt.value = ParseResult::Option::Value{.v_str = stored};
        break;
    case ValueType::kInteger:
    {
        char *endptr = nullptr;
        long n = std::strtol(stored.c_str(), &endptr, 10);
        if (endptr != stored.c_str() + stored.length())
        {
            fmt::print(std::cerr, "Couldn't interpret the argument of option \"{}\" as an integer\n", opt.origin);
            return false;
        }
        opt.value = ParseResult::Option::Value{.v_int = static_cast<int32_t>(n)};
        break;
    }

    case ValueType::kFloat:
    {
        char *endptr = nullptr;
        float n = std::strtof(stored.c_str(), &endptr);
        if (endptr != stored.c_str() + stored.length())
        {
            fmt::print(std::cerr, "Couldn't interpret the argument of option \"{}\" as a number\n", opt.origin);
            return false;
        }
        opt.value = ParseResult::Option::Value{.v_float = n};
        break;
    }

    case ValueType::kBoolean:
    {
        bool v;
        if (str == "true" || str == "TRUE")
            v = true;
        else if (str == "false" || str == "FALSE")
            v = false;
        else
        {
            fmt::print(std::cerr, "Couldn't interpret the argument of option \"{}\" as a boolean\n", opt.origin);
            return false;
        }
        opt.value = ParseResult::Option::Value{.v_bool = v};
        break;
    }
    }

    return true;
}

/* size = 2^7 * 2^7 * sizeof(int) = 2^16 bytes = 64KB */
int dp[128][128];

// If user gives an unrecognized option name due to spelling mistake, we try guessing
// the most possibly right option name by calculating Levenshtein Distance.
// Supposing `s1` and `s2` are two strings, their Levenshtein Distance `lev(s1, s2)`
// is a number N which represents that `s1` can be changed into `s2` after N times' single-character
// edits (insertions, deletions, substitutions) at least.
int solve_levenshtein_distance(const std::string_view& a, const std::string_view& b)
{
    CHECK(a.size() < 128 && b.size() < 128);

    size_t m = a.size(), n = b.size();

    for (int i = 0; i <= m; i++)
        dp[i][0] = i;
    for (int j = 0; j <= n; j++)
        dp[0][j] = j;

    for (int i = 1; i <= m; i++)
    {
        for (int j = 1; j <= n; j++)
        {
            if (a[i - 1] == b[j - 1])
                dp[i][j] = dp[i - 1][j - 1];
            else
                dp[i][j] = std::min({dp[i][j-1] + 1, dp[i-1][j] + 1, dp[i-1][j-1] + 1});
        }
    }
    return dp[m][n];
}

const char *most_possible_long_option_spell(const std::string_view& opt)
{
    int minDis = INT_MAX;
    const char *minOpt;
    for (const auto& t : g_templates)
    {
        int dis = solve_levenshtein_distance(opt, t.long_name);
        if (dis < minDis)
        {
            minDis = dis;
            minOpt = t.long_name;
        }
    }

    if (minDis > 4)
        return nullptr;
    return minOpt;
}

bool interpret_and_set_long_option(ParseResult::Option& opt, const std::string_view& str)
{
    std::string_view optionView(str);
    std::string_view valueView;
    optionView.remove_prefix(2);

    size_t equalPos = str.find_first_of('=');
    if (equalPos != std::string_view::npos)
    {
        if (equalPos + 1 == str.length())
        {
            fmt::print(std::cerr, "Unnecessary \"=\" in option \"{}\"\n", str);
            return false;
        }
        optionView.remove_suffix(str.length() - equalPos);
        valueView = str;
        valueView.remove_prefix(equalPos + 1);
    }

    opt.matched_template = match_template(optionView);
    if (!opt.matched_template)
    {
        const char *possible = most_possible_long_option_spell(optionView);
        if (possible)
            fmt::print(std::cerr, "Unrecognized long options \"{}\", did you mean \"--{}\"?\n", str, possible);
        else
            fmt::print(std::cerr, "Unrecognized long option \"{}\"\n", str);
        return false;
    }

    if (opt.matched_template->has_value == Template::RequireValue::kEmpty &&
        !valueView.empty())
    {
        fmt::print(std::cerr, "Unnecessary argument in option \"{}\"\n", str);
        return false;
    }

    opt.origin = str;
    if (!valueView.empty())
        return interpret_and_set_option_value(opt, valueView);
    else if (opt.matched_template->has_value == Template::RequireValue::kNecessary)
    {
        fmt::print(std::cerr, "Expecting an argument for option \"{}\"\n", str);
        return false;
    }
    return true;
}

bool interpret_and_set_short_options(ParseResult& result, const std::string_view& str)
{
    std::string_view p(str);
    p.remove_prefix(1);

    if (p.empty())
    {
        fmt::print(std::cerr, "Empty short option is not allowed\n");
        return false;
    }

    for (auto i = p.begin(); i != p.end(); i++)
    {
        ParseResult::Option opt;
        opt.matched_template = match_template(*i);
        if (!opt.matched_template)
        {
            fmt::print(std::cerr, "Unrecognized short option \"-{}\" in the short option sequence \"{}\"\n", *i, str);
            return false;
        }

        if (opt.matched_template->has_value == Template::RequireValue::kNecessary &&
            i != p.end() - 1)
        {
            fmt::print(std::cerr, "Short option \"-{}\" which requires an argument can only "
                       "be the last option in the short option sequence\n", *i);
            return false;
        }
        opt.origin = std::string("-") + *i;
        result.options.push_back(opt);
    }

    return true;
}

} // namespace anonymous

ParseState Parse(int argc, const char **argv, ParseResult& result)
{
    std::optional<ParseResult::Option*> pendingOption;
    for (uint32_t i = 1; i < argc; i++)
    {
        std::string current(argv[i]);
        if (current == "--")
        {
            if (pendingOption.has_value())
            {
                fmt::print(std::cerr, "Option {} expects an argument\n", pendingOption.value()->origin);
                return ParseState::kError;
            }

            for (uint32_t j = i + 1; j < argc; j++)
                result.orphans.push_back(argv[j]);
            break;
        }
        else if (utils::StrStartsWith(current, "--"))
        {
            ParseResult::Option opt;
            if (!interpret_and_set_long_option(opt, current))
            {
                fmt::print(std::cerr, "Illegal option \"{}\"\n", current);
                return ParseState::kError;
            }
            result.options.push_back(opt);
        }
        else if (utils::StrStartsWith(current, '-'))
        {
            if (!interpret_and_set_short_options(result, current))
            {
                fmt::print(std::cerr, "Illegal option \"{}\"\n", current);
                return ParseState::kError;
            }
            auto hasValue = result.options.back().matched_template->has_value;
            if (hasValue == Template::RequireValue::kOptional ||
                hasValue == Template::RequireValue::kNecessary)
            {
                pendingOption = &result.options.back();
                continue;
            }
        }
        else if (pendingOption.has_value())
        {
            if (!interpret_and_set_option_value(*pendingOption.value(), current))
            {
                fmt::print(std::cerr, "Bad argument \"{}\" for option {}\n", current,
                           pendingOption.value()->origin);
                return ParseState::kError;
            }
            pendingOption.reset();
        }
        else
        {
            result.orphans.push_back(argv[i]);
        }

        if (pendingOption.has_value())
        {
            if (pendingOption.value()->matched_template->has_value == Template::RequireValue::kNecessary)
            {
                fmt::print(std::cerr, "Option {} expects an argument\n", pendingOption.value()->origin);
                return ParseState::kError;
            }
            pendingOption.reset();
        }
    }

    return ParseState::kSuccess;
}

void startup_print_help(const char *program)
{
    fmt::print(
R"(Cocoa 2D Rendering Framework, version {}
Usage {} [<options>...] [--] [<path>]

AVAILABLE OPTIONS:
)",
    COCOA_VERSION, program);

    MeasuredTable table;
    for (const auto& p : g_templates)
    {
        std::string hdr("--");
        hdr.append(p.long_name);
        if (p.short_name.has_value())
        {
            hdr.append(", -");
            hdr.push_back(p.short_name.value());
        }

        if (p.has_value != Template::RequireValue::kEmpty)
        {
            const char *close = "";
            if (p.has_value == Template::RequireValue::kNecessary)
            {
                close = ">";
                hdr.append(" <");
            }
            else if (p.has_value == Template::RequireValue::kOptional)
            {
                close = ">]";
                hdr.append(" [<");
            }
            switch (p.value_type.value())
            {
            case ValueType::kString:    hdr.append("string"); break;
            case ValueType::kInteger:   hdr.append("int");    break;
            case ValueType::kFloat:     hdr.append("real");   break;
            case ValueType::kBoolean:   hdr.append("bool");   break;
            }
            hdr.append(close);
        }

        table.append(hdr, p.desc);
    }

    table.flush([](const std::string& line) -> void {
        fmt::print("  {}\n", line);
    });
}

void startup_print_version()
{
    fmt::print("Cocoa 2D Rendering Framework Version {}\n", COCOA_VERSION);
    fmt::print("Copyright (C) " COCOA_COPYRIGHT_YEAR " OpenACG Group | GPLv3 License\n");
}

void startup_print_greeting(const gallium::Runtime::Options& opts)
{
    QLOG(LOG_INFO, "%fg<hl>Cocoa 2D Rendering Framework, version {}%reset", COCOA_VERSION);
    QLOG(LOG_INFO, "  %fg<hl>Copyright (C) " COCOA_COPYRIGHT_YEAR " OpenACG Group | GPLv3 License%reset");
    QLOG(LOG_INFO, "  %fg<hl>libuv asynchronous I/O, version {}%reset", uv_version_string());
    QLOG(LOG_INFO, "  %fg<hl>Google V8 JavaScript Engine, version {}.{}%reset", V8_MAJOR_VERSION, V8_MINOR_VERSION);
    QLOG(LOG_INFO, "  %fg<hl>Google Skia 2D Library%reset");
    QLOG(LOG_INFO, "Startup script %fg<ye,hl>{}%reset", opts.startup);
}

} // namespace cmd

#define arg_longopt_match(s) (!std::strcmp(arg.matched_template->long_name, s))

cmd::ParseState InitializeLogger(cmd::ParseResult& args)
{
    const char *file = nullptr;
    LogLevel level = LOG_LEVEL_NORMAL;
    bool color = true;
    Journal::OutputDevice output = Journal::OutputDevice::kStandardOut;

    for (const auto& arg : args.options)
    {
        if arg_longopt_match("log-file")
        {
            file = arg.value.value().v_str.c_str();
            output = Journal::OutputDevice::kFile;
        }
        else if arg_longopt_match("log-stderr")
            output = Journal::OutputDevice::kStandardError;
        else if arg_longopt_match("log-level")
        {
            if (arg.value->v_str == "debug")         level = LOG_LEVEL_DEBUG;
            else if (arg.value->v_str == "normal")   level = LOG_LEVEL_NORMAL;
            else if (arg.value->v_str == "quiet")    level = LOG_LEVEL_QUIET;
            else if (arg.value->v_str == "silent")   level = LOG_LEVEL_SILENT;
            else if (arg.value->v_str == "disabled") level = LOG_LEVEL_DISABLED;
            else
            {
                fmt::print(std::cerr, "Illegal specifier for log level: {}\n", arg.value->v_str);
                return cmd::ParseState::kError;
            }
        }
        else if arg_longopt_match("disable-log-decoration")
            color = false;
    }
    if (output == Journal::OutputDevice::kFile)
        color = false;

    Journal::New(level, output, color, file);
    return cmd::ParseState::kSuccess;
}

const char *test_directory_env_variable(const char *name, bool report_if_unset)
{
    const char *value = ::getenv(name);
    if (!value || std::strlen(value) == 0)
    {
        if (report_if_unset)
            fmt::print(std::cerr, "Error: Environment variable ${} not set or empty\n", name);
        return nullptr;
    }

    if (value[0] != '/')
    {
        fmt::print(std::cerr, "Error: Environment variable ${} points to a relative directory\n", name);
        return nullptr;
    }

    if (!vfs::IsDirectory(value))
    {
        fmt::print(std::cerr, "Error: Environment variable ${} points to an invalid directory\n", name);
        return nullptr;
    }
    return value;
}

std::vector<std::string> test_directory_list_env_variable(const char *name)
{
    const char *value = ::getenv(name);
    if (!value || std::strlen(value) == 0)
        return {};

    std::string valueDump = value;
    auto vec = utils::SplitString(valueDump, ':');
    for (const std::string_view& sv : vec)
    {
        if (sv[0] != '/')
        {
            fmt::print(std::cerr, "Error: Environment variable ${} contains a relative directory {}\n",
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

cmd::ParseState initialize_path_table_properties()
{
    auto runtime = prop::Get()->next("Runtime")->as<PropertyObjectNode>();
    auto paths = runtime->setMember("Paths", prop::New<PropertyObjectNode>())->as<PropertyObjectNode>();

    const char *home = test_directory_env_variable("HOME", true);
    if (!home)
        return cmd::ParseState::kError;
    paths->setMember("Home", prop::New<PropertyDataNode>(home));

    const char *userDataEnv = test_directory_env_variable("XDG_DATA_HOME", false);
    std::string userData = userDataEnv ? userDataEnv
                           : vfs::Realpath(fmt::format("{}/.local/share", home));
    paths->setMember("UserData", prop::New<PropertyDataNode>(userData));

    const char *userConfigEnv = test_directory_env_variable("XDG_CONFIG_HOME", false);
    std::string userConfig = userConfigEnv ? userConfigEnv
                             : vfs::Realpath(fmt::format("{}/.config", home));
    paths->setMember("UserConfig", prop::New<PropertyDataNode>(userConfig));

    std::vector<std::string> systemDatas = test_directory_list_env_variable("XDG_DATA_DIRS");
    if (systemDatas.empty())
    {
        systemDatas.emplace_back("/usr/local/share");
        systemDatas.emplace_back("/usr/share");
    }
    paths->setMember("SystemData", prop::New<PropertyArrayNode>()->append(systemDatas));

    std::vector<std::string> systemConfigs = test_directory_list_env_variable("XDG_CONFIG_DIRS");
    if (systemConfigs.empty())
        systemConfigs.emplace_back("/etc/xdg");
    paths->setMember("SystemConfig", prop::New<PropertyArrayNode>()->append(systemConfigs));

    const char *userCacheEnv = test_directory_env_variable("XDG_CACHE_HOME", false);
    std::string userCache = userCacheEnv ? userCacheEnv
                            : vfs::Realpath(fmt::format("{}/.cache", home));
    paths->setMember("UserCache", prop::New<PropertyDataNode>(userCache));

    const char *runtimeDirEnv = test_directory_env_variable("XDG_RUNTIME_DIR", true);
    if (!runtimeDirEnv)
        return cmd::ParseState::kError;
    paths->setMember("Runtime", prop::New<PropertyDataNode>(runtimeDirEnv));

    return cmd::ParseState::kSuccess;
}

cmd::ParseState initialize_properties(int argc, const char **argv, cmd::ParseResult& args)
{
    if (args.orphans.size() > 1)
    {
        fmt::print(std::cerr, "Too many arguments\n");
        return cmd::ParseState::kError;
    }
    else if (!args.orphans.empty())
    {
        if (vfs::Chdir(args.orphans[0]) < 0)
        {
            fmt::print(std::cerr, "Failed to chdir to \"{}\": {}\n", args.orphans[0], strerror(errno));
            return cmd::ParseState::kError;
        }
    }

    /* Set `runtime` property object */
    {
        std::string execFile = utils::GetExecutablePath();
        std::string execPath = execFile.substr(0, execFile.find_last_of('/') + 1);

        auto runtimeProp = prop::New<PropertyObjectNode>();
        auto cmdlineProp = prop::New<PropertyArrayNode>();
        for (int32_t i = 0; i < argc; i++)
            cmdlineProp->append(prop::New<PropertyDataNode>(argv[i]));
        runtimeProp->setMember("Cmdline", cmdlineProp);

        std::string workingPath = utils::GetAbsoluteDirectory(".");
        runtimeProp->setMember("ExecutableFile", prop::New<PropertyDataNode>(execFile));
        runtimeProp->setMember("ExecutablePath", prop::New<PropertyDataNode>(execPath));
        runtimeProp->setMember("CurrentPath", prop::New<PropertyDataNode>(workingPath));
        prop::Get()->setMember("Runtime", runtimeProp);

        auto state = initialize_path_table_properties();
        if (state != cmd::ParseState::kSuccess)
            return state;
    }

    /* Set `system` property object */
    {
        auto systemProp = prop::New<PropertyObjectNode>();
        systemProp->setMember("VirtualMemPageSize", prop::New<PropertyDataNode>(utils::GetMemPageSize()));
        systemProp->setMember("CpuModelName", prop::New<PropertyDataNode>(utils::GetCpuModel()));
        systemProp->setMember("VirtualMemSize", prop::New<PropertyDataNode>(utils::GetMemTotalSize()));
        prop::Get()->setMember("System", systemProp);
    }

    return cmd::ParseState::kSuccess;
}

void report_vulnerability_option(const std::string& opt)
{
    QLOG(LOG_WARNING, "%bg<re>%fg<hl>(Vulnerability)%reset Option %fg<hl>\"{}\"%reset"
                      " may cause fatal security problems", opt);
}

std::shared_ptr<PropertyArrayNode> splitStringStoreToArrayNode(const std::string& str, char delimiter)
{
    auto array = prop::New<PropertyArrayNode>();
    auto v = utils::SplitString(str, delimiter);
    for (const auto& l : v)
        array->append(prop::New<PropertyDataNode>(std::string(l)));

    return array;
}

cmd::ParseState startup_initialize(int argc, char const **argv,
                                   gallium::Runtime::Options& gallium_options,
                                   gl::ContextOptions& glamor_options)
{
    cmd::ParseResult args;
    cmd::ParseState state = cmd::Parse(argc, argv, args);
    if (state == cmd::ParseState::kError)
        return cmd::ParseState::kError;

    for (const auto& arg : args.options)
    {
        if arg_longopt_match("help")
        {
            cmd::startup_print_help(argv[0]);
            return cmd::ParseState::kExit;
        }
        else if arg_longopt_match("version")
        {
            cmd::startup_print_version();
            return cmd::ParseState::kExit;
        }
    }

    /* Initialize logger */
    state = InitializeLogger(args);
    if (state == cmd::ParseState::kError)
        return cmd::ParseState::kError;

    /* Initialize necessary properties */
    state = initialize_properties(argc, argv, args);
    if (state == cmd::ParseState::kError)
        return cmd::ParseState::kError;

    EventLoop::New();

    auto lbpPreloads = prop::New<PropertyArrayNode>();
    auto lbpBlacklist = prop::New<PropertyArrayNode>();
    auto scriptArgs = prop::New<PropertyArrayNode>();
    char delimiter = ',';

    bool justInitialize = false;

    auto graphicsNode = prop::New<PropertyObjectNode>();
    prop::Get()->setMember("Graphics", graphicsNode);

    auto hwComposeNode = prop::New<PropertyObjectNode>();
    graphicsNode->setMember("HWCompose", hwComposeNode);

    for (const auto& arg : args.options)
    {
        if arg_longopt_match("disable-traceback-symbol-folding")
        {
            // TODO: implement this.
        }
        else if arg_longopt_match("initialize-only")
        {
            justInitialize = true;
        }
        else if arg_longopt_match("v8-concurrent-workers")
        {
            if (arg.value->v_int < 0)
            {
                fmt::print(std::cerr, "v8-concurrent-workers should ba a positive integer\n");
                return cmd::ParseState::kError;
            }
            gallium_options.v8_platform_thread_pool = arg.value->v_int;
        }
        else if arg_longopt_match("v8-options")
        {
            auto list = utils::SplitString(arg.value->v_str, ',');
            for (const auto& view : list)
                gallium_options.v8_options.emplace_back(view);
        }
        else if arg_longopt_match("runtime-blacklist")
        {
            std::vector<std::string_view> list = utils::SplitString(arg.value->v_str, ',');
            for (const auto& p : list)
            {
                gallium_options.bindings_blacklist.emplace_back(p);
                lbpBlacklist->append(prop::New<PropertyDataNode>(std::string(p)));
            }
        }
        else if arg_longopt_match("runtime-preload")
        {
            lbpPreloads->append(prop::New<PropertyDataNode>(arg.value->v_str));
        }
        else if arg_longopt_match("runtime-allow-override")
        {
            gallium_options.rt_allow_override = true;
            report_vulnerability_option("--runtime-allow-override");
        }
        else if arg_longopt_match("pass")
        {
            std::vector<std::string_view> argsView = utils::SplitString(arg.value->v_str, delimiter);
            for (const auto& view : argsView)
            {
                scriptArgs->append(prop::New<PropertyDataNode>(std::string(view)));
            }
        }
        else if arg_longopt_match("pass-delimiter")
        {
            if (arg.value->v_str.size() > 1)
            {
                fmt::print(std::cerr, "Delimiter must be a single character\n");
                return cmd::ParseState::kError;
            }
            delimiter = arg.value->v_str[0];
        }
        else if arg_longopt_match("runtime-expose-introspect")
        {
            gallium_options.rt_expose_introspect = !arg.value || arg.value->v_bool;
        }
        else if arg_longopt_match("introspect-policy")
        {
            auto split = utils::SplitString(arg.value->v_str, ',');
            for (auto& policy : split)
            {
                if (policy == "AllowLoadingSharedObject")
                    gallium_options.introspect_allow_loading_shared_object = true;
                else if (policy == "AllowWritingToJournal")
                    gallium_options.introspect_allow_write_journal = true;
                else if (policy == "ForbidLoadingSharedObject")
                    gallium_options.introspect_allow_loading_shared_object = false;
                else if (policy == "ForbidWritingToJournal")
                    gallium_options.introspect_allow_write_journal = false;
                else
                {
                    fmt::print(std::cerr, "Error: Unrecognized introspect policy: {}\n", policy);
                    return cmd::ParseState::kError;
                }
            }
        }
        else if arg_longopt_match("startup")
        {
            gallium_options.startup = arg.value->v_str;
        }
        else if arg_longopt_match("gl-use-jit")
        {
            glamor_options.SetSkiaJIT(arg.value->v_bool);
        }
        else if arg_longopt_match("gl-concurrent-workers")
        {
            glamor_options.SetRenderWorkersConcurrencyCount(arg.value->v_int);
        }
        else if arg_longopt_match("gl-show-tile-boundaries")
        {
            glamor_options.SetShowTileBoundaries(true);
        }
        else if arg_longopt_match("gl-disable-hwcompose")
        {
            hwComposeNode->setMember("Disabled", prop::New<PropertyDataNode>(true));
        }
        else if arg_longopt_match("gl-hwcompose-enable-vkdbg")
        {
            hwComposeNode->setMember("EnableVkDBG", prop::New<PropertyDataNode>(true));
        }
        else if arg_longopt_match("gl-hwcompose-vkdbg-severities")
        {
            hwComposeNode->setMember("VkDBGFilterSeverities", splitStringStoreToArrayNode(arg.value->v_str, ','));
        }
        else if arg_longopt_match("gl-hwcompose-vkdbg-levels")
        {
            hwComposeNode->setMember("VkDBGFilterLevels", splitStringStoreToArrayNode(arg.value->v_str, ','));
        }
        else if arg_longopt_match("gl-transfer-queue-profile")
        {
            glamor_options.SetProfileRenderHostTransfer(true);
        }
    }

    {
        auto scriptNode = prop::New<PropertyObjectNode>();
        scriptNode->setMember("LoaderPreloads", lbpPreloads);
        scriptNode->setMember("LoaderBlacklist", lbpBlacklist);
        scriptNode->setMember("Pass", scriptArgs);
        prop::Cast<PropertyObjectNode>(prop::Get()->next("Runtime"))->setMember("Script", scriptNode);

        auto persistentNode = prop::New<PropertyObjectNode>();
        persistentNode->setMember("EventLoop", prop::New<PropertyDataNode>(EventLoop::Instance()));
        persistentNode->setMember("Journal", prop::New<PropertyDataNode>(Journal::Instance()));
        prop::Get()->setMember("Persistent", persistentNode);
    }

    return justInitialize ? cmd::ParseState::kJustInitialize : cmd::ParseState::kSuccess;
}

#undef arg_longopt_match

void mainloop_finalize()
{
    Journal::Delete();
}

void mainloop_execute(bool justInitialize,
                      const gallium::Runtime::Options& options,
                      const gl::ContextOptions& glamorOptions)
{
    QResource::New();

    prop::SerializeToJournal(prop::Get());

    gl::GlobalScope::New(glamorOptions, EventLoop::Instance());
    gallium::BindingManager::New(options);

    subproc::SubprocessHostRegistry::New();

    auto preloads = prop::Get()
            ->next("Runtime")
            ->next("Script")
            ->next("LoaderPreloads")->as<PropertyArrayNode>();
    for (const auto& p : *preloads)
    {
        auto& val = prop::Cast<PropertyDataNode>(p)->extract<std::string>();
        gallium::BindingManager::Ref().loadDynamicObject(val);
    }

    BeforeEventLoopEntrypointHook();
    if (!justInitialize)
    {
        auto runtime = gallium::Runtime::Make(EventLoop::Instance(), options);
        CHECK(runtime != nullptr);
        v8::Isolate::Scope isolateScope(runtime->isolate());
        v8::HandleScope handleScope(runtime->isolate());
        v8::Context::Scope contextScope(runtime->context());

        v8::Local<v8::Value> result;
        if (!runtime->evaluateModule(options.startup).ToLocal(&result))
            return;

        InstallSecondarySignalHandler();
        EventLoop::Ref().spin([&runtime] {
            // This will also perform the microtask queue checkpoint
            runtime->drainPlatformTasks();
        });

        gallium::BindingManager::Delete();

        runtime->notifyRuntimeWillExit();
        CHECK(runtime.unique() && "Runtime is referenced by other scopes");
    }
    else
    {
        fmt::print(std::cerr, "[TESTRUN] Cocoa exits after finishing initialization steps.\n");
    }


    // No matter whether these UniquePersistent objects are created,
    // deleting them is safe.
    gl::GlobalScope::Delete();

    // RenderHost message queue profiler may register a threadpool work.
    // To make sure the task performed properly, we run event loop again.
    EventLoop::Ref().run();

    subproc::SubprocessHostRegistry::Delete();
    QResource::Delete();
    EventLoop::Delete();
}

int startup_main(int argc, char const **argv)
{
    InstallPrimarySignalHandler();

    ScopeExitAutoInvoker epilogue([]() -> void {
        mainloop_finalize();
    });

    gallium::Runtime::Options gallium_options;
    gl::ContextOptions glamor_options;
    bool only_initialize = false;

    try {
        switch (startup_initialize(argc, argv, gallium_options, glamor_options))
        {
        case cmd::ParseState::kError:
            return EXIT_FAILURE;
        case cmd::ParseState::kExit:
            return EXIT_SUCCESS;
        case cmd::ParseState::kSuccess:
            break;
        case cmd::ParseState::kJustInitialize:
            only_initialize = true;
            break;
        }

        gallium::Runtime::AdoptV8CommandOptions(gallium_options);
        cmd::startup_print_greeting(gallium_options);
        mainloop_execute(only_initialize, gallium_options, glamor_options);
    } catch (const RuntimeException& e) {
        utils::SerializeException(e);
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

} // namespace cocoa

int main(int argc, char const *argv[])
{
    return cocoa::startup_main(argc, argv);
}
