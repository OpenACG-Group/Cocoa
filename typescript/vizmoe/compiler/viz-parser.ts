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

import * as Scanner from './viz-scanner';
import * as primtives from './viz-primitive-types';
import * as AST from './viz-ast-nodes';

export class ParserException extends Error {
    constructor(public token: Scanner.VizToken, msg: string) {
        super(msg);
    }
}

function Cast<T extends R, R>(v: R): T {
    return (v as T);
}

class Parser {
    tokens: Scanner.VizToken[];
    cur: number;

    constructor(tokens: Scanner.VizToken[]) {
        this.tokens = tokens;
        this.cur = 0;
    }

    private moveNextTokenChecked(): void {
        if (this.cur + 1 >= this.tokens.length) {
            throw new ParserException(this.tokens[this.cur], 'Unexpected EOF');
        }
        this.cur++;
    }

    private tryMoveToNext(): boolean {
        if (this.cur + 1 >= this.tokens.length) {
            return false;
        }
        this.cur++;
        return true;
    }

    private currentTokenTypeIs(type: Scanner.VizTokenType): boolean {
        const tok = this.tokens[this.cur];
        return (tok.type == type);
    }

    private currentTokenIsKeyword(kw: string): boolean {
        const tok = this.tokens[this.cur];
        return (tok.type == Scanner.VizTokenType.kIdent && tok.ident == kw);
    }

    private throwCurrentError(msg: string): void {
        throw new ParserException(this.tokens[this.cur], msg);
    }

    // prologue_stmt := 'varying' IDENT1 ':' IDENT2 ';'
    public parsePrologueStmt(): null | AST.StatementNode {
        if (!this.currentTokenIsKeyword('varying')) {
            return null;
        }

        // The statement begins with 'varying', so it must be a varying statement
        this.moveNextTokenChecked();

        if (!this.currentTokenTypeIs(Scanner.VizTokenType.kIdent)) {
            this.throwCurrentError('Unexpected token, expecting an identifier after \'varying\' keyword');
        }

        const node = new AST.VaryingStmtNode(null, null);
        node.varyingName = this.tokens[this.cur].ident;

        // Eat IDENT1
        this.moveNextTokenChecked();
        if (!this.currentTokenTypeIs(Scanner.VizTokenType.kColon)) {
            this.throwCurrentError('Unexpected token, expecting a \':\' in \'varying\' statement');
        }

        // Eat ';'
        this.moveNextTokenChecked();

        if (!this.currentTokenTypeIs(Scanner.VizTokenType.kIdent)) {
            this.throwCurrentError('Unexpected token, expecting an identifier after \':\'');
        }
        node.varyingTypeName = this.tokens[this.cur].ident;

        // Eat IDENT2
        this.moveNextTokenChecked();
        if (!this.currentTokenTypeIs(Scanner.VizTokenType.kSemicolon)) {
            this.throwCurrentError('Unexpected token, expecting an \';\' at the end of statement');
        }

        this.cur++;
        return node;
    }

    // object_decl := OBJECT_NAME '{' object_property* '}'
    private parseObjectDecl(): AST.ObjectDeclNode | null {
        if (!this.currentTokenTypeIs(Scanner.VizTokenType.kObjectName)) {
            return null;
        }
        const node = new AST.ObjectDeclNode(this.tokens[this.cur].ident, new Map(), []);
        // Eat OBJECT_NAME
        this.moveNextTokenChecked();

        if (!this.currentTokenTypeIs(Scanner.VizTokenType.kLBrace)) {
            this.throwCurrentError('Object declaration should be: @Object { ... };');
        }
        // Eat {
        this.moveNextTokenChecked();

        // Parse properties
        this.parseObjectProperties(node);

        if (!this.currentTokenTypeIs(Scanner.VizTokenType.kRBrace)) {
            this.throwCurrentError('Object declaration should be: @Object { ... };');
        }
        
        this.cur++;
        return node;
    }

    // object_property := keyword_property | positional_property
    private parseObjectProperties(object: AST.ObjectDeclNode): void {
        while (this.cur < this.tokens.length) {
            if (!this.parseKeywordProperty(object) && !this.parsePositionalProperty(object)) {
                break;
            }
        }
    }

    // keyword_property := IDENT ':' expr ';'
    private parseKeywordProperty(object: AST.ObjectDeclNode): boolean {
        if (!this.currentTokenTypeIs(Scanner.VizTokenType.kIdent)) {
            return false;
        }
        const keyword = this.tokens[this.cur].ident;
        this.moveNextTokenChecked();

        if (!this.currentTokenTypeIs(Scanner.VizTokenType.kColon)) {
            this.throwCurrentError('Expecting a \':\' after identifier in object keyword property');
        }
        // Eat ':'
        this.moveNextTokenChecked();

        const expr = this.parseExpr();
        if (!expr) {
            this.throwCurrentError('Expecting an expression as value for object keyword property');
        }
        object.keywordProperties.set(keyword, expr);

        if (!this.currentTokenTypeIs(Scanner.VizTokenType.kSemicolon)) {
            this.throwCurrentError('Expecting an \';\' at the end of object keyword property');
        }

        this.cur++;
        return true;
    }

    // positional_property := object_decl
    private parsePositionalProperty(object: AST.ObjectDeclNode): boolean {
        const decl = this.parseObjectDecl();
        if (!decl) {
            return false;
        }
        object.positionalProperties.push(decl);
        return true;
    }

    // expr := object_decl
    private parseExpr(): AST.ExprNode | null {
        let expr = this.parseObjectDecl();
        if (expr) {
            return expr;
        }

        return null;
    }

    // TU := prologue_stmt* object_decl
    public parseTranslationUnit(): AST.TranslationUnitNode {
        const TU = new AST.TranslationUnitNode([], null);
        while (this.cur < this.tokens.length) {
            const stmt = this.parsePrologueStmt();
            if (!stmt) {
                break;
            }
            TU.statements.push(stmt);
        }

        if (this.cur < this.tokens.length) {
            const root = this.parseObjectDecl();
            if (root) {
                TU.rootObject = root;
            }
        }

        if (this.cur != this.tokens.length) {
            this.throwCurrentError('Invalid contents after root object declaration');
        }

        return TU;
    }
}

export function VizParse(input: string): AST.TranslationUnitNode {
    const tokens = Scanner.VizTokenize(input);
    return new Parser(tokens).parseTranslationUnit();
}
