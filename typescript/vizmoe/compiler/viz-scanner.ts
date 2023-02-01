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

import * as pritimives from "./viz-primitive-types";

export enum VizTokenType {
    kUnknown,       //< Placeholder, does not appear in the tokenized result
    kIdent,         //< Identifier
    kObjectName,    //< @Object
    kHexARGB,       //< A hexadecimal ARGB value: #ff66ccff
    kNumeric,       //< A numeric value, optionally associated with a unit: 12px, 2rem
    kLBrace,        //< {
    kRBrace,        //< }
    kLBracket,      //< [
    kRBracket,      //< ]
    kLPar,          //< (
    kRPar,          //< )
    kComma,         //< ,
    kSemicolon,     //< ;
    kColon,         //< :
    kDot,           //< .
    kString         //< "STRING"
}

// Terminator symbols must have a single character
const kTerminatorSymMap = new Map<string, VizTokenType>([
    [ '{', VizTokenType.kLBrace ],
    [ '}', VizTokenType.kRBrace ],
    [ '[', VizTokenType.kLBracket ],
    [ ']', VizTokenType.kRBracket ],
    [ '(', VizTokenType.kLPar ],
    [ ')', VizTokenType.kRPar ],
    [ ',', VizTokenType.kComma ],
    [ ';', VizTokenType.kSemicolon ],
    [ ':', VizTokenType.kColon ],
    [ '.', VizTokenType.kDot ]
]);

const kNumericUnitNamesMap = new Map<string, pritimives.VizNumericValueUnit>([
    [ "px", pritimives.VizNumericValueUnit.kPx ],
    [ 'em', pritimives.VizNumericValueUnit.kEm ],
    [ 'rem', pritimives.VizNumericValueUnit.kRem ],
    [ 'vw', pritimives.VizNumericValueUnit.kVw ],
    [ 'vh', pritimives.VizNumericValueUnit.kVh ],
    [ '%', pritimives.VizNumericValueUnit.kPercent ]
]);

export interface VizToken {
    type: VizTokenType;

    // A range of lexeme [start, end)
    lexemeRange: [number, number];

    // Only available when `type` is `kNumeric`
    numericValue?: pritimives.VizNumericValue;
    // Only available when `type` is `kHexARGB`
    hexARGBValue?: pritimives.VizHexARGBValue;
    // Only available when `type` is `kString`
    stringLiteral?: string;
    // Only available when `type` is `kIdent` or `kObjectName`
    ident?: string;
}

export class ScannerException extends Error {
    constructor(public pos: number, what: string) {
        super(what);
    }
}

class Scanner {
    input: string;
    cur: number;

    private static CODEPOINT_0: number = '0'.codePointAt(0);
    private static CODEPOINT_9: number = '9'.codePointAt(0);
    private static CODEPOINT_a: number = 'a'.codePointAt(0);
    private static CODEPOINT_z: number = 'z'.codePointAt(0);
    private static CODEPOINT_A: number = 'A'.codePointAt(0);
    private static CODEPOINT_Z: number = 'Z'.codePointAt(0);

    constructor(input: string) {
        this.input = input;
        this.cur = 0;
    }

    private checkNotEOF(): void {
        if (this.cur >= this.input.length) {
            throw new ScannerException(this.cur, 'Unexpected EOF');
        }
    }

    private moveForwardChecked(): void {
        this.cur++;
        this.checkNotEOF();
    }

    private peekNextChar(): undefined | string {
        return this.input[this.cur + 1];
    }

    // Begining at `cur`, `input` matches the given string partially
    private remainingInputPartiallyMatch(s: string): boolean {
        if (s.length > this.input.length - this.cur) {
            return false;
        }
        return (s == this.input.substring(this.cur, this.cur + s.length));
    }

    // Try matching a string literal, moving `cur` to the start of next token
    private matchStringLiteral(): VizToken | null {
        if (this.input[this.cur] != '"' && this.input[this.cur] != '\'') {
            return null;
        }

        const closeStringTag = this.input[this.cur];
        // Eat a " or ' (string open tag)
        this.moveForwardChecked();

        const lexemeStart = this.cur;
        let literal = '';

        while (this.input[this.cur] != closeStringTag) {
            // Escape characters
            if (this.input[this.cur] == '\\') {
                this.moveForwardChecked();
                switch (this.input[this.cur]) {
                    case 'b':
                        literal += '\b';
                        break;
                    case 'f':
                        literal += '\f';
                        break;
                    case 'n':
                        literal += '\n';
                        break;
                    case 't':
                        literal += '\t';
                        break;
                    case 'u':
                        // TODO(sora): 16bit hex escape
                        break;
                    case 'v':
                        literal += '\v';
                        break;
                    case 'x':
                        // TODO(sora): 8bit hex escape
                        break;
                    default:
                        literal += this.input[this.cur];
                        break;
                }
            } else {
                literal += this.input[this.cur];
            }
            this.moveForwardChecked();
        }

        // Eat a " or ' (string close tag)
        this.cur++;
        return {
            type: VizTokenType.kString,
            lexemeRange: [lexemeStart, this.cur],
            stringLiteral: literal
        };
    }

    private firstCharIsDigit(str: string): boolean {
        const code = str.codePointAt(0);
        return (code >= Scanner.CODEPOINT_0 && code <= Scanner.CODEPOINT_9);
    }

    private charIsAlpha(str: string, pos: number): boolean {
        const cp = str.codePointAt(pos);
        return (cp >= Scanner.CODEPOINT_a && cp <= Scanner.CODEPOINT_z) ||
               (cp >= Scanner.CODEPOINT_A && cp <= Scanner.CODEPOINT_Z);
    }

    private charIsDigit(str: string, pos: number): boolean {
        const cp = str.codePointAt(pos);
        return (cp >= Scanner.CODEPOINT_0 && cp <= Scanner.CODEPOINT_9);
    }

    // Try matching a numeric value, moving `cur` to the start of next token
    private matchNumericLiteral(): null | VizToken {
        let ch = this.input[this.cur];
        if (ch != '-' && ch != '.' && !this.firstCharIsDigit(ch)) {
            return null;
        }
        
        const tokenStartPos = this.cur;

        let sign: number = 1;
        let isMantissaPart: boolean = false;

        if (ch == '-')
        {
            const next = this.peekNextChar();
            if (next == undefined || (!this.firstCharIsDigit(next) && next != '.')) {
                return null;
            }
            sign = -1;
            this.moveForwardChecked();
        }
        else if (ch == '.')
        {
            const next = this.peekNextChar();
            if (next == undefined || !this.firstCharIsDigit(next)) {
                return null;
            }
            isMantissaPart = true;
            this.moveForwardChecked();
        }

        let integerValue: number = 0;
        let mantissaValue: number = 0;
        let mantissaScalar: number = 10;
        while (this.cur < this.input.length) {
            ch = this.input[this.cur];
            if (ch == '.') {
                if (isMantissaPart) {
                    throw new ScannerException(this.cur, 'Multiple \'.\' in a numeric value');
                }
                isMantissaPart = true;
            } else if (this.firstCharIsDigit(ch)) {
                const digit: number = (ch.codePointAt(0) - Scanner.CODEPOINT_0);
                if (isMantissaPart) {
                    mantissaValue += digit / mantissaScalar;
                    mantissaScalar *= 10;
                    // FIXME(sora): `mantissaScalar` may overflow
                } else {
                    integerValue = integerValue * 10 + digit;
                    if (integerValue >= Number.MAX_SAFE_INTEGER) {
                        throw new ScannerException(tokenStartPos, 'Numeric value is too large (bigger than 2^53 - 1)');
                    }
                }
            } else {
                break;
            }
            this.cur++;
        }

        let decimalValue: number = integerValue;
        if (isMantissaPart) {
            decimalValue += mantissaValue;
        }
        decimalValue *= sign;

        // Try match a unit
        let unit = pritimives.VizNumericValueUnit.kNone;
        for (const [unitStr, unitType] of kNumericUnitNamesMap) {
            if (this.remainingInputPartiallyMatch(unitStr)) {
                unit = unitType;
                this.cur += unitStr.length;
                break;
            }
        }

        const numeric = new pritimives.VizNumericValue(
            decimalValue,
            unit,
            isMantissaPart ? pritimives.VizNumericValueType.kFloat
                           : pritimives.VizNumericValueType.kInteger
        );
        return {
            type: VizTokenType.kNumeric,
            lexemeRange: [tokenStartPos, this.cur],
            numericValue: numeric
        };
    }

    // Try matching a hexadecimal ARGB literal, moving `cur` to the start of next token
    private matchHexARGBLiteral(): null | VizToken {
        let ch = this.input[this.cur];
        if (ch != '#') {
            return null;
        }
        const startPos = this.cur;
        // Consume #
        this.moveForwardChecked();

        // [v1, v2, v3, v4] where v1,v2,v3,v4∈[0, 255]; -1 is a placeholder.
        // How values will be reinterpreted as ARGB components depends on the number
        // of valid values.
        const G = [-1, -1, -1, -1];

        const parseHexDigit = (str: string, pos: number): number | null => {
            const cp = str.codePointAt(pos);
            if (cp >= Scanner.CODEPOINT_0 && cp <= Scanner.CODEPOINT_9) {
                return (cp - Scanner.CODEPOINT_0);
            }
            if (cp >= Scanner.CODEPOINT_a && cp <= Scanner.CODEPOINT_z) {
                return (cp - Scanner.CODEPOINT_a) + 10;
            }
            if (cp >= Scanner.CODEPOINT_A && cp <= Scanner.CODEPOINT_Z) {
                return (cp - Scanner.CODEPOINT_A) + 10;
            }
            return null;
        }

        const parseNextHexByte = (): number | null => {
            const hb = parseHexDigit(this.input, this.cur);
            if (hb == null) {
                return null;
            }
            this.moveForwardChecked();
            const lb = parseHexDigit(this.input, this.cur);
            if (lb == null) {
                throw new ScannerException(this.cur, 'Invalid hexadecimal packed ARGB value');
            }
            this.cur++;
            return (hb << 4 | lb);
        }

        let g = 0;
        for (; g < 4; g++) {
            const v = parseNextHexByte();
            if (v == null) {
                break;
            }
            G[g] = v;
        }

        let literal: pritimives.VizHexARGBValue;
        if (g == 1) {
            literal = new pritimives.VizHexARGBValue(0xff, G[0], G[0], G[0]);
        } else if (g == 3) {
            literal = new pritimives.VizHexARGBValue(0xff, G[0], G[1], G[2]);
        } else if (g == 4) {
            literal = new pritimives.VizHexARGBValue(G[0], G[1], G[2], G[3]);
        } else {
            throw new ScannerException(this.cur, 'Invalid hexadecimal packed ARGB value');
        }

        return {
            type: VizTokenType.kHexARGB,
            lexemeRange: [startPos, this.cur],
            hexARGBValue: literal
        };
    }

    // Try matching an identifier, moving `cur` to the start of next token
    private matchIdentifier(): VizToken | null {
        if (!this.charIsAlpha(this.input, this.cur) && this.input[this.cur] != '_') {
            return null;
        }
        // Save and eat first character in identifier
        const startPos = this.cur;
        this.moveForwardChecked();

        while (this.cur < this.input.length) {
            if (!this.charIsAlpha(this.input, this.cur) && !this.charIsDigit(this.input, this.cur) &&
                this.input[this.cur] != '_')
            {
                break;
            }
            this.cur++;
        }

        return {
            type: VizTokenType.kIdent,
            lexemeRange: [startPos, this.cur],
            ident: this.input.substring(startPos, this.cur)
        };
    }

    // Try matching an object name, moving `cur` to the start of next token
    private matchObjectName(): VizToken | null {
        if (this.input[this.cur] != '@') {
            return null;
        }
        this.moveForwardChecked();

        const ident = this.matchIdentifier();
        if (!ident) {
            throw new ScannerException(this.cur, 'Invalid object name');
        }

        return {
            type: VizTokenType.kObjectName,
            lexemeRange: [ident.lexemeRange[0] - 1, ident.lexemeRange[1]],
            ident: ident.ident
        };
    }

    public tokenize(): Array<VizToken> {
        const toks = new Array<VizToken>();

        const literalMatchers = [
            this.matchHexARGBLiteral.bind(this),
            this.matchStringLiteral.bind(this),
            this.matchNumericLiteral.bind(this),
            this.matchObjectName.bind(this),
            this.matchIdentifier.bind(this)
        ];
        while (this.cur < this.input.length) {
            // Skip whitespaces
            const ch = this.input[this.cur];
            if (ch == ' ' || ch == '\t' || ch == '\n') {
                this.cur++;
                continue;
            }

            // Try matching literals
            let token: VizToken | null = null;
            for (const matcher of literalMatchers) {
                token = matcher();
                if (token) {
                    break;
                }
            }
            if (token) {
                toks.push(token);
                continue;
            }

            // Try matching terminator symbols
            let type = VizTokenType.kUnknown;
            for (const [term, termType] of kTerminatorSymMap) {
                if (this.input[this.cur] == term[0]) {
                    type = termType;
                    break;
                }
            }
            if (type != VizTokenType.kUnknown) {
                toks.push({
                    type: type,
                    lexemeRange: [this.cur, ++this.cur]
                });
                continue;
            }

            throw new ScannerException(this.cur, `Unexpected character ${this.input[this.cur]}`);
        }
        return toks;
    }
}

export function VizTokenize(input: string): Array<VizToken> {
    return new Scanner(input).tokenize();
}
