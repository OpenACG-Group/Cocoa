#include "fmt/format.h"

#include "Reactor/GShaderExternals.h"
REACTOR_NAMESPACE_BEGIN

#undef V
#define V(x)    reinterpret_cast<void*>((x))

#define GSHADER_API         [[gnu::visibility("default")]]

GSHADER_API int32_t builtin_check_host_context(GShaderModule::HostContext *ptr)
{
    if (ptr == nullptr)
        return external::START_USER_RET_FAILED;
    if (ptr->magic_number != external::HOST_CTX_MAGIC_NUMBER)
        return external::START_USER_RET_FAILED;
    return external::START_USER_RET_NORMAL;
}

GSHADER_API void builtin_v8_trampoline(GShaderModule::HostContext *ptr, uint32_t method_id)
{
    if (ptr->v8_method_map.count(method_id) == 0)
        return;

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    v8::Local<v8::Function> func = ptr->v8_method_map[method_id].Get(isolate);
    func->Call(isolate->GetCurrentContext(),
               isolate->GetCurrentContext()->Global(), 0, nullptr).ToLocalChecked();
}

const llvm::StringMap<void *>& GetExternalSymbolMap()
{
    static llvm::StringMap<void *> map = [] {
        llvm::StringMap<void *> r;

        r.try_emplace("sin", V(sinf));
        r.try_emplace("cos", V(cosf));
        r.try_emplace("tan", V(tanf));
        r.try_emplace("__builtin_v8_trampoline", V(builtin_v8_trampoline));
        r.try_emplace("__builtin_check_host_context", V(builtin_check_host_context));

        return r;
    }();

    return map;
}

REACTOR_NAMESPACE_END
