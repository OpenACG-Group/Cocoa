import * as core from 'core';
function* formatLexIterator(fmt) {
    for (let i = 0; i < fmt.length; i++) {
        let next = null;
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
var FmtAlignment;
(function (FmtAlignment) {
    FmtAlignment["LEFT"] = "<";
    FmtAlignment["RIGHT"] = ">";
    FmtAlignment["CENTER"] = "^";
})(FmtAlignment || (FmtAlignment = {}));
var FmtSign;
(function (FmtSign) {
    FmtSign["BOTH"] = "+";
    FmtSign["NEGATIVE_ONLY"] = "-";
    FmtSign["SPACE"] = " ";
})(FmtSign || (FmtSign = {}));
var FmtType;
(function (FmtType) {
    FmtType["DECIMAL"] = "d";
    FmtType["BINARY"] = "b";
    FmtType["BINARY_UPPER"] = "B";
    FmtType["HEX"] = "x";
    FmtType["HEX_UPPER"] = "X";
    FmtType["OCTAL"] = "o";
    FmtType["FLOAT"] = "f";
    FmtType["FLOAT_EXP"] = "e";
    FmtType["FLOAT_EXP_UPPER"] = "E";
    FmtType["BOOLEAN"] = "l";
    FmtType["STRING"] = "s";
    FmtType["TYPEINFO"] = "t";
    FmtType["FUNCTION"] = "F";
})(FmtType || (FmtType = {}));
const _specRegExp = /([^{}][\^<>])?([\s+-])?(#)?(0)?([1-9][0-9]*)?(\.[0-9]+)?(L)?([dbBxXofeElstF])?/;
function stringFillAlignment(spec, str) {
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
function normalizeNumber(x) {
    x = Math.floor(x);
    if (x < 0) {
        return [-x, true];
    }
    else {
        return [x, false];
    }
}
function getSignString(sign, neg) {
    if (sign == FmtSign.NEGATIVE_ONLY) {
        return (neg ? '-' : '');
    }
    else if (sign == FmtSign.BOTH) {
        return (neg ? '-' : '+');
    }
    else if (sign == FmtSign.SPACE) {
        return (neg ? '-' : ' ');
    }
}
function stackPopToString(spec, neg, st, lexicalBits, prefix) {
    let result = getSignString(spec.sign, neg);
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
let digits = [
    '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f'
];
function toDecimal(_x, spec) {
    let [x, neg] = normalizeNumber(_x);
    let st = [];
    while (x) {
        st.push(x % 10);
        x = Math.floor(x / 10);
    }
    return stringFillAlignment(spec, stackPopToString(spec, neg, st, digits));
}
function toBinary(_x, spec) {
    let [x, neg] = normalizeNumber(_x);
    let st = [];
    while (x) {
        st.push((x & 1) == 1);
        x >>>= 1;
    }
    let result = getSignString(spec.sign, neg);
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
function toHexadecimal(_x, spec) {
    let [x, neg] = normalizeNumber(_x);
    let st = [];
    while (x) {
        st.push(x & 15);
        x >>>= 4;
    }
    let result = stackPopToString(spec, neg, st, digits, spec.alternateForm ? "0x" : undefined);
    return stringFillAlignment(spec, spec.type == FmtType.HEX_UPPER ? result.toUpperCase() : result);
}
function toFloatFixed(_x, spec) {
    let x = Math.abs(_x);
    let neg = _x < 0;
    let fixed;
    if (spec.precision == undefined) {
        fixed = x.toString();
    }
    else {
        fixed = x.toFixed(spec.precision);
    }
    let prefix = getSignString(spec.sign, neg);
    if (spec.signAwareZeroPadding && spec.width > prefix.length + fixed.length) {
        let padding = spec.width - prefix.length - fixed.length;
        prefix += '0'.repeat(padding);
    }
    return stringFillAlignment(spec, prefix + fixed);
}
function valueTypeCheck(value, index, expect) {
    if (typeof value != expect) {
        throw new TypeError(`Unexpected type of argument #${index}, expecting a ${expect}`);
    }
}
const _decorators = [
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
            return stringFillAlignment(spec, value ? "true" : "false");
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
            let ident = value.toString();
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
            return toDecimal(value, spec);
        }
    },
    {
        matchType: FmtType.BINARY,
        callback: (spec, index, value) => {
            valueTypeCheck(value, index, "number");
            return toBinary(value, spec);
        }
    },
    {
        matchType: FmtType.BINARY_UPPER,
        callback: (spec, index, value) => {
            valueTypeCheck(value, index, "number");
            return toBinary(value, spec);
        }
    },
    {
        matchType: FmtType.HEX,
        callback: (spec, index, value) => {
            valueTypeCheck(value, index, "number");
            return toHexadecimal(value, spec);
        }
    },
    {
        matchType: FmtType.HEX_UPPER,
        callback: (spec, index, value) => {
            valueTypeCheck(value, index, "number");
            return toHexadecimal(value, spec);
        }
    },
    {
        matchType: FmtType.FLOAT,
        callback: (spec, index, value) => {
            valueTypeCheck(value, index, "number");
            return toFloatFixed(value, spec);
        }
    }
];
const _lexActions = [
    (opt, group, input) => {
        if (group != input) {
            throw new Error("Invalid format spec syntax");
        }
    },
    (opt, group) => {
        opt.fillChar = group[0];
        opt.alignment = group[1];
    },
    (opt, group) => { opt.sign = group; },
    (opt) => { opt.alternateForm = true; },
    (opt) => { opt.signAwareZeroPadding = true; },
    (opt, group) => { opt.width = Number(group); },
    (opt, group) => { opt.precision = Number(group.substr(1)); },
    (opt) => { opt.insertNumberSeparator = true; },
    (opt, group) => { opt.type = group; }
];
function formatParseFormatSpec(spec) {
    // format_spec ::= [[fill]align][sign]["#"]["0"][width]["." precision]["L"][type]
    let opts = {
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
function formatParseReplace(itr, index, arg) {
    let specStr = "";
    while (true) {
        const current = itr.next();
        if (current.done || current.value == undefined) {
            throw new Error("Unexpected format placeholder ending, expecting a '}'");
        }
        let lexeme = current.value;
        if (lexeme.char == '}') {
            break;
        }
        specStr += lexeme.char;
    }
    let spec = formatParseFormatSpec(specStr);
    let argRealType;
    switch (typeof arg) {
        case "number":
            let x = arg;
            if (Number.isNaN(x)) {
                return stringFillAlignment(spec, "NaN");
            }
            else if (!Number.isFinite(x)) {
                return stringFillAlignment(spec, x == Number.POSITIVE_INFINITY ? "inf" : "-inf");
            }
            if (Number.isInteger(x)) {
                argRealType = FmtType.DECIMAL;
            }
            else {
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
export function format(fmt, ...rest) {
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
        const lexeme = current.value;
        if (lexeme.char == '{' && lexeme.lookahead == '{') {
            result += '{';
            iterator.next();
        }
        else if (lexeme.char == '}' && lexeme.lookahead == '}') {
            result += '}';
            iterator.next();
        }
        else if (lexeme.char == '{') {
            if (argIndex >= rest.length) {
                throw new Error("Format arguments not enough");
            }
            result += formatParseReplace(iterator, argIndex, rest[argIndex]);
            argIndex++;
        }
        else if (lexeme.char == '}') {
            throw new Error("Unexpected '}', expecting after '{'");
        }
        else {
            result += lexeme.char;
        }
    }
    if (argIndex < rest.length) {
        throw new Error("Too many format arguments");
    }
    return result;
}
export function printf(fmt, ...rest) {
    core.print(format(fmt, ...rest));
}
