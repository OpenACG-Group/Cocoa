#ifndef COCOA_COBALT_RENDERCLIENTEMITTERINFO_H
#define COCOA_COBALT_RENDERCLIENTEMITTERINFO_H

#include <vector>
#include <any>

#include "Core/Errors.h"
#include "Cobalt/Cobalt.h"
COBALT_NAMESPACE_BEGIN

class RenderClientObject;

class RenderClientEmitterInfo
{
public:
    RenderClientEmitterInfo() = default;
    RenderClientEmitterInfo(const RenderClientEmitterInfo&) = delete;
    RenderClientEmitterInfo(RenderClientEmitterInfo&& rhs) noexcept
        : args_vector_(std::move(rhs.args_vector_)) {}
    ~RenderClientEmitterInfo() = default;

    template<typename T, typename...Args>
    g_inline RenderClientEmitterInfo& EmplaceBack(Args&&...args) {
        args_vector_.emplace_back(std::in_place_type_t<T>{}, std::forward<Args>(args)...);
        return *this;
    }

    template<typename T>
    g_inline RenderClientEmitterInfo& PushBack(T&& value) {
        args_vector_.push_back(value);
        return *this;
    }

    g_nodiscard g_inline size_t Length() const {
        return args_vector_.size();
    }

    g_private_api g_nodiscard g_inline std::vector<std::any> MoveArgs() {
        return std::move(args_vector_);
    }

private:
    std::vector<std::any>       args_vector_;
};

COBALT_NAMESPACE_END
#endif //COCOA_COBALT_RENDERCLIENTEMITTERINFO_H
