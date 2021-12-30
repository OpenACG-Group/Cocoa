#ifndef COCOA_RUNTIME_H
#define COCOA_RUNTIME_H

#include <memory>
#include <map>
#include <vector>

#include "include/v8.h"

#include "Core/EventSource.h"
#include "Core/Exception.h"
#include "Koi/binder/Convert.h"
#include "Koi/KoiBase.h"
#include "Koi/ModuleImportURL.h"
#include "Koi/GlobalIsolateGuard.h"
#include "Koi/VMIntrospect.h"
KOI_NS_BEGIN

#define CHECKED(E)  E.ToLocalChecked()

#define ISOLATE_DATA_SLOT_RUNTIME_PTR       0

namespace bindings { class BindingBase; }

class Runtime : public LoopPrologueSource,
                public AsyncSource
{
public:
    struct Options
    {
        Options();

        std::string startup = "index.js";
        uint32_t    v8_platform_thread_pool;
        std::vector<std::string> v8_options;
        std::vector<std::string> bindings_blacklist;
        bool        rt_allow_override = false;
        bool        introspect_allow_loading_shared_object = false;
        bool        introspect_allow_write_journal = false;
        int         introspect_stacktrace_frame_limit = 10;
        bool        rt_expose_introspect = true;
        bool        start_with_inspector = false;
        int32_t     inspector_port = 9005;
        std::string inspector_address = "127.0.0.1";
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
            std::unique_ptr<v8::Platform> platform,
            v8::ArrayBuffer::Allocator *allocator,
            v8::Isolate *isolate,
            v8::Global<v8::Context> context,
            Options opts);
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    ~Runtime() override;

    static std::shared_ptr<Runtime> MakeFromSnapshot(EventLoop *loop,
                                                     const std::string& snapshotFile,
                                                     const std::string& icuDataFile = std::string(),
                                                     const Options& options = Options());

    static Runtime *GetBareFromIsolate(v8::Isolate *isolate);

    koi_nodiscard inline const Options& getOptions() const
    { return fOptions; }

    inline v8::Local<v8::Context> context()
    { return fContext.Get(fIsolate); }

    inline v8::Isolate *isolate()
    { return fIsolate; }

    v8::MaybeLocal<v8::Value> execute(const char *scriptName, const char *str);

    v8::Local<v8::Module> compileModule(const ModuleImportURL::SharedPtr& referer,
                                        const std::string& url,
                                        bool isImport);
    v8::MaybeLocal<v8::Value> evaluateModule(const std::string& url);

    bindings::BindingBase *getSyntheticModuleBinding(v8::Local<v8::Module> module);
    v8::Local<v8::Object> getSyntheticModuleExportObject(v8::Local<v8::Module> module);

    ModuleCacheMap& getModuleCache();

    koi_nodiscard inline const std::unique_ptr<VMIntrospect>& getIntrospect() const {
        return fIntrospect;
    }

    template<typename...ArgsT>
    v8::MaybeLocal<v8::Object> newObjectFromSynthetic(const std::string& import,
                                                      const std::string& ctor,
                                                      ArgsT&&...args)
    {
        v8::EscapableHandleScope scope(fIsolate);
        auto url = ModuleImportURL::Resolve(nullptr, import, ModuleImportURL::ResolvedAs::kSysImport);
        if (!url)
            return {};
        v8::Local<v8::Module> module;
        if (!getAndCacheSyntheticModule(url).ToLocal(&module) || !module->IsSyntheticModule())
            return {};

        v8::Local<v8::Object> exports = getSyntheticModuleExportObject(module);
        CHECK(!exports.IsEmpty());

        v8::Local<v8::String> ctor_key = binder::to_v8(fIsolate, ctor);
        v8::Local<v8::Value> constructorValue;
        if (!exports->Get(this->context(), ctor_key).ToLocal(&constructorValue))
            return {};
        if (!constructorValue->IsFunction())
            return {};
        v8::Local<v8::Function> constructorFunc = v8::Local<v8::Function>::Cast(constructorValue);
        if constexpr(sizeof...(args) == 0)
            return scope.EscapeMaybe(constructorFunc->NewInstance(context()));
        else
        {
            v8::Local<v8::Value> values[] = {binder::to_v8(fIsolate, std::forward<ArgsT>(args))...};
            return scope.EscapeMaybe(constructorFunc->NewInstance(context(), sizeof...(args), values));
        }
        MARK_UNREACHABLE();
    }

private:
    inline void setGlobalIsolateGuard(std::unique_ptr<GlobalIsolateGuard> ptr) {
        fIsolateGuard = std::move(ptr);
    }
    v8::MaybeLocal<v8::Module> getAndCacheSyntheticModule(const ModuleImportURL::SharedPtr& url);
    KeepInLoop loopPrologueDispatch() override;
    void asyncDispatch() override;

    Options                         fOptions;
    std::unique_ptr<v8::Platform>   fPlatform;
    v8::ArrayBuffer::Allocator     *fArrayBufferAllocator;
    v8::Isolate                    *fIsolate;
    std::unique_ptr<GlobalIsolateGuard>
                                    fIsolateGuard;
    v8::Global<v8::Context>         fContext;
    std::unique_ptr<VMIntrospect>   fIntrospect;
    ModuleCacheMap                  fModuleCache;
};

KOI_NS_END
#endif //COCOA_RUNTIME_H
