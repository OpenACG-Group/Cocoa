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

#ifndef COCOA_GALLIUM_BINDINGS_PARAGRAPH_EXPORTS_H
#define COCOA_GALLIUM_BINDINGS_PARAGRAPH_EXPORTS_H

#include <utility>

#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "include/v8.h"

#include "Core/Project.h"

#define GALLIUM_BINDINGS_PARAGRAPH_NS_BEGIN namespace cocoa::gallium::bindings::paragraph_wrap {
#define GALLIUM_BINDINGS_PARAGRAPH_NS_END   }

GALLIUM_BINDINGS_PARAGRAPH_NS_BEGIN

void SetInstanceProperties(v8::Local<v8::Object> instance);

//! TSDecl:
//! interface StrutStyle {
//!   fontFamilies: Array<string>;
//!   fontStyle: glamor.CkFontStyle;
//!   fontSize: number;
//!   height: number;
//!   leading: number;
//!   forceHeight: boolean;
//!   enabled: boolean;
//!   heightOverride: boolean;
//!   halfLeading: boolean;
//! }
skia::textlayout::StrutStyle
ExtractStrutStyle(v8::Isolate *isolate, v8::Local<v8::Value> v);

v8::Local<v8::Value> WrapStrutStyle(v8::Isolate *isolate,
                                    const skia::textlayout::StrutStyle& style);

//! TSDecl:
//! interface FontFeature {
//!   name: string;
//!   value: number;
//! }
skia::textlayout::FontFeature
ExtractFontFeature(v8::Isolate *isolate, v8::Local<v8::Value> v);

//! TSDecl:
//! interface Decoration {
//!   type: Bitfield<TextDecoration>;
//!   mode: Enum<TextDecorationMode>;
//!   color: glamor.CkColor4f;
//!   style: Enum<TextDecorationStyle>;
//!   thicknessMultiplier: number;
//! }
skia::textlayout::Decoration
ExtractDecoration(v8::Isolate *isolate, v8::Local<v8::Value> v);

v8::Local<v8::Value> WrapDecoration(v8::Isolate *isolate,
                                    const skia::textlayout::Decoration& deco);

//! TSDecl:
//! interface PlaceholderStyle {
//!   width: number;
//!   height: number;
//!   alignment: Enum<PlaceholderAlignment>;
//!   baseline: Enum<TextBaseline>;
//!   baselineOffset: number;
//! }
skia::textlayout::PlaceholderStyle
ExtractPlaceholderStyle(v8::Isolate *isolate, v8::Local<v8::Value> v);

//! TSDecl:
//! interface TextShadow {
//!   color: glamor.Color4f;
//!   offset: glamor.CkPoint;
//!   sigma: number;
//! }
skia::textlayout::TextShadow
ExtractTextShadow(v8::Isolate *isolate, v8::Local<v8::Value> v);

//! TSDecl: class TextStyle
class TextStyleWrap
{
public:
    //! TSDecl: constructor()
    TextStyleWrap() = default;

    explicit TextStyleWrap(const skia::textlayout::TextStyle& style)
        : text_style_(style) {}
    ~TextStyleWrap() = default;

    g_nodiscard g_inline skia::textlayout::TextStyle& GetTextStyle() {
        return text_style_;
    }

#define PRIMITIVE_GETTER_SETTER(prop, type) \
    g_inline type get##prop() { return text_style_.get##prop(); } \
    g_inline void set##prop(type v) { text_style_.set##prop(v); }

#define ENUM_GETTER_SETTER(prop, type) \
    g_inline int32_t get##prop() { return static_cast<int32_t>(text_style_.get##prop()); } \
    g_inline void set##prop(type v) { text_style_.set##prop(static_cast<type>(v)); }

    //! TSDecl: color: glamor.Color4f
    v8::Local<v8::Value> getColor();
    void setColor(v8::Local<v8::Value> v);

    //! TSDecl: foreground: glamor.CkPaint | null
    v8::Local<v8::Value> getForeground();
    void setForeground(v8::Local<v8::Value> v);

    //! TSDecl: background: glamor.CkPaint | null
    v8::Local<v8::Value> getBackground();
    void setBackground(v8::Local<v8::Value> v);

    //! TSDecl: decoration: Decoration
    v8::Local<v8::Value> getDecoration();
    void setDecoration(v8::Local<v8::Value> v);

    //! TSDecl: fontStyle: CkFontStyle
    v8::Local<v8::Value> getFontStyle();
    void setFontStyle(v8::Local<v8::Value> v);

    //! TSDecl: fontSize: number
    PRIMITIVE_GETTER_SETTER(FontSize, SkScalar)

    //! TSDecl: baselineShift: number
    PRIMITIVE_GETTER_SETTER(BaselineShift, SkScalar)

    //! TSDecl: height: number
    PRIMITIVE_GETTER_SETTER(Height, SkScalar)

    //! TSDecl: heightOverride: boolean
    PRIMITIVE_GETTER_SETTER(HeightOverride, bool)

    //! TSDecl: halfLeading: boolean
    PRIMITIVE_GETTER_SETTER(HalfLeading, bool)

    //! TSDecl: letterSpacing: number
    PRIMITIVE_GETTER_SETTER(LetterSpacing, SkScalar)

    //! TSDecl: wordSpacing: number
    PRIMITIVE_GETTER_SETTER(WordSpacing, SkScalar)

    //! TSDecl: locale: string
    g_inline std::string getLocale() { return text_style_.getLocale().c_str(); }
    g_inline void setLocale(const std::string& v) { text_style_.setLocale(SkString(v)); }

    //! TSDecl: textBaseline: Enum<TextBaseline>
    ENUM_GETTER_SETTER(TextBaseline, skia::textlayout::TextBaseline)

    //! TSDecl: function addShadow(shadow: TextShadow): void
    void addShadow(v8::Local<v8::Value> v);

    //! TSDecl: function resetShadows(): void
    void resetShadows();

    //! TSDecl: function addFontFeature(feature: FontFeature): void
    void addFontFeature(v8::Local<v8::Value> v);

    //! TSDecl: function resetFontFeatures(): void
    void resetFontFeatures();

    //! TSDecl: function setFontFamilies(fontFamilies: Array<string>): void
    void setFontFamilies(v8::Local<v8::Value> v);

    //! TSDecl: function setTypeface(tf: CkTypeface): void
    void setTypeface(v8::Local<v8::Value> v);

    //! TSDecl: function isPlaceholder(): boolean
    g_inline bool isPlaceholder() { return text_style_.isPlaceholder(); }

    //! TSDecl: function setPlaceholder(): void
    g_inline void setPlaceholder() { text_style_.setPlaceholder(); }

    // TODO(sora): font arguments

    //! TSDecl: function clone(): TextStyle
    v8::Local<v8::Value> clone();

    //! TSDecl: function cloneForPlaceholder(): TextStyle
    v8::Local<v8::Value> cloneForPlaceholder();

#undef PRIMITIVE_GETTER_SETTER
#undef ENUM_GETTER_SETTER

private:
    skia::textlayout::TextStyle text_style_;
};


//! TSDecl: class ParagraphStyle
class ParagraphStyleWrap
{
public:
    //! TSDecl: constructor()
    ParagraphStyleWrap() = default;

    explicit ParagraphStyleWrap(skia::textlayout::ParagraphStyle style)
        : style_(std::move(style)) {}
    ~ParagraphStyleWrap() = default;

    g_nodiscard g_inline skia::textlayout::ParagraphStyle& GetStyle() {
        return style_;
    }

    //! TSDecl: strutStyle: StrutStyle;
    v8::Local<v8::Value> getStrutStyle();
    void setStrutStyle(v8::Local<v8::Value> v);

    //! TSDecl: textStyle: TextStyle;
    v8::Local<v8::Value> getTextStyle();
    void setTextStyle(v8::Local<v8::Value> v);

    //! TSDecl: textDirection: Enum<TextDirection>
    g_inline int32_t getTextDirection() {
        return static_cast<int32_t>(style_.getTextDirection());
    }
    void setTextDirection(int32_t v);

    //! TSDecl: textAlign: Enum<TextAlign>
    g_inline int32_t getTextAlign() {
        return static_cast<int32_t>(style_.getTextAlign());
    }
    void setTextAlign(int32_t v);

    //! TSDecl: maxLines: number
    g_inline uint32_t getMaxLines() { return style_.getMaxLines(); }
    g_inline void setMaxLines(uint32_t v) { style_.setMaxLines(v); }

    //! TSDecl: height: number
    g_inline SkScalar getHeight() { return style_.getHeight(); }
    g_inline void setHeight(SkScalar v) { style_.setHeight(v); }

    //! TSDecl: textHeightBehavior: Enum<TextHeightBehavior>
    g_inline int32_t getTextHeightBehavior() {
        return static_cast<int32_t>(style_.getTextHeightBehavior());
    }
    void setTextHeightBehavior(int32_t v);

    //! TSDecl: function setEllipsis(value: string): void
    void setEllipsis(v8::Local<v8::Value> value);

    //! TSDecl: function hintingIsOn(): boolean
    g_inline bool hintingIsOn() {
        return style_.hintingIsOn();
    }

    //! TSDecl: function turnHintingOff(): void
    g_inline void turnHintingOff() {
        style_.turnHintingOff();
    }

    //! TSDecl: function getReplaceTabCharacters(): boolean
    g_inline bool getReplaceTabCharacters() {
        return style_.getReplaceTabCharacters();
    }

    //! TSDecl: function setReplaceTabCharacters(value: boolean): void
    g_inline void setReplaceTabCharacters(bool value) {
        style_.setReplaceTabCharacters(value);
    }

private:
    skia::textlayout::ParagraphStyle style_;
};


//! TSDecl: class ParagraphBuilder
class ParagraphBuilderWrap
{
public:
    ParagraphBuilderWrap(v8::Isolate *isolate,
                         std::unique_ptr<skia::textlayout::ParagraphBuilder> builder)
        : isolate_(isolate), builder_(std::move(builder)) {}
    ~ParagraphBuilderWrap() = default;

    //! TSDecl: function Make(paraStyle: ParagraphStyle,
    //!                       fontMgr: glamor.CkFontMgr): ParagraphBuilder
    static v8::Local<v8::Value> Make(v8::Local<v8::Value> paragraph_style,
                                     v8::Local<v8::Value> font_mgr);

    //! TSDecl: function pushStyle(style: TextStyle): ParagraphBuilder
    v8::Local<v8::Value> pushStyle(v8::Local<v8::Value> style);

    //! TSDecl: function pop(): ParagraphBuilder
    v8::Local<v8::Value> pop();

    //! TSDecl: function addText(text: string): ParagraphBuilder
    v8::Local<v8::Value> addText(v8::Local<v8::Value> text);

    //! TSDecl: function addPlaceholder(style: PlaceholderStyle): ParagraphBuilder
    v8::Local<v8::Value> addPlaceholder(v8::Local<v8::Value> style);

    //! TSDecl: function reset(): ParagraphBuilder
    v8::Local<v8::Value> reset();

    //! TSDecl: function build(): Paragraph
    v8::Local<v8::Value> build();

private:
    v8::Local<v8::Value> Self();

    v8::Isolate *isolate_;
    v8::Global<v8::Object> self_;
    std::unique_ptr<skia::textlayout::ParagraphBuilder> builder_;
};

//! TSDecl: class Paragraph
class ParagraphWrap
{
public:
    explicit ParagraphWrap(std::unique_ptr<skia::textlayout::Paragraph> paragraph)
        : paragraph_(std::move(paragraph)) {}
    ~ParagraphWrap() = default;

#define SCALAR_PROP_GETTER(what) \
    g_inline SkScalar get##what() { return paragraph_->get##what(); }

    //! TSDecl: readonly maxWidth: number
    SCALAR_PROP_GETTER(MaxWidth)

    //! TSDecl: readonly height: number
    SCALAR_PROP_GETTER(Height)

    //! TSDecl: readonly minIntrinsicWidth: number
    SCALAR_PROP_GETTER(MinIntrinsicWidth)

    //! TSDecl: readonly maxIntrinsicWidth: number
    SCALAR_PROP_GETTER(MaxIntrinsicWidth)

    //! TSDecl: readonly alphabeticBaseline: number
    SCALAR_PROP_GETTER(AlphabeticBaseline)

    //! TSDecl: readonly ideographicBaseline: number
    SCALAR_PROP_GETTER(IdeographicBaseline)

    //! TSDecl: readonly longestLine: number
    SCALAR_PROP_GETTER(LongestLine)

    //! TSDecl: readonly exceedMaxLines: number
    g_inline bool getExceedMaxLines() { return paragraph_->didExceedMaxLines(); }

#undef SCALAR_PROP_GETTER

    //! TSDecl: function layout(width: number): void
    void layout(SkScalar width);

    //! TSDecl: function paint(canvas: glamor.CkCanvas, x: number, y: number): void
    void paint(v8::Local<v8::Value> canvas, SkScalar x, SkScalar y);

    //! TSDecl: interface TextBox {
    //!   rect: glamor.CkRect;
    //!   direction: Enum<TextDirection>;
    //! }

    //! TSDecl: function getRectsForRange(start: number, end: number,
    //!                                   hStyle: Enum<RectHeightStyle>,
    //!                                   wStyle: Enum<RectWidthStyle>): Array<TextBox>
    v8::Local<v8::Value> getRectsForRange(int32_t start, int32_t end,
                                          int32_t hstyle, int32_t wstyle);

    //! TSDecl: function getRectsForPlaceholders(): Array<TextBox>
    v8::Local<v8::Value> getRectsForPlaceholders();

    //! TSDecl: interface PositionWithAffinity {
    //!   position: number;
    //!   affinity: Enum<Affinity>;
    //! }

    //! TSDecl: function getGlyphPositionAtCoordinate(dx: number, dy: number): PositionWithAffinity
    v8::Local<v8::Value> getGlyphPositionAtCoordinate(SkScalar dx, SkScalar dy);

    //! TSDecl: function getWordBoundary(offset: number): [number, number]
    v8::Local<v8::Value> getWordBoundary(int32_t offset);

private:
    std::unique_ptr<skia::textlayout::Paragraph> paragraph_;
};

GALLIUM_BINDINGS_PARAGRAPH_NS_END
#endif //COCOA_GALLIUM_BINDINGS_PARAGRAPH_EXPORTS_H
