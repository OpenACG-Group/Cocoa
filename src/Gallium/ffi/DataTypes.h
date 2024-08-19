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

#ifndef COCOA_GALLIUM_FFI_DATATYPES_H
#define COCOA_GALLIUM_FFI_DATATYPES_H

#include <type_traits>
#include <limits>
#include <climits>
#include <optional>
#include <map>
#include <unordered_map>

#include "include/v8.h"

#include "Gallium/Gallium.h"
#include "Gallium/ffi/TypeTraits.h"
#include "Core/Errors.h"
GALLIUM_FFI_NS_BEGIN

template<typename T>
using Opt = std::optional<T>;

using v8::Array;
using v8::Boolean;
using v8::Global;
using v8::Local;
using v8::Value;
using v8::MaybeLocal;
using v8::String;
using v8::Isolate;
using v8::Context;
using v8::Number;
using v8::Int32;
using v8::Uint32;
using v8::Object;
using v8::FunctionTemplate;
using v8::ObjectTemplate;

template<typename T>
using OptLocal = std::optional<Local<T>>;

template<typename T>
bool IsCertainHandle(Local<Value> value, T* = nullptr)
{
    static_assert(std::is_base_of<v8::Data, T>::value, "Typecheck failed");
    return false;
}

template<>
inline bool IsCertainHandle(Local<Value> value, v8::Value*)
{
    return true;
}

#define IS_SOME_SPECIALIZE(type)                                            \
    template<> inline bool IsCertainHandle(Local<Value> value, v8::type*) { \
        return value->Is##type();                                           \
    }

IS_SOME_SPECIALIZE(Name)
IS_SOME_SPECIALIZE(String)
IS_SOME_SPECIALIZE(Symbol)
IS_SOME_SPECIALIZE(Function)
IS_SOME_SPECIALIZE(Array)
IS_SOME_SPECIALIZE(Object)
IS_SOME_SPECIALIZE(BigInt)
IS_SOME_SPECIALIZE(Boolean)
IS_SOME_SPECIALIZE(Number)
IS_SOME_SPECIALIZE(External)
IS_SOME_SPECIALIZE(Int32)
IS_SOME_SPECIALIZE(Uint32)
IS_SOME_SPECIALIZE(Date)
IS_SOME_SPECIALIZE(BigIntObject)
IS_SOME_SPECIALIZE(BooleanObject)
IS_SOME_SPECIALIZE(NumberObject)
IS_SOME_SPECIALIZE(StringObject)
IS_SOME_SPECIALIZE(SymbolObject)
IS_SOME_SPECIALIZE(RegExp)
IS_SOME_SPECIALIZE(Promise)
IS_SOME_SPECIALIZE(Map)
IS_SOME_SPECIALIZE(Set)
IS_SOME_SPECIALIZE(ArrayBuffer)
IS_SOME_SPECIALIZE(ArrayBufferView)
IS_SOME_SPECIALIZE(TypedArray)
IS_SOME_SPECIALIZE(Uint8Array)
IS_SOME_SPECIALIZE(Uint8ClampedArray)
IS_SOME_SPECIALIZE(Int8Array)
IS_SOME_SPECIALIZE(Uint16Array)
IS_SOME_SPECIALIZE(Int16Array)
IS_SOME_SPECIALIZE(Uint32Array)
IS_SOME_SPECIALIZE(Int32Array)
IS_SOME_SPECIALIZE(Float32Array)
IS_SOME_SPECIALIZE(Float64Array)
IS_SOME_SPECIALIZE(BigInt64Array)
IS_SOME_SPECIALIZE(BigUint64Array)
IS_SOME_SPECIALIZE(DataView)
IS_SOME_SPECIALIZE(SharedArrayBuffer)
IS_SOME_SPECIALIZE(Proxy)
IS_SOME_SPECIALIZE(WasmMemoryObject)
IS_SOME_SPECIALIZE(WasmModuleObject)

// IS_SOME_SPECIALIZE(Module)
// IS_SOME_SPECIALIZE(FixedArray)
// IS_SOME_SPECIALIZE(Private)
// IS_SOME_SPECIALIZE(ObjectTemplate)
// IS_SOME_SPECIALIZE(FunctionTemplate)
// IS_SOME_SPECIALIZE(Context)

#undef IS_SOME_SPECIALIZE

template<typename T, typename Enable = void>
struct Cast;

template<typename Str>
struct Cast<Str, typename std::enable_if<is_string<Str>::value>::type>
{
    constexpr static bool kShouldCheckTo = true;
    constexpr static bool kWrapperCast = false;
    
    using Char = typename Str::value_type;
    using Traits = typename Str::traits_type;
    static_assert(sizeof(Char) <= sizeof(uint16_t) && "not UTF-8 or UTF-16 string");

    static bool Is(Isolate*, Local<Value> value) {
        return !value.IsEmpty() && value->IsString();
    }

    struct ConvertibleStr : std::basic_string<Char, Traits> {
        using base_class = std::basic_string<Char, Traits>;
        using base_class::base_class;

        explicit operator Char const *() const {
            return this->c_str();
        }
    };

    static Opt<ConvertibleStr> From(Isolate *isolate, Local<Value> value) {
        if (!Is(isolate, value)) {
            return std::nullopt;
        }
        if constexpr (sizeof(Char) == 1) {
            String::Utf8Value const u8_value(isolate, value);
            return ConvertibleStr(reinterpret_cast<Char const*>(*u8_value), u8_value.length());
        } else {
            String::Value const u16_value(isolate, value);
            return ConvertibleStr(reinterpret_cast<Char const*>(*u16_value), u16_value.length());
        }
    }
    
    static ConvertibleStr FromChecked(Isolate *isolate, Local<Value> value) {
        auto result = From(isolate, value);
        CHECK(result.has_value());
        return std::move(*result);
    }

    static Local<String> To(Isolate *isolate, std::basic_string_view<Char, Traits> value) {
        MaybeLocal<String> res;
        if constexpr (sizeof(Char) == 1) {
             res = String::NewFromUtf8(
                    isolate,
                    reinterpret_cast<char const*>(value.data()),
                    v8::NewStringType::kNormal,
                    static_cast<int>(value.size()));
        } else {
            res = String::NewFromTwoByte(
                    isolate,
                    reinterpret_cast<uint16_t const*>(value.data()),
                    v8::NewStringType::kNormal,
                    static_cast<int>(value.size()));
        }
        return res.FromMaybe(Local<String>());
    }
    
    static Local<String> ToChecked(Isolate *isolate, std::basic_string_view<Char, Traits> value) {
        Local<String> result = To(isolate, value);
        CHECK(!result.IsEmpty());
        return result;
    }
};

template<>
struct Cast<char const *> : Cast<std::basic_string_view<char>> {};

template<>
struct Cast<char16_t const *> : Cast<std::basic_string_view<char16_t>> {};

template<>
struct Cast<bool>
{
    constexpr static bool kShouldCheckTo = false;
    constexpr static bool kWrapperCast = false;
    
    static bool Is(Isolate*, Local<Value> value) {
        return !value.IsEmpty() && value->IsBoolean();
    }
    
    static Opt<bool> From(Isolate *isolate, Local<Value> value) {
        if (!Is(isolate, value)) {
            return std::nullopt;
        }
        return value->BooleanValue(isolate);
    }
    
    static bool FromChecked(Isolate *isolate, Local<Value> value) {
        auto result = From(isolate, value);
        CHECK(result.has_value());
        return *result;
    }

    static Local<Boolean> To(Isolate *isolate, bool value) {
        return Boolean::New(isolate, value);
    }

    static Local<Boolean> ToChecked(Isolate *isolate, bool value) {
        return To(isolate, value);
    }
};

using ObjectLiteralMap = std::map<std::string, v8::Local<v8::Value>>;

template<typename T>
struct Cast<T, typename std::enable_if<std::is_integral_v<T>>::type>
{
    constexpr static bool kShouldCheckTo = false;
    constexpr static bool kWrapperCast = false;
    
    constexpr static int kBits = sizeof(T) * CHAR_BIT;
    constexpr static bool kSigned = std::is_signed<T>::value;

    constexpr static int64_t kMaxSafeInt = 9007199254740991;
    constexpr static int64_t kMinSafeInt = -9007199254740991;

    static bool Is(Isolate*, Local<Value> value) {
        return !value.IsEmpty() && value->IsNumber();
    }

    static Opt<T> From(Isolate *isolate, Local<Value> value) {
        if (!Is(isolate, value)) {
            return std::nullopt;
        }
        Local<Context> ctx = isolate->GetCurrentContext();
        if constexpr (kBits <= 32) {
            if constexpr (kSigned) {
                return static_cast<T>(value->Int32Value(ctx).FromJust());
            } else {
                return static_cast<T>(value->Uint32Value(ctx).FromJust());
            }
        } else {
            return static_cast<T>(value->IntegerValue(ctx).FromJust());
        }
    }

    static T FromChecked(Isolate *isolate, Local<Value> value) {
        auto result = From(isolate, value);
        CHECK(result.has_value());
        return *result;
    }

    // This conversion checks whether i64/u64 value is in the safe range
    static Local<Number> To(Isolate *isolate, T value) {
        if constexpr (kBits <= 32) {
            if constexpr (kSigned) {
                return Int32::New(isolate, static_cast<int32_t>(value));
            } else {
                return Uint32::New(isolate, static_cast<uint32_t>(value));
            }
        } else {
            if (value > kMaxSafeInt || value < kMinSafeInt) {
                return {};
            }
            return Number::New(isolate, static_cast<double>(value));
        }
    }

    static Local<Number> ToChecked(Isolate *isolate, T value) {
        Local<Number> h = To(isolate, value);
        CHECK(!h.IsEmpty() && "lossy conversion");
        return h;
    }
};

template<typename T>
struct Cast<T, typename std::enable_if<std::is_floating_point_v<T>>::type>
{
    constexpr static bool kShouldCheckTo = false;
    constexpr static bool kWrapperCast = false;

    static bool Is(Isolate*, Local<Value> value) {
        return !value.IsEmpty() && value->IsNumber();
    }

    static Opt<T> From(Isolate *isolate, Local<Value> value) {
        if (!Is(isolate, value)) {
            return std::nullopt;
        }
        Local<Context> ctx = isolate->GetCurrentContext();
        return static_cast<T>(value->NumberValue(ctx).FromJust());
    }

    static T FromChecked(Isolate *isolate, Local<Value> value) {
        auto result = From(isolate, value);
        CHECK(result.has_value());
        return *result;
    }

    static Local<Number> To(Isolate *isolate, T value) {
        // TODO(sora): Check if the conversion from `T` to `double` is lossless
        return Number::New(isolate, static_cast<double>(value));
    }

    static Local<Number> ToChecked(Isolate *isolate, T value) {
        return To(isolate, value);
    }
};

// Wraps an `int64_t` number
struct PreciseI64
{
    int64_t operator*() const {
        return value;
    }
    int64_t value;
};

template<>
struct Cast<PreciseI64>
{
    constexpr static bool kShouldCheckTo = false;
    constexpr static bool kWrapperCast = false;

    static bool Is(Isolate*, Local<Value> value) {
        return !value.IsEmpty() && (value->IsBigInt() || value->IsInt32());
    }

    static Opt<PreciseI64> From(Isolate *isolate, Local<Value> value) {
        if (!Is(isolate, value)) {
            return std::nullopt;
        }
        if (value->IsInt32()) {
            return PreciseI64{value.As<Int32>()->Value()};
        }
        bool lossless;
        int64_t i64value = value.As<v8::BigInt>()->Int64Value(&lossless);
        if (!lossless) {
            // Accurate conversion fails as int64 overflow
            return std::nullopt;
        }
        return PreciseI64{i64value};
    }

    static PreciseI64 FromChecked(Isolate *isolate, Local<Value> value) {
        auto result = From(isolate, value);
        CHECK(result.has_value());
        return *result;
    }

    static Local<v8::BigInt> To(Isolate *isolate, int64_t value) {
        return v8::BigInt::New(isolate, value);
    }

    static Local<v8::BigInt> ToChecked(Isolate *isolate, int64_t value) {
        return To(isolate, value);
    }
};

// Wraps an `uint64_t` number, corresponds to `bigint` in JavaScript
struct PreciseU64
{
    uint64_t operator*() const {
        return value;
    }
    uint64_t value;
};

template<>
struct Cast<PreciseU64>
{
    constexpr static bool kShouldCheckTo = false;
    constexpr static bool kWrapperCast = false;

    static bool Is(Isolate*, Local<Value> value) {
        return !value.IsEmpty() && (value->IsBigInt() || value->IsUint32());
    }

    static Opt<PreciseU64> From(Isolate *isolate, Local<Value> value) {
        if (!Is(isolate, value)) {
            return std::nullopt;
        }
        if (value->IsUint32()) {
            return PreciseU64{value.As<Uint32>()->Value()};
        }
        bool lossless;
        uint64_t u64value = value.As<v8::BigInt>()->Uint64Value(&lossless);
        if (!lossless) {
            // Accurate conversion fails as uint64 overflow
            return std::nullopt;
        }
        return PreciseU64{u64value};
    }

    static PreciseU64 FromChecked(Isolate *isolate, Local<Value> value) {
        auto result = From(isolate, value);
        CHECK(result.has_value());
        return *result;
    }

    static Local<v8::BigInt> To(Isolate *isolate, uint64_t value) {
        return v8::BigInt::NewFromUnsigned(isolate, value);
    }

    static Local<v8::BigInt> ToChecked(Isolate *isolate, uint64_t value) {
        return To(isolate, value);
    }
};

template<typename MapT>
struct Cast<MapT, typename std::enable_if<is_mapping<MapT>::value>::type>
{
    constexpr static bool kShouldCheckTo = true;
    constexpr static bool kWrapperCast = false;

    using KeyT = typename MapT::key_type;
    using MappedT = typename MapT::mapped_type;

    static_assert(std::is_same<KeyT, std::string>::value,
                  "The key of `MapT` type must be std::string");

    static bool Is(Isolate*, Local<Value> value) {
        return !value.IsEmpty() && value->IsObject();
    }

    static Opt<MapT> From(Isolate *isolate, Local<Value> value) {
        if (!Is(isolate, value)) {
            return std::nullopt;
        }

        Local<Context> ctx = isolate->GetCurrentContext();
        Local<Object> obj = value.As<Object>();
        Local<Array> prop_names = obj->GetPropertyNames(
                ctx,
                v8::KeyCollectionMode::kIncludePrototypes,
                v8::PropertyFilter::ONLY_ENUMERABLE,
                v8::IndexFilter::kIncludeIndices,
                v8::KeyConversionMode::kConvertToString).ToLocalChecked();

        MapT result;
        uint32_t count = prop_names->Length();
        for (uint32_t i = 0; i < count; i++)
        {
            v8::EscapableHandleScope escape(isolate);
            auto key = prop_names->Get(ctx, i).ToLocalChecked();
            auto val = obj->Get(ctx, key).ToLocalChecked();

            Opt<std::string> str_key = Cast<std::string>::From(isolate, key);
            Opt<MappedT> cvtval = Cast<MappedT>::From(isolate, val);
            if (!str_key || !cvtval)
                return std::nullopt;

            if constexpr (is_local_handle<MappedT>::value) {
                result.emplace(std::move(*str_key), escape.Escape(*cvtval));
            } else {
                result.emplace(std::move(*str_key), std::move(*cvtval));
            }
        }

        return result;
    }

    static MapT FromChecked(Isolate *isolate, Local<Value> value) {
        Opt<MapT> result = From(isolate, value);
        CHECK(result.has_value());
        return std::move(*result);
    }

    static Local<Object> To(Isolate *isolate, const MapT& map) {
        v8::EscapableHandleScope handle_scope(isolate);
        Local<Context> ctx = isolate->GetCurrentContext();

        Local<Object> obj = Object::New(isolate);
        for (const auto& item : map)
        {
            Local<String> name = Cast<std::string>::To(isolate, item.first);
            if (name.IsEmpty())
                return {};

            Local<Value> value = Cast<MappedT>::To(isolate, item.second);
            if (value.IsEmpty())
                return {};
            
            obj->Set(ctx, name, value).Check();
        }
        return handle_scope.Escape(obj);
    }
    
    static Local<Object> ToChecked(Isolate *isolate, const MapT& map) {
        Local<Object> result = To(isolate, map);
        CHECK(!result.IsEmpty());
        return result;
    }
};

template<typename T>
struct Cast<std::vector<T>>
{
    constexpr static bool kShouldCheckTo = true;
    constexpr static bool kWrapperCast = false;

    static_assert(!Cast<T>::kWrapperCast, "unsupported type");

    static bool Is(Isolate*, Local<Value> value) {
        return !value.IsEmpty() && value->IsArray();
    }

    static Opt<std::vector<T>> From(v8::Isolate *isolate, v8::Local<v8::Value> value) {
        if (!Is(isolate, value)) {
            return std::nullopt;
        }
        auto array = value.As<v8::Array>();
        uint32_t length = array->Length();
        if (length == 0)
            return std::vector<T>();

        std::vector<T> vec;
        vec.reserve(length);
        v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
        v8::TryCatch try_catch(isolate);
        for (uint32_t i = 0; i < length; i++)
        {
            v8::Local<v8::Value> element;
            if (!array->Get(ctx, i).ToLocal(&element))
                return {};
            Opt<T> optv = Cast<T>::From(isolate, element);
            if (!optv)
                return {};
            vec.emplace_back(std::move(*optv));
        }

        return vec;
    }

    static std::vector<T> FromChecked(v8::Isolate *isolate, v8::Local<v8::Value> value) {
        Opt<std::vector<T>> result = From(isolate, value);
        CHECK(result.has_value());
        return std::move(*result);
    }

    static v8::Local<v8::Array> To(v8::Isolate *isolate, const std::vector<T>& vec) {
        if (vec.empty())
            return v8::Array::New(isolate);
        std::vector<v8::Local<v8::Value>> values;
        values.reserve(vec.size());
        for (uint32_t i = 0; i < vec.size(); i++)
        {
            v8::Local<v8::Value> element = Cast<T>::To(isolate, vec[i]);
            if (element.IsEmpty())
                return {};
            values.emplace_back(element);
        }
        return v8::Array::New(isolate, values.data(), values.size());
    }

    static v8::Local<v8::Array> ToChecked(v8::Isolate *isolate, const std::vector<T>& vec) {
        v8::Local<v8::Array> result = To(isolate, vec);
        CHECK(!result.IsEmpty());
        return result;
    }
};

template<typename...Ts>
struct Cast<std::tuple<Ts...>>
{
    constexpr static bool kShouldCheckTo = true;
    constexpr static bool kWrapperCast = false;

    constexpr static size_t kN = sizeof...(Ts);

    using Tuple = std::tuple<Ts...>;

    static_assert(!((... || Cast<Ts>::kWrapperCast)), "unsupported type");

    static bool Is(Isolate*, Local<Value> value) {
        return !value.IsEmpty() && value->IsArray() &&
               value.As<Array>()->Length() == kN;
    }

    static Opt<Tuple> From(Isolate *isolate, Local<Value> value) {
        if (!Is(isolate, value)) {
            return std::nullopt;
        }
        return FromImpl(isolate, value, std::make_index_sequence<kN>());
    }

    static Tuple FromChecked(Isolate *isolate, Local<Value> value) {
        auto result = From(isolate, value);
        CHECK(result.has_value());
        return std::move(*result);
    }

    static MaybeLocal<Array> To(Isolate *isolate, const Tuple& tuple) {
        return ToImpl(isolate, tuple, std::make_index_sequence<kN>());
    }

    static Local<Array> ToChecked(Isolate *isolate, const Tuple& tuple) {
        return To(isolate, tuple).ToLocalChecked();
    }

private:
    template<size_t...Idx>
    static Opt<Tuple> FromImpl(Isolate *isolate, Local<Value> value,
                               std::index_sequence<Idx...>)
    {
        v8::HandleScope handle_scope(isolate);
        Local<Context> ctx = isolate->GetCurrentContext();
        Local<Array> arr = value.As<Array>();

        std::tuple<Opt<Ts>...> opts{
            Cast<Ts>::From(isolate, arr->Get(ctx, Idx).ToLocalChecked())...
        };

        bool all_has_value = (... && std::get<Idx>(opts).has_value());
        if (!all_has_value)
            return std::nullopt;

        return Tuple{ (*std::get<Idx>(opts)) ... };
    }

    template<size_t...Idx>
    static Local<Array> ToImpl(Isolate *isolate, const Tuple& tuple,
                               std::index_sequence<Idx...>)
    {
        v8::EscapableHandleScope handle_scope(isolate);

        Local<Value> values[] = {
            Cast<Ts>::To(isolate, std::get<Idx>(tuple))...
        };
        bool any_empty = (... || values[Idx].IsEmpty());
        if (any_empty)
            return {};

        return handle_scope.Escape(Array::New(isolate, values, kN));
    }
};

// No implicit conversions are performed. Handle types will be checked strictly
// (use `Local<R>::As<T>()` instead of `Value::ToXXX()` for conversion).
template<typename T>
struct Cast<Local<T>>
{
    constexpr static bool kShouldCheckTo = false;
    constexpr static bool kWrapperCast = false;

    static bool Is(Isolate*, Local<Value> value) {
        return IsCertainHandle<T>(value);
    }

    static Opt<Local<T>> From(Isolate *isolate, Local<Value> value) {
        if (!Is(isolate, value)) {
            return std::nullopt;
        }
        return value.As<T>();
    }

    static Local<T> FromChecked(Isolate *isolate, Local<Value> value) {
        auto result = From(isolate, value);
        CHECK(result.has_value());
        return *result;
    }

    static Local<T> To(Isolate *isolate, Local<T> value) {
        return value;
    }

    static Local<T> ToChecked(Isolate *isolate, Local<T> value) {
        return To(isolate, value);
    }
};

struct ArgAdapter
{
    // Implementor `TAdapter` should:
    //  - implement static method `Ret<TAdapter> Cast(Isolate*, Local<Value>)`
};

GALLIUM_FFI_NS_END
#endif //COCOA_GALLIUM_FFI_DATATYPES_H
