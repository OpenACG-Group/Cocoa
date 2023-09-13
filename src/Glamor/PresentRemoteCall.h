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

#ifndef COCOA_GLAMOR_PRESENTREMOTECALL_H
#define COCOA_GLAMOR_PRESENTREMOTECALL_H

#include <vector>
#include <any>
#include <optional>
#include <exception>

#include "Core/Errors.h"
#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

class PresentRemoteHandle;

class PresentRemoteCall
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

    explicit PresentRemoteCall(OpCode opcode)
        : op_code_(opcode)
        , return_status_(Status::kPending) {}
    PresentRemoteCall(const PresentRemoteCall&) = delete;
    PresentRemoteCall(PresentRemoteCall&& rhs) noexcept
            : op_code_(rhs.op_code_)
            , args_vector_(std::move(rhs.args_vector_))
            , return_status_(rhs.return_status_)
            , return_value_(std::move(rhs.return_value_))
            , closure_ptr_(std::move(rhs.closure_ptr_)) {}
    ~PresentRemoteCall() = default;

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
    g_nodiscard g_inline const T& GetConst(size_t index) {
        CHECK(index < args_vector_.size());
        CHECK(args_vector_[index].has_value());
        return std::any_cast<const T&>(args_vector_[index]);
    }

    template<typename T>
    g_inline PresentRemoteCall& PushBack(T&& value) {
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
     * @p PresentRemoteCall (if receiver doesn't move or copy them).
     * They will also be destructed in the host thread after host callback is called.
     */
    template<typename T, typename...Args>
    g_inline PresentRemoteCall& EmplaceBack(Args&&...args) {
        args_vector_.emplace_back(std::in_place_type_t<T>{}, std::forward<Args>(args)...);
        return *this;
    }

    g_inline PresentRemoteCall& SwallowBack(std::any&& rvalue) {
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

    g_inline const std::any& SetReturnValueAny(std::any&& value) {
        return_value_ = std::move(value);
        return return_value_;
    }

    /* This method only can be called once */
    g_inline void SetReturnStatus(Status status) {
        CHECK(return_status_ == Status::kPending);
        CHECK(status != Status::kPending && "Set a pending ReturnStatus is meaningless");
        return_status_ = status;
    }

    g_nodiscard g_inline Shared<PresentRemoteHandle> GetThis() const {
        return this_;
    }

    g_private_api g_nodiscard g_inline std::any MoveReturnValue() {
        return std::move(return_value_);
    }

    g_private_api g_nodiscard g_inline Status GetReturnStatus() {
        return return_status_;
    }

    g_private_api g_inline void SetThis(const Shared<PresentRemoteHandle>& pThis) {
        this_ = pThis;
    }

    g_private_api g_inline void SetCaughtException(const std::string& what) {
        caught_exception_ = what;
    }

    g_private_api g_nodiscard const std::string& GetCaughtException() const {
        CHECK(caught_exception_.has_value());
        return caught_exception_.value();
    }

private:
    OpCode                      op_code_;
    std::vector<std::any>       args_vector_;
    Status                      return_status_;
    std::any                    return_value_;
    Shared<PresentRemoteHandle>   this_;
    std::optional<std::string>  caught_exception_;
    std::any                    closure_ptr_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_PRESENTREMOTECALL_H
