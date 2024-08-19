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

#include "Gallium/ffi/Module.h"
#include "Gallium/ffi/Context.h"
#include "Gallium/RuntimeBase.h"
#include "Gallium/bindings/typetraits/Module.h"
#include "Gallium/bindings/utils/Module.h"
#include "Gallium/bindings/fs/Module.h"
#include "Gallium/bindings/stream/Module.h"
#include "Gallium/bindings/workers/Module.h"
#include "Gallium/bindings/renderer/Module.h"
#include "Gallium/bindings/pixencoder/Module.h"
#include "Gallium/bindings/present/Module.h"
#include "Gallium/bindings/multimedia/Module.h"
GALLIUM_FFI_NS_BEGIN

/**
 * Define internal native modules here.
 */
#define INTERNAL_MODULES_MAP(V)         \
    V(typetraits, TypetraitsModule)     \
    V(utils, UtilsModule)               \
    V(stream, StreamModule)             \
    V(fs, FsModule)                     \
    V(workers, WorkersModule)           \
    V(renderer, RendererModule)         \
    V(pixencoder, PixencoderModule)     \
    V(present, PresentModule)           \
    V(multimedia, MultimediaModule)

ModuleRegistry::ModuleRegistry()
{
#define REGISTER_MODULE(name, klass) \
    module_map_[#name] = std::make_unique<bindings::klass>();

    INTERNAL_MODULES_MAP(REGISTER_MODULE)
}

ModuleRegistry *ModuleRegistry::FromIsolate(v8::Isolate *isolate)
{
    return TypeContext::FromIsolate(isolate)->GetModuleRegistry();
}

NativeModule *ModuleRegistry::SearchModule(const std::string_view& name)
{
    auto itr = module_map_.find(std::string(name));
    if (itr == module_map_.end())
        return nullptr;
    return itr->second.get();
}

void ModuleRegistry::StoreInstantiated(NativeModule *native_module, Local<v8::Module> v8_module,
                                       const LocalExports& exports)
{
    int hash = v8_module->GetIdentityHash();
    CHECK(instantiated_map_.count(hash) == 0);
    ModuleInfo& info = instantiated_map_[hash];
    info.native_module = native_module;

    Isolate *isolate = Isolate::GetCurrent();
    for (auto [name, value] : exports)
        info.exports[name].Reset(isolate, value);
}

ModuleRegistry::ModuleInfo *ModuleRegistry::FetchModuleInfo(Local<v8::Module> v8_module)
{
    auto itr = instantiated_map_.find(v8_module->GetIdentityHash());
    if (itr == instantiated_map_.end())
        return nullptr;
    return &itr->second;
}

ModuleRegistry::ModuleInfo*
ModuleRegistry::FetchOrInstantiateModuleInfo(v8::Isolate *isolate, const std::string& name)
{
    NativeModule *native = SearchModule(name);
    if (!native)
        return nullptr;
    v8::HandleScope handle_scope(isolate);
    return FetchModuleInfo(native->Instantiate(isolate));
}

namespace {

MaybeLocal<Value> on_native_module_eval(Local<Context> ctx, Local<v8::Module> module)
{
    Isolate *isolate = ctx->GetIsolate();

    // Fetch the module info stored by `NativeModule::Instantiate`
    auto *module_info = ModuleRegistry::FromIsolate(isolate)->FetchModuleInfo(module);
    CHECK(module_info && "ModuleInfo not found");

    for (auto& [name, global_value] : module_info->exports)
    {
        Local<Value> value = global_value.Get(isolate);
        Local<String> str_name = Cast<std::string>::ToChecked(isolate, name);
        if (!module->SetSyntheticModuleExport(isolate, str_name, value).FromMaybe(false))
            // An error was thrown by `SetSyntheticModuleExport`
            return {};
    }

    // V8 requires that a resolved Promise should be returned
    // to indicate success.
    auto resolver = v8::Promise::Resolver::New(ctx).ToLocalChecked();
    resolver->Resolve(ctx, v8::Undefined(isolate)).Check();
    return resolver->GetPromise();
}

} // namespace anonymous

NativeModule::NativeModule(std::string name, std::string description,
                           std::vector<std::string> deps)
    : name_(std::move(name))
    , description_(std::move(description))
    , deps_(std::move(deps))
{
}

Local<v8::Module> NativeModule::Instantiate(v8::Isolate *isolate)
{
    if (!module_.IsEmpty())
        return module_.Get(isolate);

    v8::EscapableHandleScope handle_scope(isolate);
    LocalExports local_exports = OnBuildExports(isolate);

    std::vector<Local<String>> export_names;
    for (auto [name, value] : local_exports)
        export_names.emplace_back(Cast<std::string>::ToChecked(isolate, name));

    v8::MemorySpan<const Local<String>> exports_names_span(
            export_names.begin(), export_names.end());

    auto handle = v8::Module::CreateSyntheticModule(
            isolate, Cast<std::string>::ToChecked(isolate, name_),
            exports_names_span, on_native_module_eval);

    // Store the module into the module registry so that the `on_native_module_eval`
    // callback can access the module info when called.
    ModuleRegistry::FromIsolate(isolate)->StoreInstantiated(this, handle, local_exports);

    // Instantiate the synthetic module, `on_native_module_eval` will be called.
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    CHECK(handle->InstantiateModule(ctx, nullptr).FromMaybe(false));

    module_.Reset(isolate, handle);
    return handle_scope.Escape(handle);
}

void NativeModule::InsertScriptExports(v8::Isolate *isolate, const std::string& url,
                                       LocalExports& target)
{
    RuntimeBase *runtime = RuntimeBase::FromIsolate(isolate);
    v8::Local<v8::Value> func;

    v8::TryCatch try_catch(isolate);
    if (!runtime->ExecuteScript(url, RuntimeBase::kSysInvoke_ScriptSourceFlag).ToLocal(&func))
    {
        runtime->ReportUncaughtExceptionInCallback(try_catch);
        CHECK_FAILED("internal script execution error");
    }
    if (!func->IsFunction())
        CHECK_FAILED("evaluation of internal script must result a function");

    // Current exports are provided to the function, since the script
    // may want to access them.
    v8::Local<v8::Value> current_exports = ffi::Cast<LocalExports>::ToChecked(isolate, target);
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    v8::Local<v8::Value> result;
    if (!func.As<v8::Function>()->Call(ctx, v8::Null(isolate), 1, &current_exports).ToLocal(&result))
        CHECK_FAILED("internal script execution error");

    ffi::Opt<LocalExports> exports = ffi::Cast<LocalExports>::From(isolate, result);
    if (!exports)
    {
        isolate->ThrowError("cannot interpret the result of script evaluation");
        return;
    }

    for (const auto& [name, value] : *exports)
    {
        if (target.find(name) != target.end())
        {
            isolate->ThrowError("export name conflicts");
            return;
        }
        target[name] = value;
    }
}

GALLIUM_FFI_NS_END
