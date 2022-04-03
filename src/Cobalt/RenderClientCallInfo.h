#ifndef COCOA_COBALT_RENDERCLIENTCALLINFO_H
#define COCOA_COBALT_RENDERCLIENTCALLINFO_H

#include <vector>
#include <any>
#include <optional>
#include <exception>

#include "Core/Errors.h"
#include "Cobalt/Cobalt.h"
COBALT_NAMESPACE_BEGIN

class RenderClientObject;

class RenderClientCallInfo
{
public:
    using OpCode = uint32_t;

    enum class Status
    {
        kPending,
        kOpCodeInvalid,
        kArgsInvalid,
        kCaught,
        kOpSuccess,
        kOpFailed
    };

    explicit RenderClientCallInfo(OpCode opcode)
        : op_code_(opcode)
        , return_status_(Status::kPending) {}
    RenderClientCallInfo(const RenderClientCallInfo&) = delete;
    RenderClientCallInfo(RenderClientCallInfo&& rhs) noexcept
            : op_code_(rhs.op_code_)
            , args_vector_(std::move(rhs.args_vector_))
            , return_status_(rhs.return_status_)
            , return_value_(std::move(rhs.return_value_))
            , closure_ptr_(std::move(rhs.closure_ptr_)) {}
    ~RenderClientCallInfo() = default;

    g_nodiscard g_inline OpCode GetOpCode() const {
        return op_code_;
    }

    g_nodiscard g_inline size_t Length() const {
        return args_vector_.size();
    }

    template<typename T>
    g_inline void SetClosure(T&& value) {
        closure_ptr_ = std::forward<T>(value);
    }

    g_nodiscard g_inline std::any& GetClosure() {
        return closure_ptr_;
    }

    template<typename T>
    g_nodiscard g_inline T& Get(size_t index) {
        CHECK(index < args_vector_.size());
        CHECK(args_vector_[index].has_value());
        return std::any_cast<T&>(args_vector_[index]);
    }

    template<typename T>
    g_inline RenderClientCallInfo& PushBack(T&& value) {
        args_vector_.push_back(value);
        return *this;
    }

    /**
     * Append a argument for the invocation.
     * An object of @p T will be constructed with @p args, which means code
     *
     * @code
     * info.EmplaceBack<T>(a1, a2, a3, ...);
     * @endcode
     *
     * is equivalent to
     *
     * @code
     * info.PushBack(T(a1, a2, a3, ...));
     * @endcode
     *
     * but the former won't do copy- or move-construction.
     *
     * All the argument objects will be constructed in the host thread, and no any
     * copy- or move-construction will happen after you emplace or push them into the
     * @p RenderClientCallInfo (if receiver doesn't move or copy them).
     * They will also be destructed in the host thread after host callback is called.
     */
    template<typename T, typename...Args>
    g_inline RenderClientCallInfo& EmplaceBack(Args&&...args) {
        args_vector_.emplace_back(std::in_place_type_t<T>{}, std::forward<Args>(args)...);
        return *this;
    }

    g_inline RenderClientCallInfo& SwallowBack(std::any&& rvalue) {
        args_vector_.emplace_back(std::forward<std::any>(rvalue));
        return *this;
    }

    /* This method only can be called once */
    template<typename T>
    g_inline const T& SetReturnValue(T&& value) {
        CHECK(!return_value_.has_value());
        return_value_ = value;
        return std::any_cast<T&>(return_value_);
    }

    /* This method only can be called once */
    g_inline void SetReturnStatus(Status status) {
        CHECK(return_status_ == Status::kPending);
        CHECK(status != Status::kPending && "Set a pending ReturnStatus is meaningless");
        return_status_ = status;
    }

    g_nodiscard g_inline co_sp<RenderClientObject> GetThis() const {
        return this_;
    }

    g_private_api g_nodiscard g_inline std::any MoveReturnValue() {
        return std::move(return_value_);
    }

    g_private_api g_nodiscard g_inline Status GetReturnStatus() {
        return return_status_;
    }

    g_private_api g_inline void SetThis(const co_sp<RenderClientObject>& pThis) {
        this_ = pThis;
    }

    g_private_api g_inline void SetCaughtException(const std::exception& e) {
        caught_exception_ = e;
    }

    g_private_api g_nodiscard std::exception& GetCaughtException() {
        CHECK(caught_exception_.has_value());
        return caught_exception_.value();
    }

private:
    OpCode                      op_code_;
    std::vector<std::any>       args_vector_;
    Status                      return_status_;
    std::any                    return_value_;
    co_sp<RenderClientObject>   this_;
    std::optional<std::exception> caught_exception_;
    std::any                    closure_ptr_;
};

COBALT_NAMESPACE_END
#endif //COCOA_COBALT_RENDERCLIENTCALLINFO_H
