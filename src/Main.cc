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
#include "Core/CmdParser.h"
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

} // namespace cmd

#define arg_longopt_match(s) (!std::strcmp(arg.matched_template->long_name, s))

cmd::ParseState initialize_logger(cmd::ParseResult& args)
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
            cmd::PrintHelp(argv[0]);
            return cmd::ParseState::kExit;
        }
        else if arg_longopt_match("version")
        {
            startup_print_version();
            return cmd::ParseState::kExit;
        }
    }

    /* Initialize logger */
    state = initialize_logger(args);
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
        else if arg_longopt_match("runtime-inspector")
        {
            gallium_options.start_with_inspector = true;
            if (arg.value)
                gallium_options.inspector_port = arg.value->v_int;
        }
        else if arg_longopt_match("runtime-inspector-no-script")
        {
            gallium_options.inspector_no_script = true;
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

    if (!justInitialize)
    {
        auto runtime = gallium::Runtime::Make(EventLoop::Instance(), options);

        v8::Isolate::Scope isolateScope(runtime->GetIsolate());
        v8::HandleScope handleScope(runtime->GetIsolate());
        v8::Context::Scope contextScope(runtime->GetContext());

        runtime->RunWithMainLoop();

        gallium::BindingManager::Delete();

        runtime->NotifyRuntimeWillExit();
        CHECK(runtime.unique() && "Runtime is referenced by other scopes");
    }
    else
    {
        QLOG(LOG_INFO, "[TESTRUN] Cocoa exits after finishing initialization steps");
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

    try
    {
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
        startup_print_greeting(gallium_options);
        mainloop_execute(only_initialize, gallium_options, glamor_options);
    }
    catch (const RuntimeException& e)
    {
        utils::SerializeException(e);
        return EXIT_FAILURE;
    }
    catch (const std::exception& e)
    {
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
