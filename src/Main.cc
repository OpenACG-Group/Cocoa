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

#include "Core/Project.h"
#include "Core/CmdParser.h"
#include "Core/Errors.h"
#include "Core/Utils.h"
#include "Core/Journal.h"
#include "Core/Exception.h"
#include "Core/EventLoop.h"
#include "Core/Filesystem.h"
#include "Core/ProcessSignalHandler.h"
#include "Core/ApplicationInfo.h"
#include "Core/TraceEvent.h"
#include "Gallium/Runtime.h"
#include "Gallium/BindingManager.h"
#include "Glamor/Glamor.h"
#include "Utau/Utau.h"

#include "CRPKG/ResourceManager.h"

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Main)

PERFETTO_TRACK_EVENT_STATIC_STORAGE();

namespace cocoa {

#define arg_longopt_match(s) (!std::strcmp(arg.matched_template->long_name, s))

cmd::ParseState initialize_logger(cmd::ParseResult& args)
{
    const char *file = nullptr;
    LogLevel level = LOG_LEVEL_QUIET;
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
                fmt::print(stderr, "Illegal specifier for log level: {}\n", arg.value->v_str);
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

std::vector<std::string> string_view_vec_dup(const std::vector<std::string_view>& svv)
{
    std::vector<std::string> sv;
    sv.reserve(svv.size());
    for (const auto& s : svv)
        sv.emplace_back(s);
    return sv;
}

cmd::ParseState startup_initialize(int argc, char const **argv,
                                   gallium::Runtime::Options& gallium_options,
                                   gl::ContextOptions& glamor_options,
                                   utau::ContextOptions& utau_options)
{
    TRACE_EVENT("main", "cocoa::startup_initialize");

    cmd::ParseResult args;
    cmd::ParseState state = cmd::Parse(argc, argv, args);
    if (state == cmd::ParseState::kError)
        return cmd::ParseState::kError;

    if (args.orphans.size() > 1)
    {
        fmt::print(stderr, "Too many arguments\n");
        return cmd::ParseState::kError;
    }

    // We must change working directory before call `ApplicationInfo::Setup()`
    // to make sure `ApplicationInfo` get user-specified working directory.
    for (const auto& arg : args.options)
    {
        if arg_longopt_match("working-dir")
        {
            int ret = vfs::Chdir(arg.value->v_str);
            if (ret < 0)
            {
                fmt::print(stderr, "Failed to chdir: {}", strerror(ret));
                return cmd::ParseState::kError;
            }
            break;
        }
    }

    // Application runtime environment, including important
    // directories (path table) and global parameters.
    if (!ApplicationInfo::Setup())
        return cmd::ParseState::kError;
    ApplicationInfo *app_env = ApplicationInfo::Instance();

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

    char delimiter = ',';

    bool init_only = false;

    for (const auto& arg : args.options)
    {
        if arg_longopt_match("disable-traceback-symbol-folding")
        {
            // TODO: implement this.
        }
        else if arg_longopt_match("initialize-only")
        {
            init_only = true;
        }
        else if arg_longopt_match("v8-concurrent-workers")
        {
            if (arg.value->v_int < 0)
            {
                fmt::print(stderr, "--v8-concurrent-workers should ba a positive integer\n");
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
        else if arg_longopt_match("runtime-inspector-startup-brk")
        {
            gallium_options.inspector_startup_brk = true;
        }
        else if arg_longopt_match("runtime-inspector-initial-brk")
        {
            // TODO(sora): implement this
        }
        else if arg_longopt_match("runtime-blacklist")
        {
            std::vector<std::string_view> list = utils::SplitString(arg.value->v_str, ',');
            for (const auto& p : list)
            {
                gallium_options.bindings_blacklist.emplace_back(p);
                app_env->js_native_preloads.emplace_back(p);
            }
        }
        else if arg_longopt_match("runtime-preload")
        {
            app_env->js_native_preloads.push_back(arg.value->v_str);
        }
        else if arg_longopt_match("runtime-allow-override")
        {
            gallium_options.rt_allow_override = true;
            report_vulnerability_option("--runtime-allow-override");
        }
        else if arg_longopt_match("pass")
        {
            std::vector<std::string_view> svv = utils::SplitString(arg.value->v_str, delimiter);
            for (const auto& s : svv)
                app_env->js_arguments.emplace_back(s);
        }
        else if arg_longopt_match("pass-delimiter")
        {
            if (arg.value->v_str.size() > 1)
            {
                fmt::print(stderr, "Delimiter must be a single character\n");
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
                    fmt::print(stderr, "Error: Unrecognized introspect policy: {}\n", policy);
                    return cmd::ParseState::kError;
                }
            }
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
            glamor_options.SetDisableHWCompose(true);
        }
        else if arg_longopt_match("gl-hwcompose-enable-vkdbg")
        {
            glamor_options.SetEnableVkDBG(true);
        }
        else if arg_longopt_match("gl-hwcompose-vkdbg-severities")
        {
            glamor_options.SetVkDBGFilterSeverities(string_view_vec_dup(
                    utils::SplitString(arg.value->v_str, ',')));
        }
        else if arg_longopt_match("gl-hwcompose-vkdbg-levels")
        {
            glamor_options.SetVkDBGFilterLevels(string_view_vec_dup(
                    utils::SplitString(arg.value->v_str, ',')));
        }
        else if arg_longopt_match("gl-transfer-queue-profile")
        {
            glamor_options.SetProfileRenderHostTransfer(true);
        }
        else if arg_longopt_match("gl-enable-profiler")
        {
            glamor_options.SetEnableProfiler(true);
        }
        else if arg_longopt_match("gl-profiler-ringbuffer-threshold")
        {
            size_t v = arg.value->v_int;
            glamor_options.SetProfilerRingBufferThreshold(v);
        }
        else if arg_longopt_match("gl-hwcompose-disable-presentation")
        {
            glamor_options.SetDisableHWComposePresent(true);
        }
        else if arg_longopt_match("utau-hwdevice-drm-devicepath")
        {
            utau_options.hwdevice_drm_device_path = arg.value->v_str;
        }
        else if arg_longopt_match("utau-filtergraph-max-threads")
        {
            utau_options.filtergraph_max_threads = arg.value->v_int;
            if (utau_options.filtergraph_max_threads < 0)
            {
                fmt::print(stderr, "Error: Option --utau-filtergraph-max-threads has an invalid value");
                return cmd::ParseState::kError;
            }
        }
    }

    if (!gallium_options.inspector_no_script)
    {
        if (args.orphans.empty())
        {
            fmt::print(stderr, "Requires a JavaScript file to run.\n");
            return cmd::ParseState::kError;
        }
        gallium_options.startup = args.orphans[0];
    }

    return init_only ? cmd::ParseState::kJustInitialize : cmd::ParseState::kSuccess;
}

#undef arg_longopt_match

void mainloop_execute(bool justInitialize,
                      const gallium::Runtime::Options& options,
                      const gl::ContextOptions& gl_options,
                      const utau::ContextOptions& utau_options)
{
    EventLoop::New();
    crpkg::ResourceManager::New();

    // Initialize Glamor (rendering engine)
    gl::GlobalScope::New(gl_options, EventLoop::GetCurrent());

    // Initialize Utau (multimedia processing engine)
    utau::InitializePlatform(utau_options);

    // Initialize binding manager
    gallium::BindingManager::New(options);

    for (const auto& lib : ApplicationInfo::Ref().js_native_preloads)
    {
        gallium::BindingManager::Ref().loadDynamicObject(lib);
    }

    if (!justInitialize)
    {
        auto runtime = gallium::Runtime::Make(EventLoop::GetCurrent(), options);

        ScopeExitAutoInvoker disposer([&runtime] {
            runtime->Dispose();
        });

        {
            v8::Isolate::Scope isolateScope(runtime->GetIsolate());
            v8::HandleScope handleScope(runtime->GetIsolate());
            v8::Context::Scope contextScope(runtime->GetContext());

            runtime->RunWithMainLoop();
            runtime->NotifyRuntimeWillExit();
        }

        disposer.cancel();

        // Language bindings have objects which is referenced by JavaScript,
        // and disposing `Runtime` object makes all those objects collected (deleted)
        // to avoid memory leaking. Therefore, it is necessary to dispose the
        // `Runtime` object before deleting the binding manager.
        runtime->Dispose();

        gallium::BindingManager::Delete();
        CHECK(runtime.unique() && "Runtime is referenced by other scopes");
    }
    else
    {
        QLOG(LOG_INFO, "[TESTRUN] Cocoa exits after finishing initialization steps");
    }


    // No matter whether these UniquePersistent objects are created,
    // deleting them is safe.
    utau::DisposePlatform();
    gl::GlobalScope::Delete();

    // RenderHost message queue profiler may register a threadpool work.
    // To make sure the task performed properly, we run event loop again.
    EventLoop::GetCurrent()->run();

    crpkg::ResourceManager::Delete();
    EventLoop::Delete();
}

int startup_main(int argc, char const **argv)
{
    InstallPrimarySignalHandler();

    {
        perfetto::TracingInitArgs args;
        args.backends |= perfetto::kInProcessBackend;
        perfetto::Tracing::Initialize(args);
        perfetto::TrackEvent::Register();
    }

    ScopeExitAutoInvoker epilogue([]() -> void {
        ApplicationInfo::Delete();
        Journal::Delete();
    });

    gallium::Runtime::Options rt_options;
    gl::ContextOptions gl_options;
    utau::ContextOptions utau_options;
    bool only_initialize = false;

    try
    {
        switch (startup_initialize(argc, argv, rt_options, gl_options, utau_options))
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

        gallium::Runtime::AdoptV8CommandOptions(rt_options);
        startup_print_greeting(rt_options);
        mainloop_execute(only_initialize, rt_options, gl_options, utau_options);
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
