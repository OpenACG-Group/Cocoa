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

#ifndef COCOA_GALLIUM_RUNTIMEBASE_H
#define COCOA_GALLIUM_RUNTIMEBASE_H

#include <map>
#include <list>

#include "include/v8.h"
#include "uv.h"

#include "Core/EventLoop.h"
#include "Core/GroupedCallbackManager.h"
#include "Gallium/Gallium.h"
#include "Gallium/ModuleImportURL.h"
#include "Gallium/Platform.h"
#include "Gallium/TracingController.h"
#include "Gallium/binder/Function.h"
GALLIUM_NS_BEGIN

#define ISOLATE_DATA_SLOT_RUNTIME_PTR       0

namespace bindings { class BindingBase; }

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

class RuntimeBase
{
public:
    RuntimeBase(uv_loop_t *event_loop, std::shared_ptr<Platform> platform,
                std::string runtime_id);
    virtual ~RuntimeBase();

    static RuntimeBase *FromIsolate(v8::Isolate *i) {
        return reinterpret_cast<RuntimeBase*>(i->GetData(ISOLATE_DATA_SLOT_RUNTIME_PTR));
    }

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

    g_nodiscard g_inline const std::string& GetRuntimeId() const {
        return runtime_id_;
    }

    g_nodiscard g_inline uv_loop_t *GetEventLoop() const {
        return event_loop_;
    }

    g_nodiscard g_inline v8::Isolate *GetIsolate() const {
        CHECK(isolate_);
        return isolate_;
    }

    g_nodiscard g_inline v8::Local<v8::Context> GetContext() const {
        CHECK(isolate_);
        return context_.Get(isolate_);
    }

    g_nodiscard g_inline std::shared_ptr<Platform> GetPlatform() const {
        return platform_;
    }

    g_nodiscard g_inline TracingController *GetTracingController() const {
        // RTTI is disabled, and we can make sure the type of
        // tracing controller, so `static_cast` is used instead
        // of `dynamic_cast`.

        // NOLINTNEXTLINE
        return static_cast<TracingController*>(platform_->GetTracingController());
    }

    void Dispose();
    void Initialize();

    g_nodiscard bindings::BindingBase *GetSyntheticModuleBinding(v8::Local<v8::Module> module);

    g_nodiscard g_inline ModuleCacheMap& GetModuleCache() {
        return module_cache_;
    }

    /**
     * Some synthetic modules depends on other synthetic modules.
     * For example, synthetic module A has an exported class `T`,
     * and another synthetic module B also has an exported class `R`
     * which inherits `T`. When module B is imported by user's JavaScript
     * before A is imported, an error will occur. That's because class `R`
     * inherits class `T`, but `T` has not been registered when registering
     * `R`, making binder cannot find type information of `T`.
     *
     * To solve this tough dependency problem, B can import A explicitly
     * by calling this method when it is imported.
     */
    v8::MaybeLocal<v8::Module> GetAndCacheSyntheticModule(const ModuleImportURL::SharedPtr& url);

    enum ScriptSourceFlags {
        kFromImport_ScriptSourceFlag = 0x01,
        kSysInvoke_ScriptSourceFlag = 0x02
    };
    v8::MaybeLocal<v8::Module> CompileModule(const ModuleImportURL::SharedPtr& referer,
                                             const std::string& url,
                                             int script_source_flags);

    v8::MaybeLocal<v8::Value> EvaluateModule(const std::string& url,
                                             v8::Local<v8::Module> *out = nullptr,
                                             const std::shared_ptr<ModuleImportURL>& referer = nullptr,
                                             int script_source_flags = 0);

    v8::MaybeLocal<v8::Value> ExecuteScript(const char *script_name, const char *source);

    void PerformTasksCheckpoint();

    // Binder's memory management
    using BinderExtValueHolderBase = binder::detail::external_data::value_holder_base;
    g_private_api void RegisterExternalValueHolder(BinderExtValueHolderBase *value);
    g_private_api void UnregisterExternalValueHolder(BinderExtValueHolderBase *value);
    g_private_api void DeleteExternalValueHolders();

    void ReportUncaughtExceptionInCallback(const v8::TryCatch& catch_block);

    void SpinRun();

    enum class ExternalCallbackType
    {
        kBeforeSpinRunExit,
        kBeforeRuntimeDispose,
        kAfterRuntimeDispose,
        kAfterRuntimeInitialize,
        kAfterTasksCheckpoint
    };

    using ExternalCallbackAfterCall = GroupedCallbackManager<ExternalCallbackType>::AfterCallBehaviour;

    uint64_t AddExternalCallback(ExternalCallbackType type, std::function<ExternalCallbackAfterCall(void)> func);
    void RemoveExternalCallback(ExternalCallbackType type, uint64_t id);

protected:
    virtual void OnPreDispose() {}
    virtual void OnPostDispose() {}
    virtual void OnPostPerformTasksCheckpoint() {}
    virtual void OnReportUncaughtExceptionInCallback(const v8::TryCatch&) {}
    virtual void OnInitialize(v8::Isolate *isolate, v8::Local<v8::Context> context) {}

private:
    void PerformIdleEventCheckpoint();

    static void promise_hook(v8::PromiseHookType type,
                             v8::Local<v8::Promise> promise,
                             v8::Local<v8::Value> parent);

    std::string                  runtime_id_;
    bool                         disposed_;
    uv_loop_t                   *event_loop_;
    std::shared_ptr<Platform>    platform_;
    v8::Isolate                 *isolate_;
    v8::Global<v8::Context>      context_;

    ModuleCacheMap               module_cache_;

    uv::CheckHandle              event_check_;
    uv::PrepareHandle            event_prepare_;
    uv::IdleHandle               event_idle_;
    uint64_t                     nb_pending_resolved_promises_;

    std::list<BinderExtValueHolderBase*> binder_external_value_holders_;

    GroupedCallbackManager<ExternalCallbackType> external_callbacks_;
};

GALLIUM_NS_END
#endif //COCOA_GALLIUM_RUNTIMEBASE_H
