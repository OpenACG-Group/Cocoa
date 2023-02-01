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

export enum NodeType {
    kTranslationUnit,
    kStatement,
    kExpr
}

export class BaseNode {
    protected constructor(public type: NodeType) {}
}

export class TranslationUnitNode extends BaseNode {
    constructor(public statements: Array<StatementNode>,
                public rootObject: ObjectDeclNode) {
        super(NodeType.kTranslationUnit);
    }
}

export enum StatementType {
    kVarying
}

export class StatementNode extends BaseNode {
    protected constructor(public statementType: StatementType) {
        super(NodeType.kStatement);
    }
}

// Statement: varying <name>: <type>
export class VaryingStmtNode extends StatementNode {
    constructor(public varyingName: string,
                public varyingTypeName: string) {
        super(StatementType.kVarying);
    }
}

export enum ExprType {
    kObjectDecl
}

// An expression which can give a certain value by evaluation
export class ExprNode extends BaseNode {
    protected constructor(public exprType: ExprType) {
        super(NodeType.kExpr);
    }
}

// Object declaration: <NAME> { ... };
export class ObjectDeclNode extends ExprNode {
    constructor(public objectName: string,
                public keywordProperties: Map<string, ExprNode>,
                public positionalProperties: Array<ExprNode>) {
        super(ExprType.kObjectDecl);
    }
}
