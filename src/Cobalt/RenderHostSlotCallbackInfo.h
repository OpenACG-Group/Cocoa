#ifndef COCOA_COBALT_RENDERHOSTSLOTCALLBACKINFO_H
#define COCOA_COBALT_RENDERHOSTSLOTCALLBACKINFO_H

#include <functional>
#include <any>
#include <vector>

#include "Core/Errors.h"
#include "Cobalt/Cobalt.h"
COBALT_NAMESPACE_BEGIN

class RenderClientSignalEmit;
class RenderClientObject;

class RenderHostSlotCallbackInfo
{
public:
    explicit RenderHostSlotCallbackInfo(RenderClientSignalEmit *emit);
    RenderHostSlotCallbackInfo(const RenderHostSlotCallbackInfo&) = delete;
    ~RenderHostSlotCallbackInfo() = default;

    g_nodiscard co_sp<RenderClientObject> GetEmitter() const;

    template<typename T>
    g_nodiscard g_inline T& Get(size_t index) {
        CHECK(index < args_vector_ref_.size());
        return std::any_cast<T&>(args_vector_ref_[index]);
    }

    g_nodiscard g_inline std::any& Get(size_t index) {
        CHECK(index < args_vector_ref_.size());
        return args_vector_ref_[index];
    }

    g_nodiscard g_inline size_t Length() const {
        return args_vector_ref_.size();
    }

private:
    RenderClientSignalEmit  *client_emit_;
    std::vector<std::any>&   args_vector_ref_;
};

using RenderHostSlotCallback = std::function<void(RenderHostSlotCallbackInfo&)>;

COBALT_NAMESPACE_END
#endif //COCOA_COBALT_RENDERHOSTSLOTCALLBACKINFO_H
