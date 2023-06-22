import * as core from 'core';

interface FmtLexResult {
    char: string;
    index: number;
    lookahead: string;
    putback(): void;
}

function* formatLexIterator(fmt: string): Generator<FmtLexResult> {
    for (let i = 0; i < fmt.length; i++) {
        let next: string = null;
        if (i + 1 < fmt.length) {
            next = fmt[i + 1];
        }
        yield {
            char: fmt[i],
            index: i,
            lookahead: next,
            putback: () => { i--; }
        };
    }
}

enum FmtAlignment {
    LEFT = '<',
    RIGHT = '>',
    CENTER = '^'
}

enum FmtSign {
    BOTH = '+',
    NEGATIVE_ONLY = '-',
    SPACE = ' '
}

enum FmtType {
    DECIMAL = 'd',
    BINARY = 'b',
    BINARY_UPPER = 'B',
    HEX = 'x',
    HEX_UPPER = 'X',
    OCTAL = 'o',
    FLOAT = 'f',
    FLOAT_EXP = 'e',
    FLOAT_EXP_UPPER = 'E',
    BOOLEAN = 'l',
    STRING = 's',
    TYPEINFO = 't',
    FUNCTION = 'F'
}

interface FmtFormatSpec {
    fillChar: string;
    alignment: FmtAlignment;
    sign: FmtSign;
    alternateForm: boolean;
    signAwareZeroPadding: boolean;
    width: number;
    precision: number;
    insertNumberSeparator: boolean;
    type: FmtType;
}

interface FmtDecorator {
    matchType: FmtType,
    callback: (spec: FmtFormatSpec, index: number, value: any) => string;
}

type FmtLexAction = (opt: FmtFormatSpec, group: string, input: string) => void;

const _specRegExp = /([^{}][\^<>])?([\s+-])?(#)?(0)?([1-9][0-9]*)?(\.[0-9]+)?(L)?([dbBxXofeElstF])?/;

function stringFillAlignment(spec: FmtFormatSpec, str: string): string {
    if (spec.width == undefined || str.length >= spec.width) {
        return str;
    }
    let remain = spec.width - str.length;
    switch (spec.alignment) {
        case FmtAlignment.CENTER:
            let l = Math.floor(remain / 2);
            let r = remain - l;
            return spec.fillChar.repeat(l) + str + spec.fillChar.repeat(r);
        case FmtAlignment.LEFT:
            return str + spec.fillChar.repeat(remain);
        case FmtAlignment.RIGHT:
            return spec.fillChar.repeat(remain) + str;
        default:
            throw new Error("Invalid alignment method");
    }
}

function normalizeNumber(x: number): [number, boolean] {
    x = Math.floor(x);
    if (x < 0) {
        return [-x, true];
    } else {
        return [x, false];
    }
}

function getSignString(sign: FmtSign, neg: boolean): string {
    if (sign == FmtSign.NEGATIVE_ONLY) {
        return (neg ? '-' : '');
    } else if (sign == FmtSign.BOTH) {
        return (neg ? '-' : '+');
    } else if (sign == FmtSign.SPACE) {
        return (neg ? '-' : ' ');
    }
}

function stackPopToString(spec: FmtFormatSpec, neg: boolean, st: number[],
                          lexicalBits: string[], prefix?: string): string {
    let result: string = getSignString(spec.sign, neg);
    if (prefix != undefined) {
        result += prefix;
    }
    if (spec.signAwareZeroPadding && spec.width > result.length + st.length) {
        let padding = spec.width - result.length - st.length;
        result += '0'.repeat(padding);
    }
    while (st.length > 0) {
        result += lexicalBits[st.pop()];
    }
    return result;
}

let digits: string[] = [
    '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f'
];
function toDecimal(_x: number, spec: FmtFormatSpec): string {
    let [x, neg] = normalizeNumber(_x);

    let st: number[] = [];
    while (x) {
        st.push(x % 10);
        x = Math.floor(x / 10);
    }
    return stringFillAlignment(spec, stackPopToString(spec, neg, st, digits));
}

function toBinary(_x: number, spec: FmtFormatSpec): string {
    let [x, neg] = normalizeNumber(_x);
    let st: boolean[] = [];
    while (x) {
        st.push((x & 1) == 1);
        x >>>= 1;
    }
    let result: string = getSignString(spec.sign, neg);
    if (spec.alternateForm) {
        result += spec.type == FmtType.BINARY ? "0b" : "0B";
    }

    if (spec.signAwareZeroPadding && spec.width > result.length + st.length) {
        let padding = spec.width - result.length - st.length;
        result += '0'.repeat(padding);
    }
    while (st.length > 0) {
        result += st.pop() ? '1' : '0';
    }
    return stringFillAlignment(spec, result);
}

function toHexadecimal(_x: number, spec: FmtFormatSpec): string {
    let [x, neg] = normalizeNumber(_x);
    let st: number[] = [];
    while (x) {
        st.push(x & 15);
        x >>>= 4;
    }
    let result: string = stackPopToString(
        spec, neg, st,
        digits,
        spec.alternateForm ? "0x" : undefined);
    return stringFillAlignment(spec,
        spec.type == FmtType.HEX_UPPER ? result.toUpperCase() : result);
}

function toFloatFixed(_x: number, spec: FmtFormatSpec): string {
    let x = Math.abs(_x);
    let neg = _x < 0;
    let fixed: string;
    if (spec.precision == undefined) {
        fixed = x.toString();
    } else {
        fixed = x.toFixed(spec.precision);
    }
    let prefix: string = getSignString(spec.sign, neg);
    if (spec.signAwareZeroPadding && spec.width > prefix.length + fixed.length) {
        let padding = spec.width - prefix.length - fixed.length;
        prefix += '0'.repeat(padding);
    }
    return stringFillAlignment(spec, prefix + fixed);
}

function valueTypeCheck(value: any, index: number, expect: string): void {
    if (typeof value != expect) {
        throw new TypeError(`Unexpected type of argument #${index}, expecting a ${expect}`);
    }
}

const _decorators: FmtDecorator[] = [
    {
        matchType: FmtType.STRING,
        callback: (spec, index, value) => {
            valueTypeCheck(value, index, "string");
            return stringFillAlignment(spec, value);
        }
    },
    {
        matchType: FmtType.BOOLEAN,
        callback: (spec, index, value) => {
            valueTypeCheck(value, index, "boolean");
            return stringFillAlignment(spec, value as boolean ? "true" : "false");
        }
    },
    {
        matchType: FmtType.TYPEINFO,
        callback: (spec, index, value) => {
            return stringFillAlignment(spec, typeof value);
        }
    },
    {
        matchType: FmtType.FUNCTION,
        callback: (spec, index, value) => {
            valueTypeCheck(value, index, "function");
            let ident: string = value.toString();
            let pos = ident.indexOf('{');
            if (ident.substr(pos) != "{ [native code] }") {
                ident = `function ${value.name}() { ... }`;
            }
            return stringFillAlignment(spec, ident);
        }
    },
    {
        matchType: FmtType.DECIMAL,
        callback: (spec, index, value) => {
            valueTypeCheck(value, index, "number");
            return toDecimal(value as number, spec);
        }
    },
    {
        matchType: FmtType.BINARY,
        callback: (spec, index, value) => {
            valueTypeCheck(value, index, "number");
            return toBinary(value as number, spec);
        }
    },
    {
        matchType: FmtType.BINARY_UPPER,
        callback: (spec, index, value) => {
            valueTypeCheck(value, index, "number");
            return toBinary(value as number, spec);
        }
    },
    {
        matchType: FmtType.HEX,
        callback: (spec, index, value) => {
            valueTypeCheck(value, index, "number");
            return toHexadecimal(value as number, spec);
        }
    },
    {
        matchType: FmtType.HEX_UPPER,
        callback: (spec, index, value) => {
            valueTypeCheck(value, index, "number");
            return toHexadecimal(value as number, spec);
        }
    },
    {
        matchType: FmtType.FLOAT,
        callback: (spec, index, value) => {
            valueTypeCheck(value, index, "number");
            return toFloatFixed(value as number, spec);
        }
    }
];

const _lexActions: FmtLexAction[] = [
    (opt, group, input) => {
        if (group != input) {
            throw new Error("Invalid format spec syntax");
        }
    },
    (opt, group) => {
        opt.fillChar = group[0];
        opt.alignment = group[1] as FmtAlignment;
    },
    (opt, group) => { opt.sign = group as FmtSign; },
    (opt) => { opt.alternateForm = true; },
    (opt) => { opt.signAwareZeroPadding = true; },
    (opt, group) => { opt.width = Number(group); },
    (opt, group) => { opt.precision = Number(group.substr(1)); },
    (opt) => { opt.insertNumberSeparator = true; },
    (opt, group) => { opt.type = group as FmtType; }
];

function formatParseFormatSpec(spec: string): FmtFormatSpec {
    // format_spec ::= [[fill]align][sign]["#"]["0"][width]["." precision]["L"][type]

    let opts: FmtFormatSpec = {
        fillChar: ' ',
        alignment: FmtAlignment.RIGHT,
        sign: FmtSign.NEGATIVE_ONLY,
        alternateForm: false,
        signAwareZeroPadding: false,
        width: undefined,
        precision: undefined,
        insertNumberSeparator: false,
        type: undefined
    };

    let matcher = _specRegExp.exec(spec);
    for (let i = 0; i < matcher.length; i++) {
        if (matcher[i] != undefined) {
            _lexActions[i](opts, matcher[i], matcher.input);
        }
    }

    return opts;
}

function formatParseReplace(itr: Generator<FmtLexResult>, index: number, arg: any): string {
    let specStr: string = "";
    while (true) {
        const current = itr.next();
        if (current.done || current.value == undefined) {
            throw new Error("Unexpected format placeholder ending, expecting a '}'");
        }
        let lexeme: FmtLexResult = current.value;
        if (lexeme.char == '}') {
            break;
        }
        specStr += lexeme.char;
    }
    let spec: FmtFormatSpec = formatParseFormatSpec(specStr);

    let argRealType: FmtType;
    switch (typeof arg) {
        case "number":
            let x = arg as number;
            if (Number.isNaN(x)) {
                return stringFillAlignment(spec, "NaN");
            } else if (!Number.isFinite(x)) {
                return stringFillAlignment(spec, x == Number.POSITIVE_INFINITY ? "inf" : "-inf");
            }
            if (Number.isInteger(x)) {
                argRealType = FmtType.DECIMAL;
            } else {
                argRealType = FmtType.FLOAT;
            }
            break;
        case "string":
            argRealType = FmtType.STRING;
            break;
        case "boolean":
            argRealType = FmtType.BOOLEAN;
            break;
        case "function":
            argRealType = FmtType.FUNCTION;
            break;
        case "object":
        case "undefined":
        case "symbol":
        case "bigint":
            throw new Error("object, bigint, undefined and symbol types are not serializable");
    }
    if (spec.type == undefined) {
        spec.type = argRealType;
    }

    for (const dec of _decorators) {
        if (dec.matchType == spec.type) {
            return dec.callback(spec, index, arg);
        }
    }
    throw new Error("Invalid format spec");
}

export function format(fmt: string, ...rest: any): string {
    if (fmt.length == 0) {
        return "";
    }

    let result = "";
    let iterator = formatLexIterator(fmt);
    let argIndex = 0;
    while (true) {
        const current = iterator.next();
        if (current.done || current.value == undefined) {
            break;
        }
        const lexeme: FmtLexResult = current.value;
        if (lexeme.char == '{' && lexeme.lookahead == '{') {
            result += '{';
            iterator.next();
        } else if (lexeme.char == '}' && lexeme.lookahead == '}') {
            result += '}';
            iterator.next();
        } else if (lexeme.char == '{') {
            if (argIndex >= rest.length) {
                throw new Error("Format arguments not enough");
            }
            result += formatParseReplace(iterator, argIndex, rest[argIndex]);
            argIndex++;
        } else if (lexeme.char == '}') {
            throw new Error("Unexpected '}', expecting after '{'");
        } else {
            result += lexeme.char;
        }
    }

    if (argIndex < rest.length) {
        throw new Error("Too many format arguments");
    }

    return result;
}

export function printf(fmt: string, ...rest: any): void {
    core.print(format(fmt, ...rest));
}
