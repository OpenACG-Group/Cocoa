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

#ifndef COCOA_GALLIUM_FFI_RETURNVALUE_H
#define COCOA_GALLIUM_FFI_RETURNVALUE_H

#include <optional>
#include <utility>

#include "include/v8.h"
#include "Core/Errors.h"
#include "Gallium/Gallium.h"
#include "Gallium/ffi/DataTypes.h"
GALLIUM_FFI_NS_BEGIN

enum class ErrorType
{
    kError,
    kRangeError,
    kTypeError,
    kReferenceError,
    kSyntaxError,
    kWasmCompileError,
    kWasmLinkError,
    kWasmRuntimeError,

    // Internal flag, never use it.
    kFreePropagate
};

// Shorthands
constexpr ErrorType kErr = ErrorType::kError;
constexpr ErrorType kRangeErr = ErrorType::kRangeError;
constexpr ErrorType kTypeErr = ErrorType::kTypeError;
constexpr ErrorType kRefErr = ErrorType::kReferenceError;
constexpr ErrorType kSyntaxErr = ErrorType::kSyntaxError;

class ReturnValueBase
{
public:
    enum class State { kOk, kError };

    struct Error {
        void Throw(Isolate *isolate) const;
        ErrorType   type;
        std::string message;
    };
    ~ReturnValueBase() = default;

    g_nodiscard bool HasError() const {
        return (state_ == State::kError);
    }

    g_nodiscard const Error& GetError() const {
        CHECK(state_ == State::kError);
        return error_;
    }

protected:
    explicit ReturnValueBase(State state, Error error = {})
            : state_(state), error_(std::move(error)) {}

    State   state_;
    Error   error_;
};

#define RV_BASE_OK_CTOR   ReturnValueBase(State::kOk, {})
#define RV_BASE_ERR_CTOR  ReturnValueBase(State::kError, std::move(err))

/**
 * Wraps the return value of a function.
 * The `Return<T>` object contains either the actual return value (OK state),
 * or a JS exception that will be thrown later (Error state).
 *
 * This is the generic version of `Return<T>`, which must not used as the return
 * value of wrapped functions. Because it does has an `AssignTo()` method, making
 * Cocoa not know how to convert it into a JS value. Only those specialized versions
 * that have the `AssignTo()` method can be used as the return value of wrapped
 * functions.
 */
template<typename T, typename Enable = void>
class Return : public ReturnValueBase
{
public:
    Return(T&& value) : RV_BASE_OK_CTOR {
        value_.emplace(std::forward<T>(value));
    }
    Return(Error err) : RV_BASE_ERR_CTOR {}

    g_nodiscard T& Extract() {
        CHECK(!HasError() && value_.has_value());
        return *value_;
    }

private:
    Opt<T> value_;
};

template<typename T>
class Return<T, typename std::enable_if<std::is_integral_v<T>>::type>
        : public ReturnValueBase
{
public:
    Return(T value) : RV_BASE_OK_CTOR, value_(value) {}
    Return(Error err) : RV_BASE_ERR_CTOR, value_(0) {}

    g_nodiscard T Extract() const {
        CHECK(!HasError());
        return value_;
    }

    void AssignTo(v8::ReturnValue<Value> ret_value) const {
        CHECK(!HasError());
        if constexpr (sizeof(T) * CHAR_BIT <= 32) {
            if (std::is_signed_v<T>) {
                ret_value.Set(static_cast<int32_t>(value_));
            } else {
                ret_value.Set(static_cast<uint32_t>(value_));
            }
        } else {
            // TODO(sora): Check if the conversion from `T` to `double` is lossless
            ret_value.Set(static_cast<double>(value_));
        }
    }

private:
    T       value_;
};

template<typename T>
class Return<T, typename std::enable_if<std::is_floating_point_v<T>>::type>
        : public ReturnValueBase
{
public:
    Return(T value) : RV_BASE_OK_CTOR, value_(value) {}
    Return(Error err) : RV_BASE_ERR_CTOR, value_(0) {}

    g_nodiscard T Extract() const {
        CHECK(!HasError());
        return value_;
    }

    void AssignTo(v8::ReturnValue<Value> ret_value) const {
        CHECK(!HasError());
        ret_value.Set(static_cast<double>(value_));
    }

private:
    T value_;
};

template<typename Str>
class Return<Str, typename std::enable_if<is_string<Str>::value>::type>
        : public ReturnValueBase
{
public:
    Return(Str str) : RV_BASE_OK_CTOR, value_(std::move(str)) {}
    Return(Error err) : RV_BASE_ERR_CTOR, value_() {}

    g_nodiscard const Str& Extract() const {
        CHECK(!HasError());
        return value_;
    }

    void AssignTo(v8::ReturnValue<Value> ret_value) const {
        CHECK(!HasError());
        ret_value.Set(ffi::Cast<Str>::ToChecked(ret_value.GetIsolate(), value_));
    }

private:
    Str value_;
};

template<>
class Return<PreciseI64> : public ReturnValueBase
{
public:
    Return(int64_t i64value) : RV_BASE_OK_CTOR, value_(i64value) {}
    Return(Error err) : RV_BASE_ERR_CTOR, value_(0) {}

    g_nodiscard const int64_t& Extract() const {
        CHECK(!HasError());
        return value_;
    }

    void AssignTo(v8::ReturnValue<Value> ret_value) const {
        CHECK(!HasError());
        ret_value.Set(v8::BigInt::New(ret_value.GetIsolate(), value_));
    }

private:
    int64_t value_;
};

template<>
class Return<PreciseU64> : public ReturnValueBase
{
public:
    Return(uint64_t i64value) : RV_BASE_OK_CTOR, value_(i64value) {}
    Return(Error err) : RV_BASE_ERR_CTOR, value_(0) {}

    g_nodiscard const uint64_t& Extract() const {
        CHECK(!HasError());
        return value_;
    }

    void AssignTo(v8::ReturnValue<Value> ret_value) const {
        CHECK(!HasError());
        ret_value.Set(v8::BigInt::NewFromUnsigned(ret_value.GetIsolate(), value_));
    }

private:
    uint64_t value_;
};

/**
 * Specialization for `void` type of `Return<T>` class. `Return<void>` only keeps
 * the error state, without any other data payload.
 * `Return<void>::CastValue()` always returns `undefined`.
 */
template<>
class Return<void> : public ReturnValueBase
{
public:
    Return() : RV_BASE_OK_CTOR {}
    Return(Error err) : RV_BASE_ERR_CTOR {}

    // Just a placeholder
    void AssignTo(v8::ReturnValue<Value>) const {}
};

template<>
class Return<bool> : public ReturnValueBase
{
public:
    Return(bool value) : RV_BASE_OK_CTOR, value_(value) {}
    Return(Error err) : RV_BASE_ERR_CTOR, value_(false) {}

    g_nodiscard bool Extract() const {
        CHECK(!HasError());
        return value_;
    }

    void AssignTo(v8::ReturnValue<Value> ret_value) const {
        CHECK(!HasError());
        ret_value.Set(value_);
    }

private:
    bool value_;
};

template<typename...Ts>
class Return<std::tuple<Ts...>> : public ReturnValueBase
{
public:
    using Tuple = std::tuple<Ts...>;

    Return(Tuple&& tuple) : RV_BASE_OK_CTOR, tuple_(std::forward<Tuple>(tuple)) {}
    Return(Error err) : RV_BASE_ERR_CTOR {}

    g_nodiscard Tuple& Extract() const {
        return tuple_;
    }

    void AssignTo(v8::ReturnValue<Value> ret_value) const {
        CHECK(!HasError());
        ret_value.Set(Cast<Tuple>::ToChecked(ret_value.GetIsolate(), tuple_));
    }

private:
    Tuple tuple_;
};


/**
 * Specialization for v8 handles of `Return<T>` class.
 * This specialization optimizes for `Local<T>` handles, avoid using `std::optional`
 * for smaller object size and lower complexity.
 */
template<typename T>
class Return<Local<T>> : public ReturnValueBase
{
public:
    static_assert(std::is_base_of<Value, T>::value, "Given handle type must inherit Value");
    
    template<typename S>
    Return(Local<S> value) : RV_BASE_OK_CTOR, storage_(value) {
        static_assert(std::is_base_of<T, S>::value, "Given handle types are not convertible");
    }
    Return(Error err) : RV_BASE_ERR_CTOR {}

    g_nodiscard Local<T> Extract() const {
        CHECK(!HasError());
        return storage_;
    }
    
    void AssignTo(v8::ReturnValue<Value> ret_value) const {
        CHECK(!HasError() && !storage_.IsEmpty());
        ret_value.Set(storage_);
    }
    
private:
    Local<T> storage_;
};

template<typename T>
using Ret = Return<T>;

template<typename T>
using RetLocal = Return<Local<T>>;

#undef RV_BASE_OK_CTOR
#undef RV_BASE_ERR_CTOR

ReturnValueBase::Error Fail(ErrorType type, const std::string_view& message);
ReturnValueBase::Error Fail(const v8::TryCatch& try_catch, const std::string_view& prefix = "");

/**
 * When a JavaScript exception was thrown directly by the function,
 * JavaScript execution is not allowed and the function should return this
 * immediately to indicate that it expects the exception will propagate
 * freely until someone can catch it.
 */
ReturnValueBase::Error FreePropagate();

#define FFI_RET_TO_PROPAGATE_IF_CAUGHT(try_catch) \
    if (try_catch.HasCaught()) { try_catch.ReThrow(); return ffi::FreePropagate(); }


GALLIUM_FFI_NS_END

#endif //COCOA_GALLIUM_FFI_RETURNVALUE_H
