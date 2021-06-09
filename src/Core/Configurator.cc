#include <initializer_list>
#include <vector>
#include <map>
#include <iostream>
#include <optional>
#include <regex>

#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Parser.h>

#include "Core/Utils.h"
#include "Core/Project.h"
#include "Core/PropertyTree.h"
#include "Core/Exception.h"
#include "Core/StrictJSONParser.h"
#include "Core/Configurator.h"

namespace {

using namespace cocoa::json;
using namespace cocoa;

#define OBJECT_MEMBER(name) &object_##name,
#define FINAL_VALUE_MEMBER(name) &finalValue_##name,
#define ARRAY_MEMBER(name)  &array_##name,

#define OBJECT_TEMPLATE(optional, name, members) \
const ObjectTemplate object_##name(optional, #name, members);

#define OBJECT_TEMPLATE_ANON(optional, name, members) \
const ObjectTemplate object_##name(optional, members);

#define FINAL_VALUE_TEMPLATE(optional, name, type) \
const FinalValueTemplate finalValue_##name(optional, #name, FinalValueTemplate::kType_##type);

#define FINAL_VALUE_TEMPLATE_ANON(optional, name, type) \
const FinalValueTemplate finalValue_##name(optional, FinalValueTemplate::kType_##type);

#define ARRAY_TEMPLATE(optional, name, element) \
const ArrayTemplate array_##name(optional, #name, element);

#define ARRAY_TEMPLATE_ANON(optional, name, element) \
const ArrayTemplate array_##name(optional, element);

/* Configuration templates */

FINAL_VALUE_TEMPLATE(false, signature, String)

FINAL_VALUE_TEMPLATE_ANON(false, versionValue, Integer)
ARRAY_TEMPLATE(false, version, (AnyTemplate *) &finalValue_versionValue)

FINAL_VALUE_TEMPLATE(true, stdout, String)
FINAL_VALUE_TEMPLATE(true, level, String)
FINAL_VALUE_TEMPLATE(true, textShader, Boolean)
FINAL_VALUE_TEMPLATE(true, exceptionTextShader, Boolean)
OBJECT_TEMPLATE(true, journal, {
    FINAL_VALUE_MEMBER(stdout)
    FINAL_VALUE_MEMBER(level)
    FINAL_VALUE_MEMBER(textShader)
    FINAL_VALUE_MEMBER(exceptionTextShader)
})

FINAL_VALUE_TEMPLATE(true, useStrictHardwareDraw, Boolean)
FINAL_VALUE_TEMPLATE(true, useGpuDraw, Boolean)
FINAL_VALUE_TEMPLATE(true, enableGpuDebugJournal, Boolean)
OBJECT_TEMPLATE(true, features, {
    FINAL_VALUE_MEMBER(useStrictHardwareDraw)
    FINAL_VALUE_MEMBER(useGpuDraw)
    FINAL_VALUE_MEMBER(enableGpuDebugJournal)
})

OBJECT_TEMPLATE(true, root, {
    FINAL_VALUE_MEMBER(signature)
    ARRAY_MEMBER(version)
    OBJECT_MEMBER(journal)
    OBJECT_MEMBER(features)
})

// ----------------------------------------------------------------------

struct CmdToken
{
    enum class Type
    {
        kLongOption,
        kShortOption,
        kIdentifier
    };
    CmdToken(Type typ, const char *pLexeme)
        : type(typ), lexeme(pLexeme) {}

    Type        type;
    char const *lexeme;
};

struct CmdOption
{
    CmdOption() = default;
    explicit CmdOption(std::string opt)
        : option(std::move(opt)) {}

    std::string  option;
    std::optional<std::string> value;
};

void lexCmdLine(int argc, char const **argv, std::vector<CmdToken>& tokens)
{
    std::regex shortOptionRegex(R"(^-.+$)");
    std::regex longOptionRegex(R"(^--.{2,}$)");

    for (int i = 1; i < argc; i++)
    {
        const char *pStr = argv[i];
        if (std::regex_match(pStr, longOptionRegex))
            tokens.emplace_back(CmdToken::Type::kLongOption, pStr);
        else if (std::regex_match(pStr, shortOptionRegex))
            tokens.emplace_back(CmdToken::Type::kShortOption, pStr);
        else
            tokens.emplace_back(CmdToken::Type::kIdentifier, pStr);
    }
}

CmdOption parseLongOption(const CmdToken& t)
{
    auto len = std::strlen(t.lexeme);
    CmdOption opt;

    bool isValue = false;
    for (std::size_t i = 2; i < len; i++)
    {
        if (t.lexeme[i] == '=' && !isValue)
            isValue = true;
        else if (isValue)
        {
            if (!opt.value.has_value())
                opt.value = std::make_optional<std::string>();
            opt.value.value().push_back(t.lexeme[i]);
        }
        else
            opt.option.push_back(t.lexeme[i]);
    }
    return opt;
}

void parseCmdLine(int argc, char const **argv, std::vector<CmdOption>& opts, std::vector<std::string>& arguments)
{
    std::vector<CmdToken> tokens;
    lexCmdLine(argc, argv, tokens);

    for (auto itr = tokens.begin(); itr != tokens.end(); itr++)
    {
        switch (itr->type)
        {
        case CmdToken::Type::kLongOption:
            opts.push_back(parseLongOption(*itr));
            break;

        case CmdToken::Type::kShortOption:
            {
                std::size_t size = std::strlen(itr->lexeme);
                for (std::size_t i = 1; i < size; i++)
                {
                    char buf[2] = { itr->lexeme[i], '\0' };
                    opts.emplace_back(buf);
                }
                auto next = itr + 1;
                if (next != tokens.end() && next->type == CmdToken::Type::kIdentifier)
                {
                    opts.back().value = std::make_optional<std::string>(next->lexeme);
                    itr++;
                }
            }
            break;

        case CmdToken::Type::kIdentifier:
            arguments.emplace_back(itr->lexeme);
            break;
        }
    }
}

const char helpUsageInfo[] = "Cocoa 2D Graphics Engine version $ver\n"
                             "Usage: $argv0 [options] [script file]\n"
                             "\n"
                             "Options:\n"
                             "  -h, --help                      Displays help information.\n"
                             "  -v, --version                   Displays version information.\n"
                             "  -c, --config=<file>             Specify a JSON file as configuration.\n"
                             "  -o, --override=<expr>           Overwrite a configuration entry.\n"
                             "  --work-directory=<dir>          Specify a working directory.\n";

void printHelp(const char *argv0)
{
    std::string info(helpUsageInfo);
    info.replace(info.find("$argv0"), 6, argv0);
    info.replace(info.find("$ver"), 4, COCOA_VERSION);

    std::cout << info << std::endl;
}

void printVersion()
{
    std::cout << "Cocoa 2D Graphics Engine version " << COCOA_VERSION << std::endl;
    std::cout << COCOA_LICENSE << std::endl;
}

// ------------------------------------------------------------------------------------------

const std::string defaultJSON = R"(
{
  "signature": "CocoaProject::JSONConfiguration",
  "version": [1, 0, 0],
  "journal": {
    "stdout": "<stdout>",
    "level": "debug",
    "textShader": true,
    "exceptionTextShader": true
  },

  "features": {
    "useStrictHardwareDraw": false,
    "useGpuDraw": true,
    "enableGpuDebugJournal": false
  }
})";

} // namespace anonymous

namespace cocoa {

Configurator::Configurator()
{
    if (PropertyTree::Instance()->resolve("/runtime") == nullptr)
    {
        PropertyTreeNode::NewDirNode(PropertyTree::Instance()->resolve("/"),
                                     "runtime");
    }

    fpNode = PropertyTree::Instance()->resolve("/runtime")
            ->cast<PropertyTreeDirNode>();
    PropertyTreeNode::NewDirNode(PropertyTree::Instance()->resolve("/"),
                                 "bootstrap");
}

Configurator::State Configurator::parse(int argc, const char **argv)
{
    if (argc <= 0)
    {
        std::cerr << "Corrupted arguments from command line." << std::endl;
        return State::kError;
    }

    /* Dump command line to property tree */
    auto *pCommandNode = PropertyTreeNode::NewDirNode(fpNode, "command");
    PropertyTreeNode::NewDataNode(pCommandNode, "argc", argc);
    PropertyTreeNode::NewDataNode(pCommandNode, "argv", argv);

    /* Pass 1: Parsing command line, don't process overrides */
    std::vector<CmdOption> options;
    std::vector<std::string> scriptFile;
    try
    {
        parseCmdLine(argc, argv, options, scriptFile);
    }
    catch (const RuntimeException& e)
    {
        std::cerr << e.what() << std::endl;
        return State::kError;
    }

#define require_argument                        \
    if (!opt.value.has_value())                 \
    {                                           \
        std::cerr << "Option " << opt.option    \
                  << " requires an argument" << std::endl; \
        return State::kError;                   \
    }

#define if_opt(olong, oshort) \
    if (opt.option == olong || opt.option == oshort)

#define if_opt_l(olong) \
    if (opt.option == olong)

    for (const auto& opt : options)
    {
        if_opt("help", "h")
        {
            printHelp(argv[0]);
            return State::kShouldExitNormally;
        }
        else if_opt("version", "v")
        {
            printVersion();
            return State::kShouldExitNormally;
        }
        else if_opt("config", "c")
        {
            require_argument
            fJSONFile = opt.value.value();
        }
        else if_opt("override", "o")
        {
            require_argument
            fCmdOverrides.push_back(opt.value.value());
        }
        else if_opt_l("work-directory")
        {
            require_argument
            PropertyTreeNode::NewDataNode(PropertyTree::Instance()->resolve("/bootstrap"),
                                          "pwd", opt.value.value());

            utils::ChangeWorkDirectory(opt.value.value());
        }
        else
        {
            std::cerr << "Unrecognized commandline option " << opt.option << std::endl;
            return State::kError;
        }
    }
#undef if_opt
#undef if_opt_l
#undef require_argument

    if (scriptFile.size() != 1)
    {
        std::cerr << "At least one script file is required" << std::endl;
        return State::kError;
    }
    PropertyTreeNode::NewDataNode(PropertyTree::Instance()->resolve("/bootstrap"),
                                  "scriptFile", scriptFile[0]);

    /* Pass 2: Loading default configuration */
    json::parseString(defaultJSON, &object_root, fpNode);

    /* Pass 3: Load user's configuration if specified */
    if (!fJSONFile.empty())
    {
        try {
            json::parseFile(fJSONFile, &object_root, fpNode);
        } catch (const RuntimeException& e) {
            std::cerr << e.what() << std::endl;
            return State::kError;
        }
    }

    /* TODO: Implement pass 4, parses overrides
             in command which are stored in fCmdOverrides */
    for (const auto& str : fCmdOverrides)
    {
        std::cout << "Override: " << str << std::endl;
    }

    return State::kSuccessful;
}

}
