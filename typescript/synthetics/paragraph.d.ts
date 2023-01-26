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

import * as GL from 'synthetic://glamor';

export type RectHeightStyle = number;
export type RectWidthStyle = number;
export type TextAlign = number;
export type TextDirection = number;
export type TextBaseline = number;
export type TextHeightBehavior = number;
export type LineMetricStyle = number;
export type TextDecoration = number;
export type TextDecorationStyle = number;
export type TextDecorationMode = number;
export type StyleType = number;
export type PlaceholderAlignment = number;
export type Affinity = number;

export type Bitfield<T> = number;
export type Enum<T> = number;

interface IConstants {
    readonly RECT_HEIGHT_STYLE_TIGHT: RectHeightStyle;
    readonly RECT_HEIGHT_STYLE_MAX: RectHeightStyle;
    readonly RECT_HEIGHT_STYLE_INCLUDE_LINE_SPACING_MIDDLE: RectHeightStyle;
    readonly RECT_HEIGHT_STYLE_INCLUDE_LINE_SPACING_TOP: RectHeightStyle;
    readonly RECT_HEIGHT_STYLE_INCLUDE_LINE_SPACING_BOTTOM: RectHeightStyle;
    readonly RECT_HEIGHT_STYLE_STRUT: RectHeightStyle;

    readonly RECT_WIDTH_STYLE_TIGHT: RectWidthStyle;
    readonly RECT_WIDTH_STYLE_MAX: RectWidthStyle;

    readonly TEXT_ALIGN_LEFT: TextAlign;
    readonly TEXT_ALIGN_RIGHT: TextAlign;
    readonly TEXT_ALIGN_CENTER: TextAlign;
    readonly TEXT_ALIGN_JUSTIFY: TextAlign;
    readonly TEXT_ALIGN_START: TextAlign;
    readonly TEXT_ALIGN_END: TextAlign;

    readonly TEXT_DIRECTION_RTL: TextDirection;
    readonly TEXT_DIRECTION_LTR: TextDirection;

    readonly TEXT_BASELINE_ALPHABETIC: TextBaseline;
    readonly TEXT_BASELINE_IDEOGRAPHIC: TextBaseline;

    readonly TEXT_HEIGHT_BEHAVIOR_ALL: TextHeightBehavior;
    readonly TEXT_HEIGHT_BEHAVIOR_DISABLE_FIRST_ASCENT: TextHeightBehavior;
    readonly TEXT_HEIGHT_BEHAVIOR_DISABLE_LAST_DESCENT: TextHeightBehavior;
    readonly TEXT_HEIGHT_BEHAVIOR_DISABLE_ALL: TextHeightBehavior;

    readonly LINE_METRIC_STYLE_TYPOGRAPHIC: LineMetricStyle;
    readonly LINE_METRIC_STYLE_CSS: LineMetricStyle;

    readonly TEXT_DECORATION_NO_DECORATION: TextDecoration;
    readonly TEXT_DECORATION_UNDERLINE: TextDecoration;
    readonly TEXT_DECORATION_OVERLINE: TextDecoration;
    readonly TEXT_DECORATION_LINE_THROUGH: TextDecoration;

    readonly TEXT_DECORATION_STYLE_SOLID: TextDecorationStyle;
    readonly TEXT_DECORATION_STYLE_DOUBLE: TextDecorationStyle;
    readonly TEXT_DECORATION_STYLE_DOTTED: TextDecorationStyle;
    readonly TEXT_DECORATION_STYLE_DASHED: TextDecorationStyle;
    readonly TEXT_DECORATION_STYLE_WAVY: TextDecorationStyle;

    readonly TEXT_DECORATION_MODE_GAPS: TextDecorationMode;
    readonly TEXT_DECORATION_MODE_THROUGH: TextDecorationMode;

    readonly STYLE_TYPE_NONE: StyleType;
    readonly STYLE_TYPE_ALL_ATTRIBUTES: StyleType;
    readonly STYLE_TYPE_FONT: StyleType;
    readonly STYLE_TYPE_FOREGROUND: StyleType;
    readonly STYLE_TYPE_BACKGROUND: StyleType;
    readonly STYLE_TYPE_SHADOW: StyleType;
    readonly STYLE_TYPE_DECORATIONS: StyleType;
    readonly STYLE_TYPE_LETTER_SPACING: StyleType;
    readonly STYLE_TYPE_WORD_SPACING: StyleType;

    readonly PLACEHOLDER_ALIGNMENT_BASELINE: PlaceholderAlignment;
    readonly PLACEHOLDER_ALIGNMENT_ABOVE_BASELINE: PlaceholderAlignment;
    readonly PLACEHOLDER_ALIGNMENT_BELOW_BASELINE: PlaceholderAlignment;
    readonly PLACEHOLDER_ALIGNMENT_TOP: PlaceholderAlignment;
    readonly PLACEHOLDER_ALIGNMENT_BOTTOM: PlaceholderAlignment;
    readonly PLACEHOLDER_ALIGNMENT_MIDDLE: PlaceholderAlignment;

    readonly AFFINITY_UPSTREAM: Affinity;
    readonly AFFINITY_DOWNSTREAM: Affinity;
}

export const Constants: IConstants;

export interface StrutStyle {
    fontFamilies: Array<string>;
    fontStyle: GL.CkFontStyle;
    fontSize: number;
    height: number;
    leading: number;
    forceHeight: boolean;
    enabled: boolean;
    heightOverride: boolean;
    halfLeading: boolean;
}

export interface FontFeature {
    name: string;
    value: number;
}

export interface Decoration {
    type: Bitfield<TextDecoration>;
    mode: Enum<TextDecorationMode>;
    color: GL.CkColor4f;
    style: Enum<TextDecorationStyle>;
    thicknessMultiplier: number;
}

export interface PlaceholderStyle {
    width: number;
    height: number;
    alignment: Enum<PlaceholderAlignment>;
    baseline: Enum<TextBaseline>;
    baselineOffset: number;
}

export interface TextShadow {
    color: GL.CkColor4f;
    offset: GL.CkPoint;
    sigma: number;
}

export class TextStyle {
    public constructor();
    public color: GL.CkColor4f;
    public foreground: GL.CkPaint | null;
    public background: GL.CkPaint | null;
    public decoration: Decoration;
    public fontStyle: GL.CkFontStyle;
    public fontSize: number;
    public baselineShift: number;
    public height: number;
    public heightOverride: boolean;
    public halfLeading: boolean;
    public letterSpacing: number;
    public wordSpacing: number;
    public locale: string;
    public textBaseline: Enum<TextBaseline>;
    public addShadow(shadow: TextShadow): void;
    public resetShadows(): void;
    public addFontFeature(feature: FontFeature): void;
    public resetFontFeatures(): void;
    public setFontFamilies(fontFamilies: Array<string>): void;
    public setTypeface(tf: GL.CkTypeface): void;
    public isPlaceholder(): boolean;
    public setPlaceholder(): void;
    public clone(): TextStyle;
    public cloneForPlaceholder(): TextStyle;
}

export class ParagraphStyle {
    public constructor();
    public strutStyle: StrutStyle;;
    public textStyle: TextStyle;;
    public textDirection: Enum<TextDirection>;
    public textAlign: Enum<TextAlign>;
    public maxLines: number;
    public height: number;
    public textHeightBehavior: Enum<TextHeightBehavior>;
    public setEllipsis(value: string): void;
    public hintingIsOn(): boolean;
    public turnHintingOff(): void;
    public getReplaceTabCharacters(): boolean;
    public setReplaceTabCharacters(value: boolean): void;
}

export class ParagraphBuilder {
    private constructor();
    public static Make(paraStyle: ParagraphStyle, fontMgr: GL.CkFontMgr): ParagraphBuilder;
    public pushStyle(style: TextStyle): ParagraphBuilder;
    public pop(): ParagraphBuilder;
    public addText(text: string): ParagraphBuilder;
    public addPlaceholder(style: PlaceholderStyle): ParagraphBuilder;
    public reset(): ParagraphBuilder;
    public build(): Paragraph;
}

export interface TextBox {
    rect: GL.CkRect;
    direction: Enum<TextDirection>;
}

export interface PositionWithAffinity {
    position: number;
    affinity: Enum<Affinity>;
}

export class Paragraph {
    private constructor();
    public readonly maxWidth: number;
    public readonly height: number;
    public readonly minIntrinsicWidth: number;
    public readonly maxIntrinsicWidth: number;
    public readonly alphabeticBaseline: number;
    public readonly ideographicBaseline: number;
    public readonly longestLine: number;
    public readonly exceedMaxLines: number;
    public layout(width: number): void;
    public paint(canvas: GL.CkCanvas, x: number, y: number): void;
    public getRectsForRange(start: number, end: number, hStyle: Enum<RectHeightStyle>, wStyle: Enum<RectWidthStyle>): Array<TextBox>;
    public getRectsForPlaceholders(): Array<TextBox>;
    public getGlyphPositionAtCoordinate(dx: number, dy: number): PositionWithAffinity;
    public getWordBoundary(offset: number): [number, number];
}
