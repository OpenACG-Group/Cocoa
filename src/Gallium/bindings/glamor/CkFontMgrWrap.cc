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

#include "include/core/SkString.h"

#include "Gallium/bindings/glamor/CkFontMgrWrap.h"
#include "Gallium/bindings/glamor/CkTypefaceWrap.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

int CkFontMgr::countFamilies()
{
    return getSkiaObject()->countFamilies();
}

std::string CkFontMgr::getFamilyName(int index)
{
    if (index < 0 || index >= countFamilies())
        g_throw(RangeError, "Invalid family index");

    SkString name;
    getSkiaObject()->getFamilyName(index, &name);
    return name.c_str();
}

v8::Local<v8::Value> CkFontMgr::createStyleSet(int index)
{
    if (index < 0 || index >= countFamilies())
        g_throw(RangeError, "Invalid family index");

    sk_sp<SkFontStyleSet> set(getSkiaObject()->createStyleSet(index));
    return binder::Class<CkFontStyleSet>::create_object(
            v8::Isolate::GetCurrent(), set);
}

v8::Local<v8::Value> CkFontMgr::matchFamilyStyle(v8::Local<v8::Value> family_name,
                                                 v8::Local<v8::Value> style)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    std::string name;
    if (!family_name->IsNullOrUndefined())
    {
        if (!family_name->IsString())
            g_throw(TypeError, "Argument `familyName` must be a string or null");
        name = binder::from_v8<std::string>(isolate, family_name);
    }

    auto *w = binder::Class<CkFontStyle>::unwrap_object(isolate, style);
    if (!w)
        g_throw(TypeError, "Argument `style` must be an instance of `CkFontStyle`");

    sk_sp<SkTypeface> tf(getSkiaObject()->matchFamilyStyle(
            name.empty() ? nullptr : name.c_str(), w->GetFontStyle()));
    if (!tf)
        return v8::Null(isolate);

    return binder::Class<CkTypeface>::create_object(isolate, tf);
}

int CkFontStyleSet::count()
{
    return getSkiaObject()->count();
}

v8::Local<v8::Value> CkFontStyleSet::getStyle(int index)
{
    if (index < 0 || index >= count())
        g_throw(RangeError, "Invalid style index");

    SkFontStyle style;
    getSkiaObject()->getStyle(index, &style, nullptr);

    return binder::Class<CkFontStyle>::create_object(
            v8::Isolate::GetCurrent(), style);
}

std::string CkFontStyleSet::getStyleName(int index)
{
    if (index < 0 || index >= count())
        g_throw(RangeError, "Invalid style index");

    SkString name;
    getSkiaObject()->getStyle(index, nullptr, &name);
    return name.c_str();
}

v8::Local<v8::Value> CkFontStyleSet::createTypeface(int index)
{
    if (index < 0 || index >= count())
        g_throw(RangeError, "Invalid style index");

    sk_sp<SkTypeface> tf(getSkiaObject()->createTypeface(index));
    if (!tf)
        return v8::Null(v8::Isolate::GetCurrent());

    return binder::Class<CkTypeface>::create_object(v8::Isolate::GetCurrent(), tf);
}

v8::Local<v8::Value> CkFontStyleSet::matchStyle(v8::Local<v8::Value> pattern)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *w = binder::Class<CkFontStyle>::unwrap_object(isolate, pattern);
    if (!w)
        g_throw(TypeError, "Argument `pattern` must be an instance of `CkFontStyle`");

    sk_sp<SkTypeface> tf(getSkiaObject()->matchStyle(w->GetFontStyle()));
    if (!tf)
        return v8::Null(isolate);

    return binder::Class<CkTypeface>::create_object(isolate, tf);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
