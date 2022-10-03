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

#include "Core/EventSource.h"
#include "Core/Exception.h"
#include "Gallium/binder/Convert.h"
#include "Gallium/binder/Function.h"
#include "Gallium/Gallium.h"
#include "Gallium/ModuleImportURL.h"
#include "Gallium/GlobalIsolateGuard.h"
#include "Gallium/VMIntrospect.h"
GALLIUM_NS_BEGIN

#define ISOLATE_DATA_SLOT_RUNTIME_PTR       0

/**
 * `JS_THROW_IF` macro throws a JavaScript exception and make current function
 * return. It does NOT throw a C++ exception which can be caught by `try`
 * statement. After we have returned to the JavaScript land, V8 will handle
 * the exception correctly.
 *
 * For the native callbacks in synthetic module (language binding), use `g_throw`
 * macro provided in the `binder/ThrowExcept.h` to throw a C++ exception which contains
 * a JavaScript exception. Binder will catch the thrown exceptions and convert them
 * to a regular JavaScript exception.
 */
#define JS_THROW_IF(cond, msg, ...)                                                     \
    do {                                                                                \
        if ((cond)) {                                                                   \
            binder::throw_(v8::Isolate::GetCurrent(), msg __VA_OPT__(,) __VA_ARGS__);   \
            return;                                                                     \
        }                                                                               \
    } while (false)

#define JS_THROW_RET_IF(cond, msg, ret, ...)                                            \
    do {                                                                                \
        if ((cond)) {                                                                   \
            binder::throw_(v8::Isolate::GetCurrent(), msg __VA_OPT__(,) __VA_ARGS__);   \
            return ret;                                                                 \
        }                                                                               \
    } while (false)

namespace bindings { class BindingBase; }

class Platform;
class Inspector;

class Runtime : public PrepareSource,
                public CheckSource
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
    };

    struct ESModuleCache
    {
        ESModuleCache() = default;
        ESModuleCache(v8::Isolate *isolate, v8::Local<v8::Module> module)
            : module(isolate, module)
            , binding(nullptr) {}
        ESModuleCache(v8::Isolate *isolate, v8::Local<v8::Module> module,
                      bindings::BindingBase *pBinding)
            : module(isolate, module)
            , binding(pBinding) {}
        ESModuleCache(ESModuleCache&& rhs) noexcept
            : module(std::move(rhs.module))
            , exports(std::move(rhs.exports))
            , binding(rhs.binding) {
            rhs.binding = nullptr;
        }
        ESModuleCache(const ESModuleCache&) = delete;
        ESModuleCache& operator=(ESModuleCache&& rhs) noexcept {
            module = std::move(rhs.module);
            exports = std::move(rhs.exports);
            binding = rhs.binding;
            rhs.binding = nullptr;
            return *this;
        }
        ESModuleCache& operator=(const ESModuleCache&) = delete;

        inline void reset() {
            module.Reset();
            exports.Reset();
            binding = nullptr;
        }

        inline void setExportsObject(v8::Isolate *isolate, v8::Local<v8::Object> obj) {
            exports.Reset(isolate, obj);
        }

        v8::Global<v8::Module> module;
        v8::Global<v8::Object> exports;
        bindings::BindingBase *binding = nullptr;
    };

    using ModuleCacheMap = std::map<ModuleImportURL::SharedPtr, ESModuleCache>;

    Runtime(EventLoop *loop,
            v8::StartupData *startupData,
            std::unique_ptr<v8::TracingController> tracing_controller,
            std::unique_ptr<Platform> platform,
            v8::ArrayBuffer::Allocator *allocator,
            v8::Isolate *isolate,
            v8::Global<v8::Context> context,
            std::unique_ptr<Inspector> inspector,
            Options opts);
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    ~Runtime() override;

    static std::shared_ptr<Runtime> Make(EventLoop *loop, const Options& options);

    static void AdoptV8CommandOptions(const Options& options);

    static Runtime *GetBareFromIsolate(v8::Isolate *isolate);

    g_nodiscard inline const Options& getOptions() const
    { return options_; }

    inline v8::Local<v8::Context> GetContext()
    { return context_.Get(isolate_); }

    inline v8::Isolate *GetIsolate()
    { return isolate_; }

    v8::MaybeLocal<v8::Value> ExecuteScript(const char *scriptName, const char *str);

    v8::Local<v8::Module> CompileModule(const ModuleImportURL::SharedPtr& referer,
                                        const std::string& url,
                                        bool isImport,
                                        bool isSysInvoke);
    v8::MaybeLocal<v8::Value> EvaluateModule(const std::string& url,
                                             v8::Local<v8::Module> *outModule = nullptr,
                                             const std::shared_ptr<ModuleImportURL> &referer = nullptr,
                                             bool isImport = false,
                                             bool isSysInvoke = false);

    bindings::BindingBase *GetSyntheticModuleBinding(v8::Local<v8::Module> module);

    // TODO(sora): deprecate this.
    v8::Local<v8::Object> getSyntheticModuleExportObject(v8::Local<v8::Module> module);

    ModuleCacheMap& GetModuleCache();

    g_nodiscard inline const std::unique_ptr<VMIntrospect>& GetIntrospect() const {
        return introspect_;
    }

    std::unique_ptr<GlobalIsolateGuard>& GetUniqueGlobalIsolateGuard() {
        return isolate_guard_;
    }

    void RunWithMainLoop();

    void PerformTasksCheckpoint();

    void ReportUncaughtExceptionInCallback(const v8::TryCatch& catchBlock);

    void NotifyRuntimeWillExit();

    void DrainPlatformTasks();

    // Binder's memory management
    using BinderExtValueHolderBase = binder::detail::external_data::value_holder_base;
    g_private_api void RegisterExternalValueHolder(BinderExtValueHolderBase *value);
    g_private_api void UnregisterExternalValueHolder(BinderExtValueHolderBase *value);
    g_private_api void DeleteExternalValueHolders();

private:
    static void PromiseHookCallback(v8::PromiseHookType type, v8::Local<v8::Promise> promise,
                                    v8::Local<v8::Value> parent);

    static v8::MaybeLocal<v8::Promise> DynamicImportHostCallback(v8::Local<v8::Context> context,
                                                                 v8::Local<v8::Data> host_defined_options,
                                                                 v8::Local<v8::Value> resource_name,
                                                                 v8::Local<v8::String> specifier,
                                                                 v8::Local<v8::FixedArray> import_assertions);

    inline void SetGlobalIsolateGuard(std::unique_ptr<GlobalIsolateGuard> ptr) {
        isolate_guard_ = std::move(ptr);
    }
    v8::MaybeLocal<v8::Module> GetAndCacheSyntheticModule(const ModuleImportURL::SharedPtr& url);

    void UpdateIdleRequirementByPromiseCounter();

    KeepInLoop prepareDispatch() override;
    KeepInLoop checkDispatch() override;

    Options                         options_;
    v8::StartupData                *startup_data_;
    std::unique_ptr<v8::TracingController> tracing_controller_;
    std::unique_ptr<Platform>       platform_;
    v8::ArrayBuffer::Allocator     *array_buffer_allocator_;
    v8::Isolate                    *isolate_;
    std::unique_ptr<Inspector>      inspector_;
    std::unique_ptr<GlobalIsolateGuard>
                                    isolate_guard_;
    v8::Global<v8::Context>         context_;
    std::unique_ptr<VMIntrospect>   introspect_;
    ModuleCacheMap                  module_cache_;
    int32_t                         resolved_promises_;
    uv_idle_t                       idle_;

    std::list<BinderExtValueHolderBase*>
                                    binder_external_value_holders_;
};

GALLIUM_NS_END
#endif //COCOA_GALLIUM_RUNTIME_H
