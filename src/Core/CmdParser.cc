#include <iostream>
#include <cmath>
#include <climits>

#include "fmt/format.h"
#include "fmt/ostream.h"

#include "Core/Errors.h"
#include "Core/MeasuredTable.h"
#include "Core/Utils.h"
#include "Core/CmdParser.h"

namespace cocoa::cmd {

const Template g_templates[] = {
        {
            .long_name = "help",
            .short_name = 'h',
            .desc = "Display available options."
        },
        {
            .long_name = "version",
            .short_name = 'v',
            .desc = "Display version information."
        },
        {
            .long_name = "log-file",
            .short_name = 'o',
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Specify a file where the log will be written."
        },
        {
            .long_name = "log-stderr",
            .desc = "Print logs to standard error."
        },
        {
            .long_name = "log-level",
            .short_name = 'L',
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Specify log level;\n"
                    "values: debug,normal,quiet,silent,disabled."
        },
        {
            .long_name = "disable-log-decoration",
            .desc = "Do NOT write logs with colors in ANSI escape code."
        },
        {
            .long_name = "initialize-only",
            .desc = "Exit immediately after finishing all the\n"
                    "initialization steps (not running script)"
        },
        {
            .long_name = "disable-traceback-symbol-folding",
            .desc = "Disable symbols folding of traceback information\n"
                    "in exception report."
        },
        {
            .long_name = "v8-concurrent-workers",
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kInteger,
            .desc = "Specify the number of worker threads to allocate\n"
                    "for background jobs for V8."
        },
        {
            .long_name = "v8-options",
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Pass the comma separated arguments to V8."
        },
        {
            .long_name = "runtime-inspector",
            .has_value = Template::RequireValue::kOptional,
            .value_type = ValueType::kInteger,
            .desc = "Start with V8 inspector to debug JavaScript;\n"
                    "optionally specify a port number to listen on (9005 by default)."
        },
        {
            .long_name = "runtime-inspector-no-script",
            .has_value = Template::RequireValue::kEmpty,
            .desc = "Do NOT run startup script after connecting to debugger;\n"
                    "code snippets can be executed in the REPL interface of debugger."
        },
        {
            .long_name = "runtime-blacklist",
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Specify a comma separated blacklist of language bindings."
        },
        {
            .long_name = "runtime-preload",
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Specify a path of a dynamic shared object to load\n"
                    "as a language binding."
        },
        {
            .long_name = "runtime-allow-override",
            .has_value = Template::RequireValue::kEmpty,
            .desc = "Language bindings with the same name can override each other.\n"
                    "Note that the option is dangerous and should only be used for\n"
                    "testing purpose. It allows a language binding to replace the\n"
                    "internal language bindings, which can cause serious security problems."
        },
        {
            .long_name = "introspect-policy",
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Enable or disable functions in 'introspect' global object;\n"
                    "values: {Allow,Forbid}{LoadingSharedObject,WritingToJournal}"
        },
        {
            .long_name = "pass",
            .short_name = 'A',
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Pass a delimiter separated arguments list to JavaScript;\n"
                    "the delimiter can be specified by --pass-delimiter."
        },
        {
            .long_name = "pass-delimiter",
            .short_name = 'D',
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Specify a character as delimiter, comma (,) by default.\n"
                    "See also --pass option for details."
        },
        {
            .long_name = "startup",
            .short_name = 's',
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Specify a JavaScript file to run (index.js by default)."
        },
        {
            .long_name = "gl-transfer-queue-profile",
            .has_value = Template::RequireValue::kEmpty,
            .desc = "Enable profiling on RenderHost's message queue and the profiling\n"
                    "result will be stored as a JSON file in working directory."
        },
        {
            .long_name = "gl-use-jit",
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kBoolean,
            .desc = "Allow skia to use JIT to accelerate CPU-bound operations\n"
                    "(true by default)."
        },
        {
            .long_name = "gl-concurrent-workers",
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kInteger,
            .desc = "Specify the number of worker threads for rendering."
        },
        {
            .long_name = "gl-show-tile-boundaries",
            .has_value = Template::RequireValue::kEmpty,
            .desc = "Draw tile boundaries if tiled rendering is available"
        },
        {
            .long_name = "gl-disable-hwcompose",
            .has_value = Template::RequireValue::kEmpty,
            .desc = "Disable vulkan hardware acceleration, which makes\n"
                    "the HWCompose surface unavailable."
        },
        {
            .long_name = "gl-hwcompose-enable-vkdbg",
            .has_value = Template::RequireValue::kEmpty,
            .desc = "Enable Vulkan debug utils to generate detailed Vulkan logs."
        },
        {
            .long_name = "gl-hwcompose-vkdbg-severities",
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Specify a comma separated list of allowed message\n"
                    "severities for vulkan debug utils."
        },
        {
            .long_name = "gl-hwcompose-vkdbg-levels",
            .has_value = Template::RequireValue::kNecessary,
            .value_type = ValueType::kString,
            .desc = "Specify a comma separated list of allowed message\n"
                    "types for vulkan debug utils."
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

namespace {

size_t tty_printable_strlen(const std::string& str)
{
    bool is_escape = false;
    size_t size = 0;
    for (char ch : str)
    {
        if (ch == '\033')
            is_escape = true;
        else if (is_escape && ch == 'm')
            is_escape = false;
        else if (!is_escape)
            size++;
    }
    return size++;
}

struct ValueTypeInfo
{
    const char *color_ansi;
    const char *name;
};

std::unordered_map<ValueType, ValueTypeInfo> g_value_type_infos = {
    { ValueType::kString,  { "\033[32;1m", "string" } },
    { ValueType::kInteger, { "\033[36;1m", "int"    } },
    { ValueType::kBoolean, { "\033[33;1m", "bool"   } },
    { ValueType::kFloat,   { "\033[35;1m", "float"  } }
};

} // namespace anonymous

void PrintHelp(const char *program)
{
    fmt::print(
R"(Cocoa 2D Rendering Framework, version {}
Usage {} [<options>...] [--] [<path>]

AVAILABLE OPTIONS:
)", COCOA_VERSION, program);

    for (const auto& p : g_templates)
    {
        std::ostringstream hdr_stream;

        hdr_stream << "\033[1m";
        if (p.short_name.has_value())
        {
            hdr_stream << "  -"
                       << p.short_name.value()
                       << ",--"
                       << p.long_name;
        }
        else
        {
            hdr_stream << "  --"
                       << p.long_name;
        }
        hdr_stream << "\033[0m";

        if (p.has_value != Template::RequireValue::kEmpty)
        {
            const char *close = "";
            hdr_stream << "=";

            hdr_stream << g_value_type_infos[p.value_type.value()].color_ansi;
            if (p.has_value == Template::RequireValue::kNecessary)
            {
                close = ">";
                hdr_stream << "<";
            }
            else if (p.has_value == Template::RequireValue::kOptional)
            {
                close = ">]";
                hdr_stream << "[<";
            }

            hdr_stream << g_value_type_infos[p.value_type.value()].name;
            hdr_stream << close << "\033[0m";
        }

        hdr_stream << ' ';

        std::string hdr = hdr_stream.str();
        fmt::print("{}", hdr);

        size_t offset;
        size_t printable_len = tty_printable_strlen(hdr);
        if (hdr.length() > 20)
        {
            fmt::print("\n");
            offset = 20;
        }
        else
        {
            offset = 20UL - printable_len;
        }

        std::vector<std::string_view> desc_lines = utils::SplitString(p.desc, '\n');
        for (const auto& line : desc_lines)
        {
            fmt::print("{}{}\n", std::string(offset, ' '), line);
            offset = 20;
        }
        fmt::print("\n");
    }
}

} // namespace cocoa::cmd
