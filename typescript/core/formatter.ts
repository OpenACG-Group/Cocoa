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

/**
 * Formatter: Stringify any JavaScript values.
 * Like the `console.log()` method in Node.js, this module implements a
 * highly flexible way to stringify and print JavaScript objects.
 */

import * as TypeTraits from './typetraits';

export const kObjectFormatter = Symbol.for('@ObjectFormatter');

/**
 * Instances of classes which implement this interface can be formatted
 * by `format` function in the user-defined way.
 */
export interface Formattable {
    /**
     * A method that will be called when the instance is formatted.
     * An array of TextBlocks representing the formatted text should be returned.
     *
     * @param ctx Current formatter context, which can be used for circular
     *            reference detection, or being passed to another formatter function.
     */
    [kObjectFormatter](ctx: FormatterContext): Array<TextBlock>;
}

export interface ObjectFormatOptions {
    colorANSI?: boolean;

    indentInSpaces?: number;

    maximumInlineWidth?: number;
    maximumStringLength?: number;

    showSymbols?: boolean;
    showProperties?: boolean;

    numberToString?: (v: number) => string;
    bigintToString?: (v: bigint) => string;
}

const defaultOptions = Object.freeze<ObjectFormatOptions>({
    colorANSI: true,
    indentInSpaces: 2,
    maximumInlineWidth: 80,
    maximumStringLength: 90,
    showSymbols: false,
    showProperties: true,

    numberToString: (v: number): string => {
        return v.toString(10);
    },

    bigintToString: (v: bigint): string => {
        return v.toString(10) + 'n';
    }
});

function mergeUserOptionsOverrides(userOptions: ObjectFormatOptions): ObjectFormatOptions {
    const userOptionKeys = Object.keys(userOptions);
    const validKeys = Object.keys(defaultOptions);
    const result: ObjectFormatOptions = {};
    Object.assign(result, defaultOptions);
    for (const key of userOptionKeys) {
        const ret = validKeys.find((value: string): boolean => {
            return (value == key);
        });
        if (ret != undefined) {
            result[key] = userOptions[key];
        }
    }

    return result;
}

export enum TextColor {
    kBlack,
    kRed,
    kGreen,
    kYellow,
    kBlue,
    kMagenta,
    kCyan,
    kWhite,
    kBlackBright,
    kRedBright,
    kGreenBright,
    kYellowBright,
    kBlueBright,
    kMagentaBright,
    kCyanBright,
    kWhiteBright,
    kDefault
}

export enum TextStyle {
    kReset,
    kBold,
    kDim,
    kItalic,
    kUnderline,
    kBlink,
    kInverse,
    kHidden,
    kStrikeThrough,
    kDoubleUnderline
}

const DEFAULT_FG = 39;
const DEFAULT_BG = 49;

type EscapeEnableDisablePair = [number, number];
const styleEscapes = new Map<TextStyle, EscapeEnableDisablePair>([
    [ TextStyle.kReset, [0, 0] ],
    [ TextStyle.kBold, [1, 22] ],
    [ TextStyle.kDim, [2, 22] ],
    [ TextStyle.kItalic, [3, 23] ],
    [ TextStyle.kUnderline, [4, 24] ],
    [ TextStyle.kBlink, [5, 25] ],
    [ TextStyle.kInverse, [7, 27] ],
    [ TextStyle.kHidden, [8, 28] ],
    [ TextStyle.kStrikeThrough, [9, 29] ],
    [ TextStyle.kDoubleUnderline, [21, 24] ]
]);

const fgColorEscapes = new Map<TextColor, EscapeEnableDisablePair>([
    [ TextColor.kBlack, [30, DEFAULT_FG] ],
    [ TextColor.kRed, [31, DEFAULT_FG] ],
    [ TextColor.kGreen, [32, DEFAULT_FG] ],
    [ TextColor.kYellow, [33, DEFAULT_FG] ],
    [ TextColor.kBlue, [34, DEFAULT_FG] ],
    [ TextColor.kMagenta, [35, DEFAULT_FG] ],
    [ TextColor.kCyan, [36, DEFAULT_FG] ],
    [ TextColor.kWhite, [37, DEFAULT_FG] ],
    [ TextColor.kBlackBright, [90, DEFAULT_FG] ],
    [ TextColor.kRedBright, [91, DEFAULT_FG] ],
    [ TextColor.kGreenBright, [92, DEFAULT_FG] ],
    [ TextColor.kYellowBright, [93, DEFAULT_FG] ],
    [ TextColor.kBlueBright, [94, DEFAULT_FG] ],
    [ TextColor.kMagentaBright, [95, DEFAULT_FG] ],
    [ TextColor.kCyanBright, [96, DEFAULT_FG] ],
    [ TextColor.kWhiteBright, [97, DEFAULT_FG] ]
]);

const bgColorEscapes = new Map<TextColor, EscapeEnableDisablePair>([
    [ TextColor.kBlack, [40, DEFAULT_BG] ],
    [ TextColor.kRed, [41, DEFAULT_BG] ],
    [ TextColor.kGreen, [42, DEFAULT_BG] ],
    [ TextColor.kYellow, [43, DEFAULT_BG] ],
    [ TextColor.kBlue, [44, DEFAULT_BG] ],
    [ TextColor.kMagenta, [45, DEFAULT_BG] ],
    [ TextColor.kCyan, [46, DEFAULT_BG] ],
    [ TextColor.kWhite, [47, DEFAULT_BG] ],
    [ TextColor.kBlackBright, [100, DEFAULT_BG] ],
    [ TextColor.kRedBright, [101, DEFAULT_BG] ],
    [ TextColor.kGreenBright, [102, DEFAULT_BG] ],
    [ TextColor.kYellowBright, [103, DEFAULT_BG] ],
    [ TextColor.kBlueBright, [104, DEFAULT_BG] ],
    [ TextColor.kMagentaBright, [105, DEFAULT_BG] ],
    [ TextColor.kCyanBright, [106, DEFAULT_BG] ],
    [ TextColor.kWhiteBright, [107, DEFAULT_BG] ]
]);

// A TextAttributeGroup represents a sequence of characters that should be
// printed in specific attributes (typically color and font style).
export class TextAttributeGroup {
    constructor(public content: string,
                public foreground: TextColor = TextColor.kDefault,
                public background: TextColor = TextColor.kDefault,
                public styles: Array<TextStyle> = []) {}
}

// Shorthand of `new TextAttributeGroup()`
export function TAG(content: string, ...args: any[]): TextAttributeGroup {
    return new TextAttributeGroup(content, ...args);
}

export enum TextBlockLayoutHint {
    kPrefix,
    kValue,
    kPropertyName,
    kSeparator,
    kArrowSeparator,
    kCompoundStructureBegin,
    kCompoundStructureEnd
}

// A TextBlock is an unsplittable sequence of TextAttributeGroups.
// It must be printed as a whole, which means no whitespaces and newlines
// can be inserted into the TextBlock. Whitespaces and newlines
// only can be inserted between TextBlocks in a sequence of TextBlocks.
export class TextBlock {
    constructor(public hint: TextBlockLayoutHint,
                public contents: Array<TextAttributeGroup>) {}

    /**
     * Count the number of characters in all the TextAttributeGroups.
     * A multibyte character like CJK character is treated as a single character
     * (just the same behaviour to JavaScript `string`).
     */
    public measureTextCharacters(): number {
        let w = 0;
        for (const G of this.contents) {
            w += G.content.length;
        }
        return w;
    }

    /**
     * Get a string representation of the TextBlock.
     * This method combines all the TextAttributeGroups and optionally inserts
     * the required ANSI escape characters.
     * @param stripEscape   Do not insert ANSI escape characters if true;
     *                      otherwise, the required ANSI escape characters will be inserted.
     *                      If the contents will be written to a plaintext file or printed
     *                      on a TTY device which does not support ANSI escape characters,
     *                      you may need to enable this.
     */
    public composite(stripEscape: boolean = false): string {
        let result = stripEscape ? '' : '\x1B[0m';

        // Fast path when escape stripped
        if (stripEscape) {
            for (const group of this.contents) {
                result += group.content;
            }
            return result;
        }

        for (const group of this.contents) {
            let fgColor = DEFAULT_FG, bgColor = DEFAULT_BG;
            if (group.foreground != TextColor.kDefault) {
                fgColor = fgColorEscapes.get(group.foreground)[0];
            }
            if (group.background != TextColor.kDefault) {
                bgColor = bgColorEscapes.get(group.background)[0];
            }
            result += `\x1B[${fgColor};${bgColor}`;

            for (const style of group.styles) {
                const p = styleEscapes.get(style)[0];
                result += `;${p}`;
            }

            result += 'm';
            result += group.content;
            result += '\x1B[0m';
        }
        return result;
    }
}

// Shorthand of `new TextBlock()`
export function TB(hint: TextBlockLayoutHint, contents: Array<TextAttributeGroup>): TextBlock {
    return new TextBlock(hint, contents);
}

interface CompoundValueStub {
    value: any;
    id: number;
    refcnt: number;
}

export class FormatterContext {
    options: ObjectFormatOptions;
    // For circular reference detecting
    compoundValueStubStack: Array<CompoundValueStub>;
    idCounter: number;

    constructor(userOptions: ObjectFormatOptions) {
        this.options = mergeUserOptionsOverrides(userOptions);
        this.compoundValueStubStack = [];
        this.idCounter = 1;
    }

    public pushCompoundValueStub(value: any): CompoundValueStub {
        const entry = {
            value: value,
            id: this.idCounter++,
            refcnt: 0
        };
        this.compoundValueStubStack.push(entry);
        return entry;
    }

    public popCompoundValueStub(): void {
        this.compoundValueStubStack.pop();
    }

    public detectCompoundValueCircularRef(value: any): CompoundValueStub | null {
        for (const stub of this.compoundValueStubStack) {
            if (value == stub.value) {
                stub.refcnt++;
                return stub;
            }
        }
        return null;
    }
}

const CLASS_REGEXP = /^(\s+[^(]*?)\s*{/;
// eslint-disable-next-line node-core/no-unescaped-regexp-dot
const STRIP_COMMENTS_REGEXP = /(\/\/.*?\n)|(\/\*(.|\n)*?\*\/)/g;

export function formatClassValue(value: Function): Array<TextBlock> {
    const name = value.name.length > 0 ? value.name : '(anonymous)';

    const result = TB(TextBlockLayoutHint.kValue, [
        TAG('[', TextColor.kCyan),
        TAG('class', TextColor.kCyan, TextColor.kDefault, [TextStyle.kItalic]),
        TAG(' '),
        TAG(name, TextColor.kCyan, TextColor.kDefault, [TextStyle.kBold])
    ]);

    const constructor = TypeTraits.GetConstructorName(value);
    if (constructor != 'Function' && constructor != null) {
        result.contents.push(
            TAG(' '),
            TAG(`[${constructor}]`, TextColor.kCyan)
        );
    }

    let superName = '[null prototype]';
    if (constructor != null) {
        superName = Object.getPrototypeOf(value).name;
    }

    if (superName) {
        result.contents.push(
            TAG(' '),
            TAG('extends', TextColor.kCyan, TextColor.kDefault, [TextStyle.kItalic]),
            TAG(' '),
            TAG(superName, TextColor.kCyan, TextColor.kDefault, [TextStyle.kBold])
        );
    }

    result.contents.push(TAG(']', TextColor.kCyan));
    return [result];
}

export function formatFunctionValue(value: Function): Array<TextBlock> {
    // `value` is a class
    const source = value.toString();

    // Reference: Node.js source code (v16.0.0-pre):
    // lib/internal/util/inspect.js:1123
    if (source.startsWith('class') && source.endsWith('}')) {
        const slice = source.slice(5, -1);
        const bracketIndex = slice.indexOf('{');
        if (bracketIndex !== -1 &&
            (!slice.slice(0, bracketIndex).includes('(') ||
                // Slow path to guarantee that it's indeed a class.
                CLASS_REGEXP.test(slice.replace(STRIP_COMMENTS_REGEXP, '')))) {
            return formatClassValue(value);
        }
    }

    // `value` is a normal function
    let funcType = 'Function';
    if (TypeTraits.IsAsyncFunction(value) && TypeTraits.IsGeneratorFunction(value)) {
        funcType = 'AsyncGeneratorFunction';
    } else if (TypeTraits.IsGeneratorFunction(value)) {
        funcType = 'GeneratorFunction';
    } else if (TypeTraits.IsAsyncFunction(value)) {
        funcType = 'AsyncFunction';
    }
    let name = (value as Function).name;
    if (name.length == 0) {
        name = '(anonymous)';
    }

    return [TB(TextBlockLayoutHint.kValue, [
        TAG('[', TextColor.kCyan),
        TAG(funcType + ':', TextColor.kCyan, TextColor.kDefault, [TextStyle.kItalic]),
        TAG(' '),
        TAG(name, TextColor.kCyan, TextColor.kDefault, [TextStyle.kBold]),
        TAG(']', TextColor.kCyan)
    ])];
}

const INDEX_PROP_REGEXP = /^[0-9]+$/;

export function formatArrayValue(value: Array<any>, ctx: FormatterContext): Array<TextBlock> {
    const constructor = TypeTraits.GetConstructorName(value);

    const result: Array<TextBlock> = [];
    let nonIndexProps: string[] = TypeTraits.GetOwnNonIndexProperties(
        value, TypeTraits.Constants.PROPERTY_FILTER_ONLY_ENUMERABLE);
    if (constructor != 'Array') {
        // Non-ordinary array (maybe it is constructed from a class that
        // extends Array object)

        // TextBlock: (optional) prefix
        result.push(TB(TextBlockLayoutHint.kPrefix, [
            TAG((constructor ? constructor : 'Array') + `(${value.length})`)
        ]));
    }

    // TextBlock: open bracket [
    result.push(TB(TextBlockLayoutHint.kCompoundStructureBegin, [TAG('[')]));

    const indexProperties = Object.getOwnPropertyNames(value)
        .filter((e) => INDEX_PROP_REGEXP.test(e))
        .map((e) => Number.parseInt(e))
        .sort((a, b) => a - b);

    for (let i = 0; i < indexProperties.length - 1; i++) {
        result.push(...formatAnyValue(value[indexProperties[i]], ctx));
        result.push(TB(TextBlockLayoutHint.kSeparator, [TAG(',')]));

        // Sparse arrays
        const sparse = indexProperties[i + 1] - indexProperties[i] - 1;
        if (sparse > 0) {
            result.push(TB(TextBlockLayoutHint.kValue, [
                TAG(sparse == 1 ? 'empty' : `empty x ${sparse}`,
                    TextColor.kBlackBright, TextColor.kDefault, [TextStyle.kItalic])
            ]));
            result.push(TB(TextBlockLayoutHint.kSeparator, [TAG(',')]));
        }
    }

    if (indexProperties.length > 0) {
        const lastIndex = indexProperties[indexProperties.length - 1];
        result.push(...formatAnyValue(value[lastIndex], ctx));

        // Sparse arrays
        const sparse = value.length - lastIndex - 1;
        if (sparse > 0) {
            result.push(TB(TextBlockLayoutHint.kSeparator, [TAG(',')]));
            result.push(TB(TextBlockLayoutHint.kValue, [
                TAG(sparse == 1 ? 'empty' : `empty x ${sparse}`,
                    TextColor.kBlackBright, TextColor.kDefault, [TextStyle.kItalic])
            ]));
        }
    } else if (value.length > 0) {
        result.push(TB(TextBlockLayoutHint.kValue, [
            TAG(`empty x ${value.length}`,
                TextColor.kBlackBright, TextColor.kDefault, [TextStyle.kItalic])
        ]));
    }

    // Extra properties
    if (value.length > 0 && nonIndexProps.length > 0) {
        // There must be something (actual values or empty placeholders)
        // that has been printed before extra properties.
        result.push(TB(TextBlockLayoutHint.kSeparator, [TAG(',')]));
    }

    let firstExtraProp = true;
    for (const key of nonIndexProps) {
        if (firstExtraProp) {
            firstExtraProp = false;
        } else {
            result.push(TB(TextBlockLayoutHint.kSeparator, [TAG(',')]));
        }
        result.push(TB(TextBlockLayoutHint.kPropertyName, [TAG(key + ':')]));
        result.push(...formatAnyValue(value[key], ctx));
    }

    // TextBlock: close bracket ]
    result.push(TB(TextBlockLayoutHint.kCompoundStructureEnd, [TAG(']')]));

    return result;
}

function internalObjectPrefix(value: any, expects: string, size: number): Array<TextBlock> {
    let constructor = TypeTraits.GetConstructorName(value);
    const result: Array<TextBlock> = [];
    if (constructor != expects) {
        // `value` is not constructed by internal constructor (for example, Set, Map, etc.).
        // Instead, it is constructed by something which extends the internal constructors.

        if (!constructor) {
            constructor = '(anonymous)';
        }

        result.push(TB(TextBlockLayoutHint.kPrefix, [TAG(`${constructor}(${value.size})`),]));
        const tag = value[Symbol.toStringTag];
        if (tag) {
            result.push(TB(TextBlockLayoutHint.kPrefix, [TAG(`[${tag}]`)]));
        }
    } else {
        result.push(TB(TextBlockLayoutHint.kPrefix, [TAG(`${expects}(${size})`)]));
    }

    return result;
}

function collectExtraOwnProperties(value: any, firstOne: boolean, ctx: FormatterContext): Array<TextBlock> {
    const result: Array<TextBlock> = [];
    const propertyNames = Object.getOwnPropertyNames(value);
    for (const key of propertyNames) {
        if (firstOne) {
            firstOne = false;
        } else {
            result.push(TB(TextBlockLayoutHint.kSeparator, [TAG(',')]));
        }
        result.push(
            TB(TextBlockLayoutHint.kPropertyName, [TAG(`${key}:`)]),
            ...formatAnyValue(value[key], ctx)
        );
    }
    return result;
}

export function formatSetValue(value: Set<any>, ctx: FormatterContext): Array<TextBlock> {
    const result: Array<TextBlock> = internalObjectPrefix(value, 'Set', value.size);

    result.push(TB(TextBlockLayoutHint.kCompoundStructureBegin, [TAG('{')]));
    let firstElement = true;
    for (const element of value) {
        if (firstElement) {
            firstElement = false;
        } else {
            result.push(TB(TextBlockLayoutHint.kSeparator, [TAG(',')]));
        }
        result.push(...formatAnyValue(element, ctx));
    }

    result.push(...collectExtraOwnProperties(value, firstElement, ctx));

    result.push(TB(TextBlockLayoutHint.kCompoundStructureEnd, [TAG('}')]));
    return result;
}

export function formatMapValue(value: Map<any, any>, ctx: FormatterContext): Array<TextBlock> {
    const result: Array<TextBlock> = internalObjectPrefix(value, 'Map', value.size);

    result.push(TB(TextBlockLayoutHint.kCompoundStructureBegin, [TAG('{')]));

    let firstEntry = true;
    for (const [k, v] of value) {
        if (firstEntry) {
            firstEntry = false;
        } else {
            result.push(TB(TextBlockLayoutHint.kSeparator, [TAG(',')]));
        }
        result.push(
            ...formatAnyValue(k, ctx),
            TB(TextBlockLayoutHint.kArrowSeparator, [TAG('=>')]),
            ...formatAnyValue(v, ctx)
        );
    }

    result.push(...collectExtraOwnProperties(value, firstEntry, ctx));

    result.push(TB(TextBlockLayoutHint.kCompoundStructureEnd, [TAG('}')]));
    return result;
}

export function formatTypedArray(value: object, ctx: FormatterContext): Array<TextBlock> {
    const typename = value[Symbol.toStringTag];
    const length = value['length'];

    const result: Array<TextBlock> = [
        TB(TextBlockLayoutHint.kPrefix, [
            TAG(`${typename}(${length})`)
        ]),
        TB(TextBlockLayoutHint.kCompoundStructureBegin, [
            TAG('[')
        ])
    ];

    for (let i = 0; i < length; i++) {
        result.push(...formatAnyValue(value[i], ctx));
        if (i != length - 1) {
            result.push(TB(TextBlockLayoutHint.kSeparator, [TAG(',')]));
        }
    }

    result.push(TB(TextBlockLayoutHint.kCompoundStructureEnd, [TAG(']')]));
    return result;
}

export function formatObjectValue(value: object, ctx: FormatterContext): Array<TextBlock> {
    // Fast path for null
    if (value == null) {
        return [TB(TextBlockLayoutHint.kValue, [
            TAG('null', TextColor.kBlack, TextColor.kDefault, [TextStyle.kBold])
        ])];
    }

    // User-defined formatter
    if (value[kObjectFormatter] != undefined) {
        return value[kObjectFormatter](ctx);
    }

    if (Array.isArray(value)) {
        return formatArrayValue(value as Array<any>, ctx);
    }

    if (TypeTraits.IsSet(value)) {
        return formatSetValue(value as Set<any>, ctx);
    }

    if (TypeTraits.IsMap(value)) {
        return formatMapValue(value as Map<any, any>, ctx);
    }

    if (TypeTraits.IsTypedArray(value)) {
        return formatTypedArray(value, ctx);
    }

    // TODO(sora): More internal objects (iterator, proxy, promise...)

    // For trivial object
    const result: Array<TextBlock> = [
        TB(TextBlockLayoutHint.kCompoundStructureBegin, [TAG('{')])
    ];
    const propertyNames = Object.getOwnPropertyNames(value);
    let isFirstProp = true;
    for (const key of propertyNames) {
        if (isFirstProp) {
            isFirstProp = false;
        } else {
            result.push(TB(TextBlockLayoutHint.kSeparator, [TAG(',')]));
        }
        result.push(
            TB(TextBlockLayoutHint.kPropertyName, [TAG(key + ':')]),
            ...formatAnyValue(value[key], ctx)
        );
    }
    result.push(TB(TextBlockLayoutHint.kCompoundStructureEnd, [TAG('}')]));
    return result;
}

// Use empty string to fill unused entries
const escapedCtrlChars = [
    '\\x00', '\\x01', '\\x02', '\\x03', '\\x04', '\\x05', '\\x06', '\\x07', // x07
    '\\b', '\\t', '\\n', '\\x0B', '\\f', '\\r', '\\x0E', '\\x0F',           // x0F
    '\\x10', '\\x11', '\\x12', '\\x13', '\\x14', '\\x15', '\\x16', '\\x17', // x17
    '\\x18', '\\x19', '\\x1A', '\\x1B', '\\x1C', '\\x1D', '\\x1E', '\\x1F', // x1F
    '', '', '', '', '', '', '', "\\'", '', '', '', '', '', '', '', '',      // x2F
    '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '',         // x3F
    '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '',         // x4F
    '', '', '', '', '', '', '', '', '', '', '', '', '\\\\', '', '', '',     // x5F
    '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '',         // x6F
    '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '\\x7F',    // x7F
    '\\x80', '\\x81', '\\x82', '\\x83', '\\x84', '\\x85', '\\x86', '\\x87', // x87
    '\\x88', '\\x89', '\\x8A', '\\x8B', '\\x8C', '\\x8D', '\\x8E', '\\x8F', // x8F
    '\\x90', '\\x91', '\\x92', '\\x93', '\\x94', '\\x95', '\\x96', '\\x97', // x97
    '\\x98', '\\x99', '\\x9A', '\\x9B', '\\x9C', '\\x9D', '\\x9E', '\\x9F'  // x9F
];

export function formatStringValue(value: string, ctx: FormatterContext): Array<TextBlock> {
    const block = TB(TextBlockLayoutHint.kValue, [
        TAG('\'', TextColor.kGreen)
    ]);

    const appendSubstr = (prev: number, cur: number, B: TextBlock): number => {
        if (cur <= prev) {
            return;
        }

        // -1 to exclude the open quote '
        const width = B.measureTextCharacters() - 1;
        if (width + (cur - prev) > ctx.options.maximumStringLength) {
            cur = prev + ctx.options.maximumStringLength - width;
            B.contents.push(TAG(value.substring(prev, cur), TextColor.kGreen));
            B.contents.push(TAG('...', TextColor.kBlue));
            return cur;
        } else {
            // Not oversize
            B.contents.push(TAG(value.substring(prev, cur), TextColor.kGreen));
            return -1;
        }
    };

    let prev = 0;
    let oversizeAt = -1;
    for (let i = 0; i < value.length; i++) {
        const code = value.codePointAt(i);
        if (code >= escapedCtrlChars.length || escapedCtrlChars[code].length == 0) {
            continue;
        }

        oversizeAt = appendSubstr(prev, i, block);
        if (oversizeAt > 0) {
            break;
        }

        prev = i + 1;
        block.contents.push(TAG(escapedCtrlChars[code], TextColor.kBlue));
        // -1 to exclude the open quote '
        if (block.measureTextCharacters() - 1 > ctx.options.maximumStringLength) {
            block.contents.pop();
            block.contents.push(TAG('...', TextColor.kBlue));
            oversizeAt = i;
            break;
        }
    }
    if (prev < value.length && oversizeAt < 0) {
        oversizeAt = appendSubstr(prev, value.length, block);
    }

    block.contents.push(TAG('\'', TextColor.kGreen));
    if (oversizeAt >= 0) {
        const remaining = value.length - oversizeAt;
        block.contents.push(TAG(' '));
        block.contents.push(TAG(`(${remaining} more characters)`, TextColor.kCyan,
            TextColor.kDefault, [TextStyle.kItalic, TextStyle.kBold]));
    }

    return [block];
}

export function formatAnyValue(value: any, ctx: FormatterContext): Array<TextBlock> {
    switch (typeof value) {
        case "undefined":
            return [TB(TextBlockLayoutHint.kValue, [
                TAG('undefined', TextColor.kBlackBright)
            ])];
        case "boolean":
            return [TB(TextBlockLayoutHint.kValue, [
                TAG(value.toString(), TextColor.kMagenta)
            ])];
        case "number":
            return [TB(TextBlockLayoutHint.kValue, [
                TAG(ctx.options.numberToString(value as number), TextColor.kMagenta)
            ])];
        case "bigint":
            return [TB(TextBlockLayoutHint.kValue, [
                TAG(ctx.options.bigintToString(value as bigint), TextColor.kMagentaBright)
            ])];
    }

    if (typeof value == 'string') {
        return formatStringValue(value, ctx);
    }

    if (typeof value == 'function') {
        return formatFunctionValue(value as Function);
    }

    if (typeof value == 'object') {
        const circularStub = ctx.detectCompoundValueCircularRef(value);
        if (circularStub) {
            return [TB(TextBlockLayoutHint.kValue, [
                TAG('[', TextColor.kCyan),
                TAG(`Circular *${circularStub.id}`,
                    TextColor.kCyan, TextColor.kDefault, [TextStyle.kItalic]),
                TAG(']', TextColor.kCyan)
            ])];
        }

        const stub = ctx.pushCompoundValueStub(value);
        const blocks = formatObjectValue(value as object, ctx);
        if (stub.refcnt > 0) {
            blocks.unshift(TB(TextBlockLayoutHint.kPrefix, [
                TAG('<', TextColor.kCyan),
                TAG(`ref *${stub.id}`,
                    TextColor.kCyan, TextColor.kDefault, [TextStyle.kItalic]),
                TAG('>', TextColor.kCyan)
            ]));
        }
        ctx.popCompoundValueStub();

        return blocks;
    }

    // TODO(sora): Support `Symbol`

    return [TB(TextBlockLayoutHint.kValue, [
        TAG('[', TextColor.kCyan),
        TAG('Unknown', TextColor.kCyan, TextColor.kDefault, [TextStyle.kItalic]),
        TAG(']', TextColor.kCyan)
    ])];
}

/**
 * Perform formatter pipeline stage 1: format to TextBlocks.
 * @param value         Any JavaScript value to stringify.
 * @param userOptions   Formatting options; provided options will override
 *                      the default values. Not all the options will be used.
 */
export function formatToTextBlocks(value: any, userOptions: ObjectFormatOptions = {}): Array<TextBlock> {
    return formatAnyValue(value, new FormatterContext(userOptions));
}

class LayoutContext {
    public options: ObjectFormatOptions;
    constructor(from: FormatterContext) {
        this.options = from.options;
    }
}

function layoutNewline(nextLineIndentLevel: number, ctx: LayoutContext): string {
    return '\n' + (' '.repeat(nextLineIndentLevel * ctx.options.indentInSpaces) + '');
}

function *iterateCompoundStructure(blocks: Array<TextBlock>, begin: number): Generator<TextBlock> {
    const BEGIN = TextBlockLayoutHint.kCompoundStructureBegin;
    const END = TextBlockLayoutHint.kCompoundStructureEnd;

    if (blocks[begin].hint != BEGIN) {
        return;
    }

    yield blocks[begin];

    let depth = 1;
    for (let i = begin + 1; i < blocks.length; i++) {
        if (blocks[i].hint == BEGIN) {
            depth++;
        } else if (blocks[i].hint == END) {
            depth--;
        }
        yield blocks[i];
        if (depth == 0) {
            break;
        }
    }
}

function layoutTextBlockList(blocks: Array<TextBlock>, ctx: LayoutContext): string {
    let result = '';
    let indentLevel = 0;
    let compoundStructureInline = new Array<boolean>();
    for (let i = 0; i < blocks.length; i++) {
        const cur = blocks[i];
        if (cur.hint == TextBlockLayoutHint.kPrefix) {
            result += cur.composite(!ctx.options.colorANSI);
            if (i != blocks.length - 1) {
                result += ' ';
            }
        } else if (cur.hint == TextBlockLayoutHint.kCompoundStructureBegin) {
            const END = TextBlockLayoutHint.kCompoundStructureEnd;
            result += cur.composite(!ctx.options.colorANSI);

            // Measure the length of compound structure value
            let inline = true;
            let valueWidth = 0;
            for (let B of iterateCompoundStructure(blocks, i)) {
                valueWidth += B.measureTextCharacters();
            }
            inline = valueWidth <= ctx.options.maximumInlineWidth;

            if (!inline && i < blocks.length - 1) {
                indentLevel++;
                result += layoutNewline(indentLevel, ctx);
            }

            if (inline && i + 1 < blocks.length && blocks[i + 1].hint != END) {
                result += ' ';
            }

            compoundStructureInline.push(inline);
        } else if (cur.hint == TextBlockLayoutHint.kCompoundStructureEnd) {
            const inline = compoundStructureInline[compoundStructureInline.length - 1];
            if (inline) {
                result += ' ';
            } else {
                indentLevel--;
                result += layoutNewline(indentLevel, ctx);
            }
            result += cur.composite(!ctx.options.colorANSI);
            compoundStructureInline.pop();
        } else if (cur.hint == TextBlockLayoutHint.kValue) {
            result += cur.composite(!ctx.options.colorANSI);
        } else if (cur.hint == TextBlockLayoutHint.kPropertyName) {
            result += cur.composite(!ctx.options.colorANSI) + ' ';
        } else if (cur.hint == TextBlockLayoutHint.kSeparator) {
            result += cur.composite(!ctx.options.colorANSI);
            const inline = compoundStructureInline[compoundStructureInline.length - 1];
            if (!inline) {
                result += layoutNewline(indentLevel, ctx);
            } else {
                result += ' ';
            }
        } else if (cur.hint == TextBlockLayoutHint.kArrowSeparator) {
            result += ' ' + cur.composite(!ctx.options.colorANSI) + ' ';
        }
    }

    return result;
}

/**
 * Like `JSON.stringify`, it returns a string representation of argument `value`.
 * Compared with `JSON.stringify`, this function generates more detailed output
 * and supports many JavaScript internal objects (e.g. Array, Set, Map, Promise, etc.),
 * and ANSI colored output is also optionally available.
 * Note that the returned string is not in JSON format or any other well-known formats.
 * It is expected to be displayed to humans instead of parsed by programs.
 *
 * @param value         Any JavaScript value to be stringified.
 * @param userOptions   Formatting options; provided options will override
 *                      the corresponding default values.
 */
export function format(value: any, userOptions: ObjectFormatOptions = {}): string {
    const formatterCtx = new FormatterContext(userOptions);
    const textBlocks = formatAnyValue(value, formatterCtx);
    if (textBlocks.length == 0) {
        return '';
    }
    return layoutTextBlockList(textBlocks, new LayoutContext(formatterCtx));
}
