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

#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <sstream>
#include "Core/Errors.h"
#include <optional>

#include "fmt/format.h"
#include "Core/Journal.h"
#include "Core/Filesystem.h"
namespace cocoa {

namespace {

int OpenRealJournalFile(const char *path)
{
    if (vfs::Access(path, {vfs::AccessMode::kExist}) == vfs::AccessResult::kOk)
    {
        std::string newName(std::string(path) + ".old");
        if (vfs::Rename(path, newName) < 0)
        {
            fmt::print("Failed to rename old log file {}: {}\n", newName, std::strerror(errno));
            return -1;
        }
    }

    int fd = vfs::Open(path, {vfs::OpenFlags::kWriteOnly, vfs::OpenFlags::kCreate},
                       {vfs::Mode::kUsrR, vfs::Mode::kUsrW, vfs::Mode::kGrpR, vfs::Mode::kGrpW, vfs::Mode::kOthR});
    if (fd < 0)
    {
        fmt::print("Failed to open log file {}: {}\n", path, std::strerror(errno));
        return -1;
    }
    return fd;
}

struct Token
{
    enum class Type
    {
        kFragment,
        kDecorator
    };

    explicit Token(Type t) : type(t) {}
    virtual ~Token() = default;
    Type    type;
};

struct Fragment : public Token
{
    Fragment() : Token(Type::kFragment) {}
    ~Fragment() override = default;
    std::string_view view;
};

struct Decorator : public Token
{
    Decorator() : Token(Type::kDecorator) {}
    ~Decorator() override = default;
    std::string_view specifier;
    std::vector<std::string_view> args;
};

void unacceptable_pattern(int pos, char const *message)
{
    throw std::runtime_error(fmt::format("[{:3d}] Error: {}", pos, message));
}

bool is_ident(char ch)
{
    return std::isdigit(ch) || std::isalpha(ch) || ch == '_';
}

void match_and_transit(char ch, const std::vector<char>& match, const std::vector<int>& transition, int& st)
{
    CHECK(match.size() == transition.size());

    for (int i = 0; i < match.size(); i++)
    {
        if (match[i] == ch)
        {
            st = transition[i];
            break;
        }
    }
}

std::vector<std::shared_ptr<Token>> parse_decorators(const std::string_view& origin)
{
    std::vector<std::shared_ptr<Token>> tokens;
    std::shared_ptr<Token> current;
    int len = origin.length();
    int state = 0;

    auto as_fragment = [&current]() -> std::shared_ptr<Fragment> {
        return std::static_pointer_cast<Fragment>(current);
    };

    auto as_decorator = [&current]() -> std::shared_ptr<Decorator> {
        return std::static_pointer_cast<Decorator>(current);
    };

    for (int i = 0; i < len + 1; i++)
    {
        char ch = origin[i];
        switch (state)
        {
        case 0:
            if (ch == '\0' || ch == '%' || ch == '\\')
            {
                if (current)
                {
                    as_fragment()->view.remove_suffix(len - i);
                    tokens.emplace_back(std::move(current));
                }
                match_and_transit(ch, {'%', '\0', '\\'}, {1, 4, 5}, state);
            }
            else
            {
                if (!current)
                {
                    current = std::make_shared<Fragment>();
                    as_fragment()->view = std::string_view(origin);
                    as_fragment()->view.remove_prefix(i);
                }
                // Implicit transition: 0 -> 0
            }
            break;

        case 1:
            if (ch == '<' || ch == '\0')
            {
                if (!current)
                    unacceptable_pattern(i, "Unexpected whitespace, '<' or null-terminator");
                as_decorator()->specifier.remove_suffix(len - i);
                match_and_transit(ch, {'<', '\0'}, {2, 4}, state);
                if (ch != '<')
                    tokens.emplace_back(std::move(current));
            }
            else if (is_ident(ch))
            {
                if (!current)
                {
                    current = std::make_shared<Decorator>();
                    as_decorator()->specifier = std::string_view(origin);
                    as_decorator()->specifier.remove_prefix(i);
                }
                // Implicit transition: 1 -> 1
            }
            else
            {
                if (!current)
                    unacceptable_pattern(i, "Unexpected character after %");
                as_decorator()->specifier.remove_suffix(len - i);
                tokens.emplace_back(std::move(current));
                i--;
                state = 0;
            }
            break;

        case 2:
            if (ch == ' ' || ch == '\t')
            {
                // Implicit transition: 2 -> 2
                break;
            }
            else if (ch == '>')
            {
                // as_decorator()->specifier.remove_suffix(len - i);
                tokens.emplace_back(std::move(current));
                state = 0;
            }
            else if (is_ident(ch))
            {
                as_decorator()->args.emplace_back(origin);
                as_decorator()->args.back().remove_prefix(i);
                state = 3;
            }
            else if (ch == '\0')
                unacceptable_pattern(i, "Unexpected null-terminator in %specifier<...>");
            else
                unacceptable_pattern(i, "Unexpected character in %specifier<...>");
            break;

        case 3:
            if (ch == ',')
            {
                as_decorator()->args.back().remove_suffix(len - i);
                state = 2;
            }
            else if (ch == '>')
            {
                as_decorator()->args.back().remove_suffix(len - i);
                tokens.emplace_back(std::move(current));
                state = 0;
            }
            else if (is_ident(ch))
            {
                // Implicit transition: 3 -> 3
                break;
            }
            else
                unacceptable_pattern(i, "Unexpected character");
            break;

        case 4:
            // Accepted
            break;

        case 5:
            current = std::make_shared<Fragment>();
            as_fragment()->view = origin;
            as_fragment()->view.remove_prefix(i);
            as_fragment()->view.remove_suffix(len - i - 1);
            tokens.emplace_back(std::move(current));
            state = 0;
            break;

        default:
            unacceptable_pattern(i, "Internal: Invalid state");
            break;
        }
    }

    return tokens;
}

struct TranslationContext
{
    bool enabled;
    std::chrono::steady_clock::time_point startTime;
    std::shared_ptr<Decorator> decorator;
    bool enableColor;
};

struct TranslationResult
{
    enum class Operation
    {
        kRemove,
        kReplace
    };

    TranslationResult() : op(Operation::kRemove) {}
    explicit TranslationResult(std::string replace)
        : op(Operation::kReplace), replacement(std::move(replace)) {}
    Operation op;
    std::optional<std::string> replacement;
};

using TranslatorFunc = TranslationResult(*)(TranslationContext*);
struct Translator
{
    const char *specifier;
    int argc;
    TranslatorFunc pfn;
};

bool match_list(const std::string_view& str, const std::vector<std::string>& vec)
{
    for (const auto& item : vec)
        if (str == item)
            return true;
    return false;
}

Translator translators[] = {
        {
            "disable",
            0,
            [](TranslationContext *ctx) -> TranslationResult {
                ctx->enabled = false;
                return {};
            }
        },
        {
            "enable",
            0,
            [](TranslationContext *ctx) -> TranslationResult {
                ctx->enabled = true;
                return {};
            }
        },
        {
            "timestamp",
            0,
            [](TranslationContext *ctx) -> TranslationResult {
                using namespace std::chrono;
                if (!ctx->enabled)
                    return {};
                auto duration = duration_cast<microseconds>(steady_clock::now() - ctx->startTime);
                double dt = static_cast<double>(duration.count()) *
                            microseconds::period::num / microseconds::period::den;
                return TranslationResult(fmt::format("[{:12.6f}]", dt));
            }
        },
        {
            "reset",
            0,
            [](TranslationContext *ctx) -> TranslationResult {
                if (!ctx->enabled || !ctx->enableColor)
                    return {};
                return TranslationResult("\033[0m");
            }
        },
        {
            "fg",
            -1,
            [](TranslationContext *ctx) -> TranslationResult {
                if (!ctx->enabled || !ctx->enableColor)
                    return {};
                std::string buf;
                for (const auto& item : ctx->decorator->args)
                {
                    if (match_list(item, {"bk", "black"}))
                        buf.append("\033[30m");
                    else if (match_list(item, {"re", "red"}))
                        buf.append("\033[31m");
                    else if (match_list(item, {"gr", "green"}))
                        buf.append("\033[32m");
                    else if (match_list(item, {"ye", "yellow"}))
                        buf.append("\033[33m");
                    else if (match_list(item, {"bl", "blue"}))
                        buf.append("\033[34m");
                    else if (match_list(item, {"ma", "magenta"}))
                        buf.append("\033[35m");
                    else if (match_list(item, {"cy", "cyan"}))
                        buf.append("\033[36m");
                    else if (match_list(item, {"wh", "white"}))
                        buf.append("\033[37m");
                    else if (match_list(item, {"hl", "highlight"}))
                        buf.append("\033[1m");
                    else
                        throw std::runtime_error(fmt::format("Unknown color code \"{}\"", item));
                }
                return TranslationResult(buf);
            }
        },
        {
            "bg",
            -1,
            [](TranslationContext *ctx) -> TranslationResult {
                if (!ctx->enabled || !ctx->enableColor)
                    return {};
                std::string buf;
                for (const auto& item : ctx->decorator->args)
                {
                    if (match_list(item, {"bk", "black"}))
                        buf.append("\033[40m");
                    else if (match_list(item, {"re", "red"}))
                        buf.append("\033[41m");
                    else if (match_list(item, {"gr", "green"}))
                        buf.append("\033[42m");
                    else if (match_list(item, {"ye", "yellow"}))
                        buf.append("\033[43m");
                    else if (match_list(item, {"bl", "blue"}))
                        buf.append("\033[44m");
                    else if (match_list(item, {"ma", "magenta"}))
                        buf.append("\033[45m");
                    else if (match_list(item, {"cy", "cyan"}))
                        buf.append("\033[46m");
                    else if (match_list(item, {"wh", "white"}))
                        buf.append("\033[47m");
                    else
                        throw std::runtime_error(fmt::format("Unknown color code \"{}\"", item));
                }
                return TranslationResult(buf);
            }
        },
        {
            "italic",
            0,
            [](TranslationContext *ctx) -> TranslationResult {
                if (!ctx->enabled || !ctx->enableColor)
                    return {};
                return TranslationResult("\033[3m");
            }
        },
        {
            "pid",
            0,
            [](TranslationContext *ctx) -> TranslationResult {
                if (!ctx->enabled)
                    return {};
                return TranslationResult(fmt::format("{}", getpid()));
            }
        },
        {
            "tid",
            0,
            [](TranslationContext *ctx) -> TranslationResult {
                if (!ctx->enabled)
                    return {};
                return TranslationResult(fmt::format("{}", gettid()));
            }
        }
};

std::string translate_decorators(const std::string_view& origin,
                                 std::chrono::steady_clock::time_point startTime,
                                 bool color)
{
    auto tokens = parse_decorators(origin);
    TranslationContext ctx{};
    ctx.enabled = true;
    ctx.startTime = startTime;
    ctx.enableColor = color;

    std::ostringstream finalString;

    for (const auto& t : tokens)
    {
        if (t->type == Token::Type::kDecorator)
        {
            auto decorator = std::static_pointer_cast<Decorator>(t);
            ctx.decorator = decorator;
            const Translator *translator = nullptr;
            for (const Translator& trans : translators)
                if (trans.specifier == decorator->specifier)
                {
                    translator = &trans;
                    break;
                }

            if (translator == nullptr)
                throw std::runtime_error(fmt::format("Invalid decorator specifier \"{}\"", decorator->specifier));
            if (translator->argc >= 0 && decorator->args.size() != translator->argc)
            {
                throw std::runtime_error(fmt::format("Decorator \"{}\" requires {} argument(s), but {} are provided",
                                                     translator->specifier, translator->argc, decorator->args.size()));
            }
            TranslationResult result = translator->pfn(&ctx);
            if (result.op == TranslationResult::Operation::kReplace)
            {
                CHECK(result.replacement.has_value());
                finalString << result.replacement.value();
            }
        }
        else
        {
            auto f = std::static_pointer_cast<Fragment>(t);
            finalString << f->view;
        }
    }
    return finalString.str();
}

std::vector<std::string_view> separate_lines(const std::string& str)
{
    std::vector<std::string_view> views;
    views.emplace_back(str);

    size_t pos = 0;
    size_t len = str.length();
    while ((pos = str.find('\n', pos + 1)) != std::string::npos)
    {
        views.back().remove_suffix(len - pos);
        views.emplace_back(str);
        views.back().remove_prefix(pos + 1);
    }
    return views;
}

} // namespace anonymous

Journal::Journal(LogLevel level, OutputDevice output,
                 bool enableColor, const char *file)
    : fEnableColor(enableColor),
      fLevel(level),
      fOutputFd(-1),
      fStartTime(std::chrono::steady_clock::now())
{
    switch (output)
    {
    case OutputDevice::kStandardOut:
        fOutputFd = STDOUT_FILENO;
        break;
    case OutputDevice::kStandardError:
        fOutputFd = STDERR_FILENO;
        break;
    case OutputDevice::kFile:
        fOutputFd = OpenRealJournalFile(file);
        break;
    }

    if (fOutputFd < 0)
        throw std::runtime_error("Failed to open log file");
}

Journal::~Journal()
{
    if (fOutputFd != STDOUT_FILENO && fOutputFd == STDERR_FILENO)
        vfs::Close(fOutputFd);
}

bool Journal::filter(LogType type)
{
    return (static_cast<uint32_t>(fLevel) & static_cast<uint32_t>(type)) == type;
}

void Journal::commit(LogType type, const std::string& str)
{
    const char *levelStr = nullptr;
    const char *levelColor = nullptr;
    switch (type)
    {
    case LOG_DEBUG:
        levelStr = "debug";
        levelColor = "cy";
        break;
    case LOG_INFO:
        levelStr = "info";
        levelColor = "gr";
        break;
    case LOG_WARNING:
        levelStr = "warn";
        levelColor = "ye";
        break;
    case LOG_EXCEPTION:
        levelStr = "fatal";
        levelColor = "re,hl";
        break;
    case LOG_ERROR:
        levelStr = "error";
        levelColor = "re";
        break;
    default:
        throw std::runtime_error("Unknown log level");
    }

    std::vector<std::string_view> lineViews = separate_lines(str);
    std::string finalStr;
    for (const auto& view : lineViews)
    {
        std::string formatted = fmt::format(
                "%fg<ma>%timestamp%reset %fg<{}>[{}:%tid]%reset {}",
                levelColor, levelStr, view);
        finalStr.append(translate_decorators(formatted, fStartTime, fEnableColor));
        finalStr.push_back('\n');
    }

    std::scoped_lock<std::mutex> lock(fWriteMutex);
    vfs::Write(fOutputFd, finalStr.c_str(), finalStr.length());
}

} // namespace cocoa
