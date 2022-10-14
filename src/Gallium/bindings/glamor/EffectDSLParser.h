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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_EFFECTDSLPARSER_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_EFFECTDSLPARSER_H

#include <memory>
#include <vector>

#include "include/core/SkPoint3.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkColorFilter.h"
#include "fmt/format.h"

#include "Gallium/bindings/glamor/Exports.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

struct Effector
{
    enum Type
    {
        kImageFilter,
        kColorFilter,
        kNull
    };

    Effector(sk_sp<SkImageFilter> IF)
        : type(kImageFilter), image_filter(std::move(IF)) {}

    Effector(sk_sp<SkColorFilter> CF)
        : type(kColorFilter), color_filter(std::move(CF)) {}

    explicit Effector() : type(kNull) {}

    g_inline explicit operator bool() const noexcept {
        return (image_filter || color_filter);
    }

    g_nodiscard g_inline sk_sp<SkImageFilter> CheckImageFilter() const {
        if (type != kImageFilter)
            g_throw(Error, "Operand is not an image filter");
        return image_filter;
    }

    g_nodiscard g_inline sk_sp<SkColorFilter> CheckColorFilter() const {
        if (type != kColorFilter)
            g_throw(Error, "Operand is not a color filter");
        return color_filter;
    }

    Type type;
    sk_sp<SkImageFilter> image_filter;
    sk_sp<SkColorFilter> color_filter;
};

struct EffectStackOperand
{
    using Ptr = std::unique_ptr<EffectStackOperand>;
    enum Type
    {
        kNull,
        kInt,
        kFloat,
        kEffector,
        kArray,
        kKWArgs
    };

    template<typename T>
    using Nullable = std::optional<T>;

    using KWArgsMap = std::unordered_map<std::string, v8::Local<v8::Value>>;
    using KWArgsPair = std::pair<std::string, v8::Local<v8::Value>>;

    static const char *GetTypeName(Type type);

    template<size_t N>
    void AssertTypes(const std::array<Type, N>& types) const
    {
        bool match = false;
        for (Type expected : types)
        {
            if (type == expected)
                match = true;
        }
        if (!match)
        {
            auto msg = fmt::format("Unexpected operand type {}", GetTypeName(type));
            g_throw(Error, msg);
        }
    }

    void AssertKWArgsJSType(bool check_result);

    // Cast functions:
    // Cast an operand to a required C++ type (primitive type or compound type).
    // These functions return a null value if the operand is null,
    // and throw a JSException if fail to convert.

    Nullable<SkScalar> ToFloatSafe();
    Nullable<int32_t> ToIntegerSafe();
    Nullable<Effector> ToEffectorSafe();
    Nullable<sk_sp<SkImage>> ToImageSafe();

    Nullable<sk_sp<SkImageFilter>> ToImageFilterSafe() {
        if (auto e = ToEffectorSafe())
            return e->CheckImageFilter();
        return std::nullopt;
    }

    Nullable<sk_sp<SkColorFilter>> ToColorFilterSafe() {
        if (auto e = ToEffectorSafe())
            return e->CheckColorFilter();
        return std::nullopt;
    }

    // Float[4]
    Nullable<SkRect> ToRectSafe();

    // Float[4]
    Nullable<SkColor> ToColorSafe();

    // Float[3]
    Nullable<SkPoint3> ToVector3Safe();

    // Float[2]
    Nullable<SkPoint> ToVector2Safe();

    // Int[2]
    Nullable<SkIPoint> ToIVector2Safe();

    template<typename T, typename Cast = std::function<Nullable<T>(const Ptr&)>>
    Nullable<std::vector<T>> ToMonoTypeArraySafe(const Cast& value_cast)
    {
        if (type == kNull)
            return {};

        AssertTypes<2>({kArray, kKWArgs});

        std::vector<T> result;
        if (type == kArray)
        {
            for (const auto& operand : array)
            {
                Nullable<T> maybe = value_cast(operand);
                if (!maybe)
                    g_throw(Error, "Array members must not be null");
                result.emplace_back(std::move(*maybe));
            }
        }
        else if (type == kKWArgs)
        {
            if (!kwarg_pair.second->IsTypedArray())
            {
                g_throw(TypeError,
                        fmt::format("Keyword argument `{}` must be a typed array",
                                    kwarg_pair.first));
            }

            auto typed_array = v8::Local<v8::TypedArray>::Cast(kwarg_pair.second);
            v8::Isolate *isolate = v8::Isolate::GetCurrent();
            auto context = isolate->GetCurrentContext();
            for (int i = 0; i < typed_array->Length(); i++)
            {
                result.push_back(binder::from_v8<T>(isolate,
                    typed_array->Get(context, i).ToLocalChecked()));
            }
        }
        return std::move(result);
    }

    Type type;

    // Only available when `type` is `kInt` or `kFloat`
    union Numeric_ {
        int32_t     vi = 0;
        SkScalar    vf;
    } numeric;

    // Only available when `type` is `kEffector`
    Effector effector;

    // Only available when `type` is `kArray`
    std::vector<Ptr> array;

    // Only available when `type` is `kKWArgs`
    KWArgsPair kwarg_pair;
};
using EffectStack = std::stack<EffectStackOperand::Ptr>;

class EffectDSLParser
{
public:
    // Effector builder functions can throw a `JSException` to indicate that
    // an error has occurred during creating a specific effector.
    using EffectorBuilder = std::function<Effector(EffectStack&, int)>;

    // A map from effect names to the corresponding effector builder functions
    using EffectorBuildersMap = std::map<std::string_view, EffectorBuilder>;

    g_nodiscard static Effector Parse(v8::Isolate *isolate,
                                      v8::Local<v8::String> dsl,
                                      v8::Local<v8::Object> kwargs,
                                      EffectorBuildersMap& builders_map);
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_EFFECTDSLPARSER_H
