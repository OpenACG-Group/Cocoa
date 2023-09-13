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

#ifndef COCOA_GALLIUM_RUNTIME_H
#define COCOA_GALLIUM_RUNTIME_H

#include <memory>
#include <map>
#include <vector>

#include "include/v8.h"

#include "Core/Exception.h"
#include "Gallium/RuntimeBase.h"
#include "Gallium/binder/Convert.h"
#include "Gallium/Gallium.h"
#include "Gallium/ModuleImportURL.h"
#include "Gallium/GlobalIsolateGuard.h"
#include "Gallium/VMIntrospect.h"
GALLIUM_NS_BEGIN

class Platform;
class Inspector;

class Runtime : public RuntimeBase
{
public:
    struct Options
    {
        Options();

        std::string startup = "index.js";
        int32_t     v8_platform_thread_pool = 0;
        std::vector<std::string> v8_options;
        std::vector<std::string> bindings_blacklist;
        bool        rt_allow_override = false;
        bool        introspect_allow_loading_shared_object = true;
        bool        introspect_allow_write_journal = false;
        int         introspect_stacktrace_frame_limit = 10;
        bool        rt_expose_introspect = true;
        bool        start_with_inspector = false;
        int32_t     inspector_port = 9005;
        std::string inspector_address = "127.0.0.1";
        bool        inspector_no_script = false;
        bool        inspector_startup_brk = false;
    };

    Runtime(EventLoop *loop, std::shared_ptr<Platform> platform, Options opts);

    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    ~Runtime() override;

    static std::shared_ptr<Runtime> Make(EventLoop *loop, const Options& options);

    static void AdoptV8CommandOptions(const Options& options);

    static Runtime *GetBareFromIsolate(v8::Isolate *isolate);

    g_nodiscard inline const Options& getOptions() const {
        return options_;
    }


    g_nodiscard inline const std::unique_ptr<VMIntrospect>& GetIntrospect() const {
        return introspect_;
    }

    std::unique_ptr<GlobalIsolateGuard>& GetUniqueGlobalIsolateGuard() {
        return isolate_guard_;
    }

    void RunWithMainLoop();

    void NotifyRuntimeWillExit();

private:
    void OnPreDispose() override;
    void OnPostDispose() override;
    void OnInitialize(v8::Isolate *isolate, v8::Local<v8::Context> context) override;
    void OnPostPerformTasksCheckpoint() override;
    void OnReportUncaughtExceptionInCallback(const v8::TryCatch& catch_block) override;

    Options                         options_;
    std::unique_ptr<Inspector>      inspector_;
    std::unique_ptr<GlobalIsolateGuard>
                                    isolate_guard_;
    std::unique_ptr<VMIntrospect>   introspect_;
};

GALLIUM_NS_END
#endif //COCOA_GALLIUM_RUNTIME_H
