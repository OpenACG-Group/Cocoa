#include <iostream>
#include <optional>
#include <vector>
#include <string_view>

#include "Core/Project.h"
#include "Core/Properties.h"
#include "Core/Errors.h"
#include "Core/Utils.h"
#include "Core/MeasuredTable.h"
#include "Core/Journal.h"
#include "Core/CpuInfo.h"
#include "Core/Exception.h"
#include "Core/EventLoop.h"
#include "Core/Filesystem.h"
#include "Core/QResource.h"
#include "fmt/format.h"
#include "include/core/SkTypes.h"

#include "Gallium/Runtime.h"
#include "Gallium/BindingManager.h"

#include "Cobalt/Cobalt.h"

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

    const char                 *longName = nullptr;
    std::optional<char>         shortName;
    RequireValue                hasValue = RequireValue::kEmpty;
    std::optional<ValueType>    valueType;
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
        const Template          *matchedTemplate = nullptr;
        std::string              origin;
        std::optional<Value>     value;
    };

    std::vector<const char*> orphans;
    std::vector<Option>      options;
};

const Template gTemplates[] = {
        {
            .longName = "help",
            .shortName = 'h',
            .desc = "Display available options"
        },
        {
            .longName = "version",
            .shortName = 'v',
            .desc = "Display version information"
        },
        {
            .longName = "log-file",
            .shortName = 'o',
            .hasValue = Template::RequireValue::kNecessary,
            .valueType = ValueType::kString,
            .desc = "Specify a file to print logs"
        },
        {
            .longName = "log-stderr",
            .desc = "Print logs to standard error"
        },
        {
            .longName = "log-level",
            .shortName = 'L',
            .hasValue = Template::RequireValue::kNecessary,
            .valueType = ValueType::kString,
            .desc = "Specify log level. Valid arguments: debug|normal|quiet|silent|disabled"
        },
        {
            .longName = "disable-log-decoration",
            .desc = "Don't print logs with colors through ANSI escape code"
        },
        {
            .longName = "just-initialize",
            .desc = "Exit immediately after finishing all the initialization steps (not running script)"
        },
        {
            .longName = "vm-thread-pool-size",
            .hasValue = Template::RequireValue::kNecessary,
            .valueType = ValueType::kInteger,
            .desc = "Specify the number of worker threads to allocate for background jobs for V8"
        },
        {
            .longName = "vm-options",
            .hasValue = Template::RequireValue::kNecessary,
            .valueType = ValueType::kString,
            .desc = "Pass the comma separated arguments to V8"
        },
        {
            .longName = "rt-blacklist",
            .hasValue = Template::RequireValue::kNecessary,
            .valueType = ValueType::kString,
            .desc = "Specify a comma separated blacklist of language bindings"
        },
        {
            .longName = "rt-preload",
            .hasValue = Template::RequireValue::kNecessary,
            .valueType = ValueType::kString,
            .desc = "Specify a path of a dynamic library to load it as language bindings"
        },
        {
            .longName = "rt-allow-override",
            .hasValue = Template::RequireValue::kEmpty,
            .desc = "Language bindings with the same name can override each other"
        },
        {
            .longName = "rt-expose-introspect",
            .hasValue = Template::RequireValue::kOptional,
            .valueType = ValueType::kBoolean,
            .desc = "Specify whether VM expose 'introspect' global object to JavaScript land"
        },
        {
            .longName = "introspect-policy",
            .hasValue = Template::RequireValue::kNecessary,
            .valueType = ValueType::kString,
            .desc = "Enable/disable functions in 'introspect' global object"
        },
        {
            .longName = "pass",
            .shortName = 'A',
            .hasValue = Template::RequireValue::kNecessary,
            .valueType = ValueType::kString,
            .desc = "Specify a delimiter separated list passed to JavaScript"
        },
        {
            .longName = "pass-delimiter",
            .shortName = 'D',
            .hasValue = Template::RequireValue::kNecessary,
            .valueType = ValueType::kString,
            .desc = "Specify a character as delimiter. Default is ','"
        },
        {
            .longName = "startup",
            .shortName = 's',
            .hasValue = Template::RequireValue::kNecessary,
            .valueType = ValueType::kString,
            .desc = "Specify a JavaScript file to run. (index.js for default)"
        },
        {
            .longName = "renderhost-transfer-profile",
            .hasValue = Template::RequireValue::kEmpty,
            .desc = "Enable profiling on RenderHost's message queue"
        },
        {
            .longName = "cobalt-use-jit",
            .hasValue = Template::RequireValue::kNecessary,
            .valueType = ValueType::kBoolean,
            .desc = "Use JIT to accelerate CPU-bound operations while rendering (true by default)"
        },
        {
            .longName = "disable-hwcompose",
            .hasValue = Template::RequireValue::kEmpty,
            .desc = "Disable Vulkan-based hardware acceleration (disable HWCompose surfaces)"
        },
        {
            .longName = "hwcompose-enable-vkdbg",
            .hasValue = Template::RequireValue::kEmpty,
            .desc = "Enable Vulkan debug utils to generate detailed Vulkan logs"
        },
        {
            .longName = "hwcompose-vkdbg-severities",
            .hasValue = Template::RequireValue::kNecessary,
            .valueType = ValueType::kString,
            .desc = "Specify a comma separated list of allowed message severities for Vulkan debug utils"
        },
        {
            .longName = "hwcompose-vkdbg-levels",
            .hasValue = Template::RequireValue::kNecessary,
            .valueType = ValueType::kString,
            .desc = "Specify a comma separated list of allowed message types for Vulkan debug utils"
        }
};

namespace {

const Template *match_template(const std::string_view& longOpt)
{
    for (const Template& t : gTemplates)
    {
        if (longOpt == t.longName)
            return &t;
    }
    return nullptr;
}

const Template *match_template(char shortOpt)
{
    for (const Template& t : gTemplates)
    {
        if (t.shortName.has_value() && shortOpt == t.shortName)
            return &t;
    }
    return nullptr;
}

bool interpret_and_set_option_value(ParseResult::Option& opt, const std::string_view& str)
{
    std::string stored(str);

    switch (opt.matchedTemplate->valueType.value())
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
            std::cerr << "Couldn't interpret the argument of option \""
                      << opt.origin << "\" as an integer" << std::endl;
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
            std::cerr << "Couldn't interpret the argument of option \""
                      << opt.origin << "\" as a number" << std::endl;
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
            std::cerr << "Couldn't interpret the argument of option \""
                      << opt.origin << "\" as a boolean value" << std::endl;
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
    for (const auto& t : gTemplates)
    {
        int dis = solve_levenshtein_distance(opt, t.longName);
        if (dis < minDis)
        {
            minDis = dis;
            minOpt = t.longName;
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
            std::cerr << R"(Unnecessary "=" in option ")"
                      << std::string(str) << '\"' << std::endl;
            return false;
        }
        optionView.remove_suffix(str.length() - equalPos);
        valueView = str;
        valueView.remove_prefix(equalPos + 1);
    }

    opt.matchedTemplate = match_template(optionView);
    if (!opt.matchedTemplate)
    {
        std::cerr << "Unrecognized long option \""
                  << std::string(str) << '\"';
        const char *possible = most_possible_long_option_spell(optionView);
        if (possible)
            std::cerr << ", did you mean \"--" << possible << "\"?";
        std::cerr << std::endl;
        return false;
    }

    if (opt.matchedTemplate->hasValue == Template::RequireValue::kEmpty &&
        !valueView.empty())
    {
        std::cerr << "Unnecessary argument in option \""
                  << std::string(str) << '\"' << std::endl;
        return false;
    }

    opt.origin = str;
    if (!valueView.empty())
        return interpret_and_set_option_value(opt, valueView);
    else if (opt.matchedTemplate->hasValue == Template::RequireValue::kNecessary)
    {
        std::cerr << "Expecting an argument for option \"" << std::string(str)
                  << "\"" << std::endl;
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
        std::cerr << "Empty short option is not allowed" << std::endl;
        return false;
    }

    for (auto i = p.begin(); i != p.end(); i++)
    {
        ParseResult::Option opt;
        opt.matchedTemplate = match_template(*i);
        if (!opt.matchedTemplate)
        {
            std::cerr << "Unrecognized short option \"-" << *i
                      << "\" in the short option sequence \"" << std::string(str) << "\""
                      << std::endl;
            return false;
        }

        if (opt.matchedTemplate->hasValue == Template::RequireValue::kNecessary &&
            i != p.end() - 1)
        {
            std::cerr << "Short option \"-" << *i << "\" which requires an argument can only "
                      << "be the last option in the short option sequence" << std::endl;
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
                std::cerr << "Option " << pendingOption.value()->origin
                          << " expects an argument" << std::endl;
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
                std::cerr << "Illegal option \"" << current << "\"" << std::endl;
                return ParseState::kError;
            }
            result.options.push_back(opt);
        }
        else if (utils::StrStartsWith(current, '-'))
        {
            if (!interpret_and_set_short_options(result, current))
            {
                std::cerr << "Illegal option \"" << current << "\"" << std::endl;
                return ParseState::kError;
            }
            auto hasValue = result.options.back().matchedTemplate->hasValue;
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
                std::cerr << "Bad argument \"" << current << "\" for option "
                          << pendingOption.value()->origin << std::endl;
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
            if (pendingOption.value()->matchedTemplate->hasValue == Template::RequireValue::kNecessary)
            {
                std::cerr << "Option " << pendingOption.value()->origin
                          << " expects an argument" << std::endl;
                return ParseState::kError;
            }
            pendingOption.reset();
        }
    }

    return ParseState::kSuccess;
}

void PrintHelp(const char *program)
{
    fmt::print(
R"(Cocoa 2D Rendering Framework, version {}
Usage {} [<options>...] [--] [<path>]

AVAILABLE OPTIONS:
)",
    COCOA_VERSION, program);

    MeasuredTable table;
    for (const auto& p : gTemplates)
    {
        std::string hdr("--");
        hdr.append(p.longName);
        if (p.shortName.has_value())
        {
            hdr.append(", -");
            hdr.push_back(p.shortName.value());
        }

        if (p.hasValue != Template::RequireValue::kEmpty)
        {
            const char *close = "";
            if (p.hasValue == Template::RequireValue::kNecessary)
            {
                close = ">";
                hdr.append(" <");
            }
            else if (p.hasValue == Template::RequireValue::kOptional)
            {
                close = ">]";
                hdr.append(" [<");
            }
            switch (p.valueType.value())
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

void PrintVersion()
{
    fmt::print("Cocoa 2D Rendering Framework Version {}\n", COCOA_VERSION);
    fmt::print("Copyright (C) " COCOA_COPYRIGHT_YEAR " OpenACG Group | GPLv3 License\n");
}

void PrintGreeting(const gallium::Runtime::Options& opts)
{
    QLOG(LOG_INFO, "%fg<hl>Cocoa 2D Rendering Framework, version {}%reset", COCOA_VERSION);
    QLOG(LOG_INFO, "  %fg<hl>Copyright (C) " COCOA_COPYRIGHT_YEAR " OpenACG Group | GPLv3 License%reset");
    QLOG(LOG_INFO, "  %fg<hl>libuv asynchronous I/O, version {}%reset", uv_version_string());
    QLOG(LOG_INFO, "  %fg<hl>Google V8 JavaScript Engine, version {}.{}%reset", V8_MAJOR_VERSION, V8_MINOR_VERSION);
    QLOG(LOG_INFO, "  %fg<hl>Google Skia 2D Library%reset");
    QLOG(LOG_INFO, "Startup script %fg<ye,hl>{}%reset", opts.startup);
}

} // namespace cmd

#define arg_longopt_match(s) (!std::strcmp(arg.matchedTemplate->longName, s))

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
                std::cerr << "Illegal specifier for log level: " << arg.value->v_str
                          << std::endl;
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

cmd::ParseState InitializeProperties(int argc, const char **argv, cmd::ParseResult& args)
{
    if (args.orphans.size() > 1)
    {
        std::cerr << "Too many arguments" << std::endl;
        return cmd::ParseState::kError;
    }
    else if (!args.orphans.empty())
    {
        if (vfs::Chdir(args.orphans[0]) < 0)
        {
            std::cerr << "Failed to chdir to \'" << args.orphans[0]
                      << "\': " << ::strerror(errno) << std::endl;
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

cmd::ParseState Initialize(int argc, char const **argv,
                           gallium::Runtime::Options& koiOptions,
                           cobalt::ContextOptions& cobaltOptions)
{
    cmd::ParseResult args;
    cmd::ParseState state = cmd::Parse(argc, argv, args);
    if (state == cmd::ParseState::kError)
        return cmd::ParseState::kError;

    for (const auto& arg : args.options)
    {
        if arg_longopt_match("help")
        {
            cmd::PrintHelp(argv[0]);
            return cmd::ParseState::kExit;
        }
        else if arg_longopt_match("version")
        {
            cmd::PrintVersion();
            return cmd::ParseState::kExit;
        }
    }

    /* Initialize logger */
    state = InitializeLogger(args);
    if (state == cmd::ParseState::kError)
        return cmd::ParseState::kError;

    /* Initialize necessary properties */
    state = InitializeProperties(argc, argv, args);
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
        if arg_longopt_match("just-initialize")
        {
            justInitialize = true;
        }
        else if arg_longopt_match("vm-thread-pool-size")
        {
            if (arg.value->v_int < 0)
            {
                std::cerr << "vm-thread-pool-size should ba a positive integer" << std::endl;
                return cmd::ParseState::kError;
            }
            koiOptions.v8_platform_thread_pool = arg.value->v_int;
        }
        else if arg_longopt_match("vm-options")
        {
            auto list = utils::SplitString(arg.value->v_str, ',');
            for (const auto& view : list)
                koiOptions.v8_options.emplace_back(view);
        }
        else if arg_longopt_match("rt-blacklist")
        {
            std::vector<std::string_view> list = utils::SplitString(arg.value->v_str, ',');
            for (const auto& p : list)
            {
                koiOptions.bindings_blacklist.emplace_back(p);
                lbpBlacklist->append(prop::New<PropertyDataNode>(std::string(p)));
            }
        }
        else if arg_longopt_match("rt-preload")
        {
            lbpPreloads->append(prop::New<PropertyDataNode>(arg.value->v_str));
        }
        else if arg_longopt_match("rt-allow-override")
        {
            koiOptions.rt_allow_override = true;
            report_vulnerability_option("--rt-allow-override");
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
                std::cerr << "Delimiter is considered to be a single character" << std::endl;
                return cmd::ParseState::kError;
            }
            delimiter = arg.value->v_str[0];
        }
        else if arg_longopt_match("rt-expose-introspect")
        {
            koiOptions.rt_expose_introspect = !arg.value || arg.value->v_bool;
        }
        else if arg_longopt_match("introspect-policy")
        {
            auto split = utils::SplitString(arg.value->v_str, ',');
            for (auto& policy : split)
            {
                if (policy == "AllowLoadingSharedObject")
                    koiOptions.introspect_allow_loading_shared_object = true;
                else if (policy == "AllowWritingToJournal")
                    koiOptions.introspect_allow_write_journal = true;
                else if (policy == "ForbidLoadingSharedObject")
                    koiOptions.introspect_allow_loading_shared_object = false;
                else if (policy == "ForbidWritingToJournal")
                    koiOptions.introspect_allow_write_journal = false;
                else
                {
                    fmt::print(std::cerr, "Error: Unrecognized introspect policy: {}\n", policy);
                    return cmd::ParseState::kError;
                }
            }
        }
        else if arg_longopt_match("startup")
        {
            koiOptions.startup = arg.value->v_str;
        }
        else if arg_longopt_match("cobalt-use-jit")
        {
            cobaltOptions.SetSkiaJIT(arg.value->v_bool);
        }
        else if arg_longopt_match("disable-hwcompose")
        {
            hwComposeNode->setMember("Disabled", prop::New<PropertyDataNode>(true));
        }
        else if arg_longopt_match("hwcompose-enable-vkdbg")
        {
            hwComposeNode->setMember("EnableVkDBG", prop::New<PropertyDataNode>(true));
        }
        else if arg_longopt_match("hwcompose-vkdbg-severities")
        {
            hwComposeNode->setMember("VkDBGFilterSeverities", splitStringStoreToArrayNode(arg.value->v_str, ','));
        }
        else if arg_longopt_match("hwcompose-vkdbg-levels")
        {
            hwComposeNode->setMember("VkDBGFilterLevels", splitStringStoreToArrayNode(arg.value->v_str, ','));
        }
        else if arg_longopt_match("renderhost-transfer-profile")
        {
            cobaltOptions.SetProfileRenderHostTransfer(true);
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

void Finalize()
{
    Journal::Delete();
    CpuInfo::Delete();
}

void Execute(bool justInitialize,
             const gallium::Runtime::Options& options,
             const cobalt::ContextOptions& cobaltOptions)
{
    QResource::New();

    prop::SerializeToJournal(prop::Get());

    cobalt::GlobalScope::New(cobaltOptions, EventLoop::Instance());
    gallium::BindingManager::New(options);

    auto preloads = prop::Get()
            ->next("Runtime")
            ->next("Script")
            ->next("LoaderPreloads")->as<PropertyArrayNode>();
    for (const auto& p : *preloads)
    {
        auto& val = prop::Cast<PropertyDataNode>(p)->extract<std::string>();
        gallium::BindingManager::Ref().loadDynamicObject(val);
    }

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
        EventLoop::Ref().run();
        gallium::BindingManager::Delete();

        runtime->notifyRuntimeWillExit();
        CHECK(runtime.unique() && "Runtime is referenced by other scopes");
    }
    else
    {
        fmt::print(std::cerr, "[TESTRUN] Cocoa exits after finishing initialization steps.\n");
    }


    /* No matter whether these UniquePersistent objects are created,
     * deleting them is safe. */
    cobalt::GlobalScope::Delete();

    /* RenderHost message queue profiler may register a threadpool work.
     * To make sure the task performed properly, we run event loop again. */
    EventLoop::Ref().run();

    QResource::Delete();
    EventLoop::Delete();
}

int Main(int argc, char const **argv)
{
    CpuInfo::New();
    ScopeEpilogue epilogue([]() -> void { Finalize(); });

    gallium::Runtime::Options koiOptions;
    cobalt::ContextOptions cobaltOptions;
    bool justInitialize = false;

    try {
        switch (Initialize(argc, argv, koiOptions, cobaltOptions))
        {
        case cmd::ParseState::kError:
            return EXIT_FAILURE;
        case cmd::ParseState::kExit:
            return EXIT_SUCCESS;
        case cmd::ParseState::kSuccess:
            break;
        case cmd::ParseState::kJustInitialize:
            justInitialize = true;
            break;
        }

        gallium::Runtime::AdoptV8CommandOptions(koiOptions);
        cmd::PrintGreeting(koiOptions);
        Execute(justInitialize, koiOptions, cobaltOptions);
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
    return cocoa::Main(argc, argv);
}
