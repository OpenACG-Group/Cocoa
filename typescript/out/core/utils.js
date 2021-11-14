/**
 * Copy bytes from one Uint8Array to another.  Bytes from `src` which don't fit
 * into `dst` will not be copied.
 *
 * @param src Source byte array
 * @param dst Destination byte array
 * @param off Offset into `dst` at which to begin writing values from `src`.
 * @return number of bytes copied
 */
export function copy(src, dst, off = 0) {
    off = Math.max(0, Math.min(off, dst.byteLength));
    const dstBytesAvailable = dst.byteLength - off;
    if (src.byteLength > dstBytesAvailable) {
        src = src.subarray(0, dstBytesAvailable);
    }
    dst.set(src, off);
    return src.byteLength;
}
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
    FmtAlignment[FmtAlignment["LEFT"] = 0] = "LEFT";
    FmtAlignment[FmtAlignment["RIGHT"] = 1] = "RIGHT";
    FmtAlignment[FmtAlignment["CENTER"] = 2] = "CENTER";
})(FmtAlignment || (FmtAlignment = {}));
var FmtSign;
(function (FmtSign) {
    FmtSign[FmtSign["BOTH"] = 0] = "BOTH";
    FmtSign[FmtSign["NEGATIVE_ONLY"] = 1] = "NEGATIVE_ONLY";
    FmtSign[FmtSign["AUTO"] = 2] = "AUTO";
})(FmtSign || (FmtSign = {}));
function formatParseReplace(itr, arg) {
    // format_spec ::= [[fill]align][sign]["#"]["0"][width]["." precision]["L"][type]
    let opts = {
        fillChar: undefined,
        alignment: FmtAlignment.RIGHT,
        sign: FmtSign.NEGATIVE_ONLY,
        alternateForm: false,
        signAwareZeroPadding: false,
        width: undefined,
        precision: undefined,
        insertNumberSeparator: undefined,
        type: undefined
    };
    while (true) {
        const current = itr.next();
        if (current.done || current.value == undefined) {
            throw new Error("Unexpected format placeholder ending, expecting a '}'");
        }
        let lexeme = current.value;
        if (lexeme.char == '}') {
            break;
        }
    }
    return "";
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
        if (lexeme.char == '{' || lexeme.lookahead == '{') {
            result += '{';
            iterator.next();
        }
        else if (lexeme.char == '}' || lexeme.lookahead == '}') {
            result += '}';
            iterator.next();
        }
        else if (lexeme.char == '{') {
            if (argIndex >= rest.length) {
                throw new Error("Format arguments not enough");
            }
            result += formatParseReplace(iterator, rest[argIndex++]);
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
function printf(fmt, ...rest) {
    Cocoa.core.print(format(fmt, rest));
}
