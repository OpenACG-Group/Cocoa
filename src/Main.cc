#include <iostream>
#include <optional>
#include <vector>
#include <cassert>
#include <string_view>

#include "Core/Project.h"
#include "Core/Properties.h"
#include "Core/Utils.h"
#include "Core/MeasuredTable.h"
#include "Core/Journal.h"
#include "Core/Exception.h"
#include "Core/EventLoop.h"

#include "Koi/Runtime.h"
#include "Koi/BindingsLoader.h"

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
    kError
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
            .longName = "disable-color-log",
            .desc = "Don't print logs with colors through ANSI escape code"
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
    assert(a.size() < 128 && b.size() < 128);

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
        else if (current.starts_with("--"))
        {
            ParseResult::Option opt;
            if (!interpret_and_set_long_option(opt, current))
            {
                std::cerr << "Illegal option \"" << current << "\"" << std::endl;
                return ParseState::kError;
            }
            result.options.push_back(opt);
        }
        else if (current.starts_with('-'))
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
    fmt::print("Copyright (C) 2021 OpenACG Group | GPLv3 License\n");
}

void PrintGreeting()
{
    LOGF(LOG_INFO, "%fg<hl>Cocoa 2D Rendering Framework, version {}%reset", COCOA_VERSION)
    LOGW(LOG_INFO, "%fg<hl>Copyright (C) 2021 OpenACG Group | GPLv3 License%reset")
    auto dir = prop::Cast<PropertyDataNode>(prop::Get()->next("runtime")->next("working-path"))
                ->extract<std::string>();
    LOGF(LOG_INFO, "Startup script %fg<ye,hl>{}/index.js%reset", dir)
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
        else if arg_longopt_match("disable-color-log")
            color = false;
    }
    if (output == Journal::OutputDevice::kFile)
        color = false;

    Journal::New(level, output, color, file);
    return cmd::ParseState::kSuccess;
}

cmd::ParseState InitializeProperties(int argc, const char **argv, cmd::ParseResult& args)
{
    std::string workingPath = utils::GetAbsoluteDirectory(".");
    if (args.orphans.size() > 1)
    {
        std::cerr << "Too many arguments" << std::endl;
        return cmd::ParseState::kError;
    }
    else if (!args.orphans.empty())
        workingPath = utils::GetAbsoluteDirectory(args.orphans[0]);

    /* Set `runtime` property object */
    {
        std::string execFile = utils::GetExecutablePath();
        std::string execPath = execFile.substr(0, execFile.find_last_of('/') + 1);

        auto runtimeProp = prop::New<PropertyObjectNode>();
        auto cmdlineProp = prop::New<PropertyArrayNode>();
        for (int32_t i = 0; i < argc; i++)
            cmdlineProp->append(prop::New<PropertyDataNode>(argv[i]));
        runtimeProp->setMember("cmdline", cmdlineProp);

        runtimeProp->setMember("executable-file", prop::New<PropertyDataNode>(execFile));
        runtimeProp->setMember("executable-path", prop::New<PropertyDataNode>(execPath));
        runtimeProp->setMember("working-path", prop::New<PropertyDataNode>(workingPath));
        prop::Get()->setMember("runtime", runtimeProp);
    }

    /* Set `system` property object */
    {
        auto systemProp = prop::New<PropertyObjectNode>();
        systemProp->setMember("mem-page-size", prop::New<PropertyDataNode>(utils::GetMemPageSize()));
        systemProp->setMember("cpu-model-name", prop::New<PropertyDataNode>(utils::GetCpuModel()));
        systemProp->setMember("mem-size", prop::New<PropertyDataNode>(utils::GetMemTotalSize()));
        prop::Get()->setMember("system", systemProp);
    }

    return cmd::ParseState::kSuccess;
}

cmd::ParseState Initialize(int argc, char const **argv)
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
    koi::Initialize();

    {
        auto persistentNode = prop::New<PropertyObjectNode>();
        persistentNode->setProtection(PropertyNode::Protection::kPrivate);
        persistentNode->setMember("event-loop", prop::New<PropertyDataNode>(EventLoop::Instance()));
        persistentNode->setMember("journal", prop::New<PropertyDataNode>(Journal::Instance()));
        prop::Get()->setMember("persistent", persistentNode);
    }

    return cmd::ParseState::kSuccess;
}

#undef arg_longopt_match

void Finalize()
{
    koi::Dispose();
    EventLoop::Delete();
    Journal::Delete();
}

void Execute()
{
    LOGW(LOG_INFO, "Current Properties Tree:")
    utils::DumpProperties(prop::Get());

    auto workingPath = prop::Cast<PropertyDataNode>(prop::Get()
            ->next("runtime")->next("working-path"))->extract<std::string>();
    auto execPath = prop::Cast<PropertyDataNode>(prop::Get()
            ->next("runtime")->next("executable-path"))->extract<std::string>();

    koi::PreloadInternalBindings();

    std::string snapshotFile = execPath + "snapshot_blob.bin";
    auto runtime = koi::Runtime::MakeFromSnapshot(EventLoop::Instance(), snapshotFile);
    v8::Isolate::Scope isolateScope(runtime->isolate());
    v8::HandleScope handleScope(runtime->isolate());
    v8::Context::Scope contextScope(runtime->context());

    v8::Local<v8::Value> result;
    if (!runtime->evaluateModule("index.js").ToLocal(&result))
        return;
    EventLoop::Ref().run();
}

int Main(int argc, char const **argv)
{
    ScopeEpilogue epilogue([]() -> void {
        Finalize();
    });

    try {
        switch (Initialize(argc, argv))
        {
        case cmd::ParseState::kError:
            return EXIT_FAILURE;

        case cmd::ParseState::kExit:
            return EXIT_SUCCESS;

        case cmd::ParseState::kSuccess:
            break;
        }

        cmd::PrintGreeting();
        Execute();
    } catch (const RuntimeException& e) {
        utils::DumpRuntimeException(e);
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
