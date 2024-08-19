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

#ifndef COCOA_GALLIUM_FFI_ENUM_H
#define COCOA_GALLIUM_FFI_ENUM_H

#include <type_traits>

#include "Gallium/Gallium.h"
#include "Gallium/ffi/DataTypes.h"
#include "Gallium/ffi/EnumRegistry.h"
GALLIUM_FFI_NS_BEGIN

template<typename T>
class Enum
{
    static_assert(std::is_enum<T>::value, "`T` must be an enum");

public:
    using UnderT = typename std::underlying_type<T>::type;

    Enum() : value_(0) {}
    explicit Enum(T value) : value_(static_cast<UnderT>(value)) {}
    ~Enum() = default;

    T operator*() const {
        return static_cast<T>(value_);
    }

    Enum<T>& operator=(T v) {
        value_ = static_cast<UnderT>(v);
        return *this;
    }

    g_nodiscard T Get() const {
        return static_cast<T>(value_);
    }

    g_nodiscard UnderT GetInteger() const {
        return value_;
    }

private:
    UnderT value_;
};

template<typename T>
class Return<Enum<T>> : public ReturnValueBase
{
public:
    Return(Enum<T> value) : ReturnValueBase(State::kOk), value_(value) {}
    Return(T value) : ReturnValueBase(State::kOk), value_(value) {}
    Return(Error err) : ReturnValueBase(State::kError, err) {}

    g_nodiscard Enum<T> Extract() const {
        CHECK(!HasError());
        return value_;
    }

    void AssignTo(v8::ReturnValue<Value> ret_value) {
        CHECK(!HasError());
        ret_value.Set(value_.GetInteger());
    }

private:
    Enum<T> value_;
};

template<typename T>
struct Cast<Enum<T>>
{
    constexpr static bool kWrapperCast = true;

    static Ret<Enum<T>> From(Isolate *isolate, Local<Value> value) {
        EnumRegistry *registry = EnumRegistry::FromIsolate(isolate);
        Ret<int64_t> result = registry->CastEnumValueSafe(ClassTypeInfo::Get<T>(), value);
        if (result.HasError())
            return result.GetError();
        return static_cast<T>(result.Extract());
    }

    static RetLocal<Value> To(Isolate *isolate, const Enum<T>& value) {
        auto ival = value.GetInteger();
        return Cast<typename Enum<T>::UnderT>::To(isolate, ival);
    }
};

template<typename T>
class DefineEnum
{
    static_assert(std::is_enum<T>::value, "`T` must be an enum");

public:
    DefineEnum(Isolate *isolate, const std::string_view& name, bool bitfield);
    ~DefineEnum() = default;

    DefineEnum<T>& Item(const std::string_view& name, T value);
    Local<Object> Finalize();

private:
    EnumRegistry *registry_;
    EnumRegistry::EnumInfo info_;
};

template<typename T>
DefineEnum<T>::DefineEnum(Isolate *isolate, const std::string_view& name, bool bitfield)
    : registry_(EnumRegistry::FromIsolate(isolate))
    , info_(name, bitfield)
{
}

template<typename T>
DefineEnum<T>& DefineEnum<T>::Item(const std::string_view& name, T value)
{
    info_.items.emplace(name, static_cast<int64_t>(value));
    return *this;
}

template<typename T>
Local<Object> DefineEnum<T>::Finalize()
{
    return registry_->AddEnum(ClassTypeInfo::Get<T>(), std::move(info_));
}

GALLIUM_FFI_NS_END
#endif //COCOA_GALLIUM_FFI_ENUM_H
