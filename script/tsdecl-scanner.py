#!/usr/bin/env python3

# This file is part of Cocoa.
#
# Cocoa is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
#
# Cocoa is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Cocoa. If not, see <https://www.gnu.org/licenses/>.

from typing import Literal, Callable
from enum import Enum
import sys
import os
import re


TSDECL_PREFIX = '//! TSDecl: '
TSDOC_BEGIN = '//! @tsdocbegin'
TSDOC_BODY_PREFIX = '//!'
TSDOC_END = '//! @tsdocend'


class Tokens(Enum):
    KW_END = re.compile('@end')
    KW_INTERFACE = re.compile('@interface')
    KW_CLASS = re.compile('@class')
    KW_ENUMITEM = re.compile('@enumitem')
    KW_ENUM = re.compile('@enum')
    KW_TYPEDEF = re.compile('@typedef')
    KW_METHOD = re.compile('@method')
    KW_PROPERTY = re.compile('@property')
    KW_FUNCTION = re.compile('@function')
    KW_CONSTRUCTOR = re.compile('@constructor')
    HINT_STATIC = re.compile('@static')
    HINT_EXTENDS = re.compile('@extends')
    HINT_IMPL = re.compile('@impl')
    HINT_PRIV = re.compile('@priv')
    HINT_READONLY = re.compile('@readonly')
    HINT_TUPLE = re.compile('@tuple')
    HINT_MEM = re.compile('@mem')
    HINT_IMPORT = re.compile('@import')
    HINT_FUNCTION = re.compile('@fn')
    HINT_ARRAY = re.compile('@array')
    HINT_NON_CONSTRUCTIBLE = re.compile('@nonconstructible')
    HINT_PROT_CONSTRUCTIBLE = re.compile('@protconstructible')
    HINT_GENERIC = re.compile('@generic')
    HINT_PROMISE = re.compile('@promise')
    HINT_UNION = re.compile('@union')
    HINT_OPTIONAL = re.compile('@optional')
    OP_COMMA = re.compile(',')
    OP_OR = re.compile('\\|')
    OP_COLON = re.compile(':')
    OP_QUES = re.compile('\\?')
    OP_MUL = re.compile('\\*')
    OP_EQ = re.compile('=')
    PAR_L = re.compile('\\(')
    PAR_R = re.compile('\\)')
    BRACKET_L = re.compile('\\[')
    BRACKET_R = re.compile(']')
    V_INTEGER = re.compile(r'[1-9][0-9]*')
    V_IDENT = re.compile(r'[a-zA-Z_/][a-zA-Z_0-9/]*')
    # This regex is just a placeholder, will not be used for matching
    COMMENT_LINES = re.compile('COMMENT_LINES')

    WHITESPACE = re.compile(' +')
    NEWLINE = re.compile('\n+')


TOKEN_TYPEGRP_STARTDECL = [Tokens.KW_INTERFACE, Tokens.KW_CLASS, Tokens.KW_ENUM, Tokens.KW_TYPEDEF,
                           Tokens.KW_FUNCTION]
TOKEN_TYPEGRP_HINTS = [Tokens.HINT_STATIC, Tokens.HINT_TUPLE, Tokens.HINT_MEM, Tokens.HINT_IMPORT,
                       Tokens.HINT_READONLY, Tokens.HINT_FUNCTION, Tokens.HINT_ARRAY,
                       Tokens.HINT_IMPL, Tokens.HINT_PRIV, Tokens.HINT_EXTENDS,
                       Tokens.HINT_NON_CONSTRUCTIBLE, Tokens.HINT_GENERIC, Tokens.HINT_PROMISE,
                       Tokens.HINT_UNION, Tokens.HINT_OPTIONAL, Tokens.HINT_PROT_CONSTRUCTIBLE]

BUILTIN_TYPES_MAP: dict[str, str] = {
    'i8': 'number',
    'u8': 'number',
    'i16': 'number',
    'u16': 'number',
    'i32': 'number',
    'u32': 'number',
    'i64': 'number',
    'u64': 'number',
    'f32': 'number',
    'f64': 'number',
    'bigint': 'bigint',
    'boolean': 'boolean',
    'object': 'object',
    'string': 'string',
    'void': 'void',
    'any': 'any',
    'null': 'null',
    'undefined': 'undefined',

    'Promise': 'Promise',
    'Array': 'Array',
    'Map': 'Map',
    'Set': 'Set',
    'WeakMap': 'WeakMap',
    'WeakSet': 'WeakSet',
    'ArrayLike': 'ArrayLike',
    'Proxy': 'Proxy',

    'Generator': 'Generator'
}

NUMERIC_TO_TYPED_ARRAY_MAP: dict[str, str] = {
    'i8': 'Int8Array',
    'u8': 'Uint8Array',
    'i16': 'Int16Array',
    'u16': 'Uint16Array',
    'i32': 'Int32Array',
    'u32': 'Uint32Array',
    'i64': 'BigInt64Array',
    'u64': 'BigUint64Array',
    'f32': 'Float32Array',
    'f64': 'Float64Array',
    'any': 'TypedArray',
    'raw': 'ArrayBuffer'
}


class TokenInfo:
    # `value` is `list[str]` only if `type` is `COMMENT_LINES`
    def __init__(self, type: Tokens, value: str | int | list[str], file: str, lineno: int):
        self.type = type
        self.value = value
        self.file = file
        self.lineno = lineno


TypeSymbolKind = Literal['builtin', 'declared', 'imported', 'complex', 'ref']


class TypeSymbol:
    def __init__(self, kind: TypeSymbolKind):
        self.kind = kind

    def __eq__(self, other: 'TypeSymbol') -> bool:
        return self.kind == other.kind and \
               self.name == other.name and \
               self.import_from == other.import_from and \
               self.complex_tuple == other.complex_tuple and \
               self.complex_array == other.complex_array and \
               self.complex_function == other.complex_function and \
               self.complex_mem == other.complex_mem and \
               self.complex_generic == other.complex_generic and \
               self.complex_union == other.complex_union

    kind: TypeSymbolKind
    typeid: int

    name: str | None = None

    # Only available when `kind` is 'ref'
    ref_typeid: int | None = None

    # Only available when `kind` is 'imported'
    import_from: str | None = None

    # Integers are type-id in the symbol-table.
    # Only available when `kind` is 'complex'
    complex_tuple: list[int] | None = None
    complex_array: int | None = None
    complex_function: tuple[int, dict[str, int]] | None = None
    complex_mem: str | None = None
    complex_generic: tuple[int, list[int]] | None = None
    complex_union: list[int] | None = None


def import_url_to_name(import_url: str) -> str:
    tmp = import_url.replace('/', '_')
    return f'_{tmp}'


class PendingTypeSymbol:
    def __eq__(self, other: 'PendingTypeSymbol') -> bool:
        return self.name == other.name and self.import_from == other.import_from

    name: str
    import_from: str | None


class SymbolTable:
    def __init__(self):
        self.defined_table: dict[int, TypeSymbol] = {}
        self.pending_table: dict[int, PendingTypeSymbol] = {}
        self.id_counter = 0

        for builtin_type in BUILTIN_TYPES_MAP:
            sym = TypeSymbol('builtin')
            sym.name = builtin_type
            sym.typeid = self.id_counter
            self.defined_table[sym.typeid] = sym
            self.id_counter += 1

    def define(self, sym: TypeSymbol) -> int:
        for entry in self.defined_table.items():
            if entry[1] == sym:
                if sym.kind != 'complex':
                    raise Exception(f'redefinition of type `{sym.name}` (import from `{sym.import_from}`)')
                else:
                    return entry[0]

        sym.typeid = self.id_counter
        self.id_counter += 1

        self.defined_table[sym.typeid] = sym
        return sym.typeid

    def define_pending(self, sym: PendingTypeSymbol) -> int:
        for typeid, pending_sym in self.pending_table.items():
            if pending_sym == sym:
                return typeid
        self.pending_table[self.id_counter] = sym
        self.id_counter += 1
        return self.id_counter - 1

    def resolve_pending(self) -> None:
        for typeid, sym in self.pending_table.items():
            # External symbol, define it directly
            if sym.import_from is not None:
                defsym = TypeSymbol('imported')
                defsym.typeid = typeid
                defsym.import_from = sym.import_from
                defsym.name = sym.name
                self.defined_table[typeid] = defsym
                continue

            found = False
            for referee_typeid, referee in self.defined_table.items():
                if sym.name != referee.name:
                    continue

                found = True
                defsym = TypeSymbol('ref')
                defsym.typeid = typeid
                defsym.name = referee.name
                defsym.ref_typeid = referee_typeid
                self.defined_table[typeid] = defsym
                break

            if not found:
                raise Exception(f'undefined symbol `{sym.name}`')

        self.pending_table.clear()

    def deref_ref_chain(self, typeid: int) -> int:
        sym = self.defined_table[typeid]
        while sym.kind == 'ref':
            sym = self.defined_table[sym.ref_typeid]

        return sym.typeid

    def eliminate_typerefs(self) -> None:
        new_table: dict[int, TypeSymbol] = {}
        for typeid, sym in self.defined_table.items():
            if sym.kind == 'ref':
                continue

            new_sym = TypeSymbol(sym.kind)
            new_sym.typeid = typeid
            new_sym.name = sym.name
            new_sym.import_from = sym.import_from
            new_sym.ref_typeid = None
            if sym.complex_array is not None:
                new_sym.complex_array = self.deref_ref_chain(sym.complex_array)
            elif sym.complex_tuple is not None:
                new_sym.complex_tuple = [self.deref_ref_chain(p) for p in sym.complex_tuple]
            elif sym.complex_function is not None:
                args_dict: dict[str, int] = {}
                for argname, argtype in sym.complex_function[1].items():
                    args_dict[argname] = self.deref_ref_chain(argtype)

                ret_type = self.deref_ref_chain(sym.complex_function[0])
                new_sym.complex_function = (ret_type, args_dict)
            elif sym.complex_generic is not None:
                generic_type = self.deref_ref_chain(sym.complex_generic[0])
                generic_args = [self.deref_ref_chain(typeid) for typeid in sym.complex_generic[1]]
                new_sym.complex_generic = (generic_type, generic_args)

            elif sym.complex_union is not None:
                new_sym.complex_union = [self.deref_ref_chain(tid) for tid in sym.complex_union]

            new_sym.complex_mem = sym.complex_mem
            new_table[typeid] = new_sym

        self.defined_table = new_table

    def stringify_typename(self, typeid: int) -> str:
        if typeid not in self.defined_table:
            raise Exception(f'use of undefined or eliminated typeid {typeid}')

        sym = self.defined_table[typeid]
        if sym.kind in ['builtin', 'declared']:
            return sym.name

        if sym.kind == 'imported':
            return f'{import_url_to_name(sym.import_from)}.{sym.name}'

        if sym.complex_array is not None:
            return f'{self.stringify_typename(sym.complex_array)}[]'

        if sym.complex_tuple is not None:
            comma_list = ', '.join([self.stringify_typename(tid) for tid in sym.complex_tuple])
            return f'[{comma_list}]'

        if sym.complex_mem is not None:
            return NUMERIC_TO_TYPED_ARRAY_MAP[sym.complex_mem]

        if sym.complex_function is not None:
            ret_type = self.stringify_typename(sym.complex_function[0])
            args_dict = sym.complex_function[1]
            args_list = [f'{name}: {self.stringify_typename(tid)}' for name, tid in args_dict.items()]
            args_comma_list = ', '.join(args_list)
            return f'(({args_comma_list}) => {ret_type})'

        if sym.complex_generic is not None:
            args_comma_list = ', '.join([self.stringify_typename(tid) for tid in sym.complex_generic[1]])
            return f'{self.stringify_typename(sym.complex_generic[0])}<{args_comma_list}>'

        if sym.complex_union is not None:
            union_types = ' | '.join([self.stringify_typename(tid) for tid in sym.complex_union])
            return f'({union_types})'


class HierarchyPrinter:
    def __init__(self, ident: int):
        self.depth = 0
        self.ident = ident

    def enter(self):
        self.depth += 1

    def leave(self):
        self.depth -= 1

    def println(self, content: str):
        print(' ' * self.ident * self.depth + content)

    def newline(self):
        print()


def write_comment_lines(lines: list[str], printer: HierarchyPrinter) -> None:
    if len(lines) == 0:
        return
    if len(lines) == 1:
        printer.println(f'/** {lines[0]} */')
        return

    printer.println('/**')
    for line in lines:
        printer.println(f' * {line}')
    printer.println(' */')


class PropertyDecl:
    comment_lines: list[str]
    name: str
    readonly: bool
    static: bool
    typeid: int
    optional: bool

    def replace_typeid(self, callback: Callable[[int], int]):
        self.typeid = callback(self.typeid)

    def emit(self, table: SymbolTable, printer: HierarchyPrinter):
        write_comment_lines(self.comment_lines, printer)
        prefix = ''
        if self.static:
            prefix += 'static '
        if self.readonly:
            prefix += 'readonly '

        postfix = '?' if self.optional else ''
        printer.println(f'{prefix}{self.name}{postfix}: {table.stringify_typename(self.typeid)};')


class MethodDecl:
    comment_lines: list[str]
    name: str
    static: bool
    args: dict[str, int]
    rettype: int

    def replace_typeid(self, callback: Callable[[int], int]):
        self.rettype = callback(self.rettype)

        new_args_dict: dict[str, int] = {}
        for name, typeid in self.args.items():
            new_args_dict[name] = callback(typeid)
        self.args = new_args_dict

    def emit(self, table: SymbolTable, printer: HierarchyPrinter):
        write_comment_lines(self.comment_lines, printer)
        prefix = ''
        if self.static:
            prefix += 'static '

        args_list = ', '.join([f'{arg_name}: {table.stringify_typename(arg_type)}'
                               for arg_name, arg_type in self.args.items()])
        printer.println(f'{prefix}{self.name}({args_list}): {table.stringify_typename(self.rettype)};')


class DeclStmt:
    comment_lines: list[str]
    kind: Literal['iface', 'class', 'typedef', 'enum', 'func']
    typeid: int | None
    priv: bool

    def replace_typeid(self, callback: Callable[[int], int]):
        if self.typeid is not None:
            self.typeid = callback(self.typeid)
        self.on_replace_typeid(callback)

    def on_replace_typeid(self, callback: Callable[[int], int]):
        pass

    def emit(self, table: SymbolTable, printer: HierarchyPrinter):
        write_comment_lines(self.comment_lines, printer)
        prefix = ''
        if not self.priv:
            prefix += 'export '

        self.on_emit(table, prefix, printer)

    def on_emit(self, table: SymbolTable, prefix: str, printer: HierarchyPrinter):
        pass


class InterfaceDeclStmt(DeclStmt):
    name: str
    extends: list[int]
    properties: list[PropertyDecl]
    methods: list[MethodDecl]

    def on_replace_typeid(self, callback: Callable[[int], int]):
        for prop in self.properties:
            prop.replace_typeid(callback)

        for method in self.methods:
            method.replace_typeid(callback)

    def on_emit(self, table: SymbolTable, prefix: str, printer: HierarchyPrinter):
        content = prefix + f'interface {self.name}'
        if len(self.extends) > 0:
            extends_list = ', '.join([table.stringify_typename(tid) for tid in self.extends])
            content += f' extends {extends_list}'
        content += ' {'
        printer.println(content)

        printer.enter()
        for prop in self.properties:
            prop.emit(table, printer)

        for method in self.methods:
            method.emit(table, printer)
        printer.leave()

        printer.println('}')


class ClassDeclStmt(DeclStmt):
    name: str
    non_constructible: bool
    prot_constructible: bool
    extends: int | None
    implements: list[int]
    properties: list[PropertyDecl]
    methods: list[MethodDecl]
    ctor_args: dict[str, int] | None
    ctor_comment_lines: list[str]

    def on_replace_typeid(self, callback: Callable[[int], int]):
        if self.extends is not None:
            self.extends = callback(self.extends)

        new_impl_list: list[int] = []
        for typeid in self.implements:
            new_impl_list.append(callback(typeid))
        self.implements = new_impl_list

        for prop in self.properties:
            prop.replace_typeid(callback)

        for method in self.methods:
            method.replace_typeid(callback)

        if self.ctor_args is not None:
            new_ctor_args: dict[str, int] = {}
            for arg_name, arg_type in self.ctor_args.items():
                new_ctor_args[arg_name] = callback(arg_type)
            self.ctor_args = new_ctor_args

    def on_emit(self, table: SymbolTable, prefix: str, printer: HierarchyPrinter):
        content = prefix
        if self.priv:
            content += 'declare '

        content += f'class {self.name}'
        if self.extends is not None:
            content += f' extends {table.stringify_typename(self.extends)}'

        if len(self.implements) > 0:
            impl_list = ', '.join([table.stringify_typename(tid) for tid in self.implements])
            content += f' implements {impl_list}'

        content += ' {'
        printer.println(content)

        printer.enter()

        # Emit constructor definition
        if self.non_constructible:
            printer.println('private constructor();')
        elif self.prot_constructible:
            printer.println('protected constructor();')
        else:
            if self.ctor_args is None:
                raise ValueError(f'class {self.name}: undefined constructor argument list, but class is not marked with @nonconstructible')

            write_comment_lines(self.ctor_comment_lines, printer)
            args_list = ', '.join([f'{arg_name}: {table.stringify_typename(arg_type)}'
                                   for arg_name, arg_type in self.ctor_args.items()])
            printer.println(f'constructor({args_list});')

        for prop in self.properties:
            prop.emit(table, printer)

        for method in self.methods:
            method.emit(table, printer)

        printer.leave()
        printer.println('}')


class TypedefDeclStmt(DeclStmt):
    name: str
    right_typeid: int

    def on_replace_typeid(self, callback: Callable[[int], int]):
        self.right_typeid = callback(self.right_typeid)

    def on_emit(self, table: SymbolTable, prefix: str, printer: HierarchyPrinter):
        printer.println(f'type {self.name} = {table.stringify_typename(self.right_typeid)};')


class EnumItem:
    comment_lines: list[str]
    ident: str


class EnumDeclStmt(DeclStmt):
    name: str
    items: list[EnumItem]

    def on_emit(self, table: SymbolTable, prefix: str, printer: HierarchyPrinter):
        printer.println(prefix + f'declare enum {self.name} {{')
        printer.enter()

        for item in self.items:
            write_comment_lines(item.comment_lines, printer)
            printer.println(f'{item.ident},')

        printer.leave()
        printer.println('}')


class FunctionDeclStmt(DeclStmt):
    name: str
    rettype: int
    args: dict[str, int]

    def on_replace_typeid(self, callback: Callable[[int], int]):
        self.rettype = callback(self.rettype)

        new_args_dict: dict[str, int] = {}
        for name, typeid in self.args.items():
            new_args_dict[name] = callback(typeid)
        self.args = new_args_dict

    def on_emit(self, table: SymbolTable, prefix: str, printer: HierarchyPrinter):
        args_list = ', '.join([f'{arg_name}: {table.stringify_typename(arg_type)}'
                               for arg_name, arg_type in self.args.items()])
        printer.println(f'{prefix}function {self.name}({args_list}): {table.stringify_typename(self.rettype)};')


class ModuleDeclContext:
    def __init__(self):
        self.token_list: list[TokenInfo] = []
        self.curtok = 0
        self.symbol_table = SymbolTable()

    def tokenize_line(self, line: str, path: str, lineno: int) -> None:
        pos = 0
        while pos < len(line):
            line_remain = line[pos:]
            matched = False
            for token_type in Tokens:
                m = token_type.value.match(line_remain)
                if m is None:
                    continue

                # success match
                if token_type == Tokens.V_INTEGER:
                    self.token_list.append(TokenInfo(token_type, int(m.group()), path, lineno))
                elif token_type != Tokens.WHITESPACE and token_type != Tokens.NEWLINE:
                    self.token_list.append(TokenInfo(token_type, m.group(), path, lineno))

                # move to the next token
                pos += m.end()
                matched = True
                break

            if not matched:
                raise Exception(f'Unexpected token at {path} line {lineno}')

    def tokenize_comment_lines(self, lines: list[str], file: str, lineno: int) -> None:
        self.token_list.append(TokenInfo(Tokens.COMMENT_LINES, lines, file, lineno))

    def report_syntax_err(self, what: str):
        token = self.current_token()
        if token is None:
            raise Exception(what)
        raise Exception(f'file {token.file}:{token.lineno}: {what}')

    def move_to_next_token(self) -> TokenInfo | None:
        self.curtok += 1
        if self.curtok >= len(self.token_list):
            return None
        return self.token_list[self.curtok]

    def peek_next_token(self) -> TokenInfo | None:
        if self.curtok + 1 >= len(self.token_list):
            return None
        return self.token_list[self.curtok + 1]

    def current_token(self) -> TokenInfo | None:
        if self.curtok >= len(self.token_list):
            return None
        return self.token_list[self.curtok]

    def current_token_is(self, types: list[Tokens]) -> bool:
        token = self.current_token()
        if token is None:
            return False
        for token_type in types:
            if token.type == token_type:
                return True
        return False

    def current_token_should_be(self, types: list[Tokens]) -> None:
        if not self.current_token_is(types):
            self.report_syntax_err('unexpected token')

    def move_to_next_token_checked(self) -> TokenInfo:
        token = self.move_to_next_token()
        if token is None:
            raise Exception(f'Unexpected EOF in file {self.token_list[-1].file}')
        return token

    def parse_statement_list(self) -> list[DeclStmt]:
        stmt_list: list[DeclStmt] = []
        while self.current_token() is not None:
            stmt_list.append(self.parse_statement())
        return stmt_list

    def parse_statement(self) -> DeclStmt:
        pending_comments: list[str] = []
        if self.current_token_is([Tokens.COMMENT_LINES]):
            pending_comments = self.current_token().value
            self.move_to_next_token()

        if not self.current_token_is(TOKEN_TYPEGRP_STARTDECL):
            self.report_syntax_err('expects any of [@interface, @class, @enum, @typedecl] to start a declaration')

        cur = self.current_token()
        if cur.type == Tokens.KW_INTERFACE:
            return self.parse_compound_interface(pending_comments)
        elif cur.type == Tokens.KW_CLASS:
            return self.parse_compound_class(pending_comments)
        elif cur.type == Tokens.KW_ENUM:
            return self.parse_compound_enum(pending_comments)
        elif cur.type == Tokens.KW_TYPEDEF:
            return self.parse_typedef_stmt(pending_comments)
        elif cur.type == Tokens.KW_FUNCTION:
            return self.parse_function_decl_stmt(pending_comments)

    # Syntax: type-expr1, type-expr2, ...
    def parse_type_expr_list(self) -> list[int]:
        result: list[int] = []

        while True:
            # Consume an ident
            result.append(self.parse_type_expr())

            # Consume a possible ','
            if self.current_token().type == Tokens.OP_COMMA:
                self.move_to_next_token_checked()
            else:
                break

        return result

    def parse_compound_interface(self, comments: list[str]) -> InterfaceDeclStmt:
        iface = InterfaceDeclStmt()
        iface.comment_lines = comments
        iface.kind = 'iface'
        iface.priv = False

        # Consume '@interface'
        self.current_token_should_be([Tokens.KW_INTERFACE])
        self.move_to_next_token_checked()

        # Parse possible hints
        iface.extends = []
        while self.current_token_is(TOKEN_TYPEGRP_HINTS):
            token = self.current_token()
            self.move_to_next_token_checked()

            if token.type == Tokens.HINT_PRIV:
                iface.priv = True

            elif token.type == Tokens.HINT_EXTENDS:
                # Consume a '('
                self.current_token_should_be([Tokens.PAR_L])
                self.move_to_next_token_checked()

                iface.extends.extend(self.parse_type_expr_list())

                # Consume a ')'
                self.current_token_should_be([Tokens.PAR_R])
                self.move_to_next_token_checked()

            else:
                self.report_syntax_err('unsupported hint for interface declaration')

        # Consume an ident as interface name
        self.current_token_should_be([Tokens.V_IDENT])
        iface.name = self.current_token().value
        self.move_to_next_token_checked()

        # Parse until `@end`
        iface.properties = []
        iface.methods = []
        while self.current_token().type != Tokens.KW_END:
            # Consume `@property` or `@method`
            token = self.current_token()

            comments: list[str] = []
            if token.type == Tokens.COMMENT_LINES:
                comments = token.value
                self.move_to_next_token()
                token = self.current_token()

            if token.type == Tokens.KW_PROPERTY:
                iface.properties.append(self.parse_property_decl(comments, allow_static=False))

            elif token.type == Tokens.KW_METHOD:
                iface.methods.append(self.parse_method_decl(comments, allow_static=False))

            else:
                self.report_syntax_err('unsupported keyword for interface declaration')

        # Consume '@end'
        self.move_to_next_token()

        sym = TypeSymbol('declared')
        sym.name = iface.name
        iface.typeid = self.symbol_table.define(sym)

        return iface

    def parse_compound_class(self, comments: list[str]) -> ClassDeclStmt:
        cl = ClassDeclStmt()
        cl.comment_lines = comments
        cl.kind = 'class'
        cl.priv = False

        # Consume '@class'
        self.current_token_should_be([Tokens.KW_CLASS])
        self.move_to_next_token_checked()

        # Parse possible hints
        cl.non_constructible = False
        cl.prot_constructible = False
        cl.implements = []
        cl.extends = None
        while self.current_token_is(TOKEN_TYPEGRP_HINTS):
            token = self.current_token()
            self.move_to_next_token_checked()

            if token.type == Tokens.HINT_PRIV:
                cl.priv = True

            elif token.type == Tokens.HINT_NON_CONSTRUCTIBLE:
                cl.non_constructible = True

            elif token.type == Tokens.HINT_PROT_CONSTRUCTIBLE:
                cl.prot_constructible = True

            elif token.type == Tokens.HINT_EXTENDS:
                # Consume a '('
                self.current_token_should_be([Tokens.PAR_L])
                self.move_to_next_token_checked()

                cl.extends = self.parse_type_expr()

                # Consume a ')'
                self.current_token_should_be([Tokens.PAR_R])
                self.move_to_next_token_checked()

            elif token.type == Tokens.HINT_IMPL:
                # Consume a '('
                self.current_token_should_be([Tokens.PAR_L])
                self.move_to_next_token_checked()

                cl.implements.extend(self.parse_type_expr_list())

                # Consume a ')'
                self.current_token_should_be([Tokens.PAR_R])
                self.move_to_next_token_checked()

            else:
                self.report_syntax_err('unsupported hint for class declaration')

        # Consume an ident for class name
        self.current_token_should_be([Tokens.V_IDENT])
        cl.name = self.current_token().value
        self.move_to_next_token_checked()

        # Parse until `@end`
        cl.properties = []
        cl.methods = []
        cl.ctor_args = None
        cl.ctor_comment_lines = []
        while self.current_token().type != Tokens.KW_END:
            # Consume `@property` or `@method`
            token = self.current_token()

            comments: list[str] = []
            if token.type == Tokens.COMMENT_LINES:
                comments = token.value
                self.move_to_next_token()
                token = self.current_token()

            if token.type == Tokens.KW_PROPERTY:
                cl.properties.append(self.parse_property_decl(comments, allow_static=True))

            elif token.type == Tokens.KW_METHOD:
                cl.methods.append(self.parse_method_decl(comments, allow_static=True))

            elif token.type == Tokens.KW_CONSTRUCTOR:
                if cl.non_constructible or cl.prot_constructible:
                    self.report_syntax_err(f'non-constructible or prot-constructible class {cl.name} declared a constructor')

                # Consume `@constructor`
                self.move_to_next_token_checked()
                cl.ctor_args = self.parse_arguments_dict()
                cl.ctor_comment_lines = comments

            else:
                self.report_syntax_err('unsupported keyword for interface declaration')

        # Consume '@end'
        self.move_to_next_token()

        sym = TypeSymbol('declared')
        sym.name = cl.name
        cl.typeid = self.symbol_table.define(sym)

        return cl

    def parse_compound_enum(self, comments: list[str]) -> EnumDeclStmt:
        enum = EnumDeclStmt()
        enum.comment_lines = comments
        enum.kind = 'enum'
        enum.priv = False

        # Consume '@enum'
        self.current_token_should_be([Tokens.KW_ENUM])
        self.move_to_next_token_checked()

        # Consume an ident for enum name
        self.current_token_should_be([Tokens.V_IDENT])
        enum.name = self.current_token().value
        self.move_to_next_token_checked()

        enum.items = []
        while self.current_token().type != Tokens.KW_END:
            current_item = EnumItem()
            current_item.comment_lines = []

            if self.current_token().type == Tokens.COMMENT_LINES:
                current_item.comment_lines = self.current_token().value
                self.move_to_next_token_checked()

            # Consume '@enumitem'
            self.current_token_should_be([Tokens.KW_ENUMITEM])
            self.move_to_next_token_checked()

            self.current_token_should_be([Tokens.V_IDENT])
            current_item.ident = self.current_token().value
            enum.items.append(current_item)
            self.move_to_next_token_checked()

        # Consume '@end'
        self.move_to_next_token()

        sym = TypeSymbol('declared')
        sym.name = enum.name
        enum.typeid = self.symbol_table.define(sym)

        return enum

    def parse_typedef_stmt(self, comments: list[str]) -> TypedefDeclStmt:
        typedef = TypedefDeclStmt()
        typedef.comment_lines = comments
        typedef.kind = 'typedef'
        typedef.priv = False

        # Consume '@typedef'
        self.current_token_should_be([Tokens.KW_TYPEDEF])
        self.move_to_next_token_checked()

        # Parse possible hints
        while self.current_token_is(TOKEN_TYPEGRP_HINTS):
            token = self.current_token()
            self.move_to_next_token_checked()

            if token.type == Tokens.HINT_PRIV:
                typedef.priv = True
            else:
                self.report_syntax_err('unsupported hint for typedef declaration')

        # Consume an ident for type name
        self.current_token_should_be([Tokens.V_IDENT])
        typedef.name = self.current_token().value
        self.move_to_next_token_checked()

        # Consume a '='
        self.current_token_should_be([Tokens.OP_EQ])
        self.move_to_next_token_checked()

        # Parse type
        typedef.right_typeid = self.parse_type_expr()

        sym = TypeSymbol('declared')
        sym.name = typedef.name
        typedef.typeid = self.symbol_table.define(sym)

        return typedef

    def parse_property_decl(self, comments: list[str], allow_static: bool) -> PropertyDecl:
        prop = PropertyDecl()
        prop.comment_lines = comments
        prop.static = False
        prop.readonly = False
        prop.optional = False

        # Consume '@property'
        self.current_token_should_be([Tokens.KW_PROPERTY])
        self.move_to_next_token_checked()

        # Parse possible hints
        while self.current_token_is(TOKEN_TYPEGRP_HINTS):
            token = self.current_token()
            self.move_to_next_token_checked()

            if token.type == Tokens.HINT_STATIC:
                if not allow_static:
                    self.report_syntax_err('this property must not be static')
                prop.static = True

            elif token.type == Tokens.HINT_READONLY:
                prop.readonly = True

            elif token.type == Tokens.HINT_OPTIONAL:
                prop.optional = True

            else:
                self.report_syntax_err('unsupported hint for property declaration')

        # Consume an ident for property name
        self.current_token_should_be([Tokens.V_IDENT])
        prop.name = self.current_token().value
        self.move_to_next_token_checked()

        # Consume a ':'
        self.current_token_should_be([Tokens.OP_COLON])
        self.move_to_next_token_checked()

        # Parse property type
        prop.typeid = self.parse_type_expr()

        return prop

    # argument_expr -> ARG_NAME ':' type_expr
    # arguments_list -> argument_expr | arguments_list ',' argument_expr
    # arguments_dict -> '(' arguments_list ')'
    def parse_arguments_dict(self) -> dict[str, int]:
        # Consume a '('
        self.current_token_should_be([Tokens.PAR_L])
        self.move_to_next_token_checked()

        result: dict[str, int] = {}
        while self.current_token().type != Tokens.PAR_R:
            # Consume an ident as argument name
            self.current_token_should_be([Tokens.V_IDENT])
            arg_name = self.current_token().value
            self.move_to_next_token_checked()

            # Consume a ':'
            self.current_token_should_be([Tokens.OP_COLON])
            self.move_to_next_token_checked()

            # Parse argument type
            result[arg_name] = self.parse_type_expr()

            # Consume a possible ','
            if self.current_token().type == Tokens.OP_COMMA:
                self.move_to_next_token_checked()

        # Consume a ')'
        self.move_to_next_token_checked()
        return result

    def parse_method_decl(self, comments: list[str], allow_static: bool) -> MethodDecl:
        method = MethodDecl()
        method.comment_lines = comments
        method.static = False

        # Consume '@method'
        self.current_token_should_be([Tokens.KW_METHOD])
        self.move_to_next_token_checked()

        # Parse possible hints
        while self.current_token_is(TOKEN_TYPEGRP_HINTS):
            token = self.current_token()
            self.move_to_next_token_checked()

            if token.type == Tokens.HINT_STATIC:
                if not allow_static:
                    self.report_syntax_err('this method must not be static')
                method.static = True

            else:
                self.report_syntax_err('unsupported hint for method declaration')

        # Consume an ident for method name
        self.current_token_should_be([Tokens.V_IDENT])
        method.name = self.current_token().value
        self.move_to_next_token_checked()

        # Parse arguments list
        method.args = self.parse_arguments_dict()

        # Consume a ':'
        self.current_token_should_be([Tokens.OP_COLON])
        self.move_to_next_token_checked()

        # Parse return type
        method.rettype = self.parse_type_expr()

        return method

    def parse_function_decl_stmt(self, comments: list[str]) -> FunctionDeclStmt:
        func = FunctionDeclStmt()
        func.comment_lines = comments
        func.kind = 'func'
        func.priv = False
        func.typeid = None

        # Consume '@function'
        self.current_token_should_be([Tokens.KW_FUNCTION])
        self.move_to_next_token_checked()

        # Consume an ident for function name
        self.current_token_should_be([Tokens.V_IDENT])
        func.name = self.current_token().value
        self.move_to_next_token_checked()

        # Consume a '('
        self.current_token_should_be([Tokens.PAR_L])
        self.move_to_next_token_checked()

        # Parse arguments list
        args_dict: dict[str, int] = {}
        while self.current_token().type != Tokens.PAR_R:
            # Consume an ident as argument name
            self.current_token_should_be([Tokens.V_IDENT])
            arg_name = self.current_token().value
            self.move_to_next_token_checked()

            # Consume a ':'
            self.current_token_should_be([Tokens.OP_COLON])
            self.move_to_next_token_checked()

            # Parse argument type
            args_dict[arg_name] = self.parse_type_expr()

            # Consume a possible ','
            if self.current_token().type == Tokens.OP_COMMA:
                self.move_to_next_token_checked()

        # Consume a ')'
        self.move_to_next_token_checked()

        func.args = args_dict

        # Consume a ':'
        self.current_token_should_be([Tokens.OP_COLON])
        self.move_to_next_token_checked()

        # Parse return type
        func.rettype = self.parse_type_expr()

        return func

    def parse_type_expr(self) -> int:
        define_complex_type = False

        sym = TypeSymbol('complex')
        sym.name = None
        sym.import_from = None
        sym.complex_tuple = None
        sym.complex_array = None
        sym.complex_function = None
        sym.complex_generic = None
        sym.complex_union = None

        import_from: str | None = None

        # Parse optional hints
        while self.current_token_is(TOKEN_TYPEGRP_HINTS):
            token = self.current_token()
            self.move_to_next_token_checked()

            if token.type == Tokens.HINT_IMPORT:
                # Consume a '(`
                self.current_token_should_be([Tokens.PAR_L])
                self.move_to_next_token_checked()

                # Consume the import name
                self.current_token_should_be([Tokens.V_IDENT])
                import_from = self.current_token().value
                self.move_to_next_token_checked()

                # Consume a ')'
                self.current_token_should_be([Tokens.PAR_R])
                self.move_to_next_token_checked()

            elif token.type == Tokens.HINT_TUPLE:
                if define_complex_type:
                    self.report_syntax_err('define multiple complex type')

                # Consume a '('
                self.current_token_should_be([Tokens.PAR_L])
                self.move_to_next_token_checked()

                sym.complex_tuple = self.parse_type_expr_list()

                # Consume a ')'
                self.current_token_should_be([Tokens.PAR_R])
                self.move_to_next_token()
                define_complex_type = True

            elif token.type == Tokens.HINT_ARRAY:
                if define_complex_type:
                    self.report_syntax_err('define multiple complex type')

                # Consume a '('
                self.current_token_should_be([Tokens.PAR_L])
                self.move_to_next_token_checked()

                # Parse a typename
                sym.complex_array = self.parse_type_expr()

                # Consume a ')'
                self.current_token_should_be([Tokens.PAR_R])
                self.move_to_next_token()
                define_complex_type = True

            elif token.type == Tokens.HINT_FUNCTION:
                if define_complex_type:
                    self.report_syntax_err('define multiple complex type')

                # Consume a '('
                self.current_token_should_be([Tokens.PAR_L])
                self.move_to_next_token_checked()

                # Parse a typename as the return type of function
                ret_type = self.parse_type_expr()
                args_dict: dict[str, int] = {}
                while self.current_token_is([Tokens.OP_COMMA]):
                    # Consume a ',' (for list separator)
                    self.move_to_next_token_checked()

                    # Consume an ident for argument name
                    self.current_token_should_be([Tokens.V_IDENT])
                    arg_name = self.current_token().value
                    self.move_to_next_token_checked()

                    # Consume a ':'
                    self.current_token_should_be([Tokens.OP_COLON])
                    self.move_to_next_token_checked()

                    # Parse a type for argument type
                    args_dict[arg_name] = self.parse_type_expr()
                    if self.current_token_is([Tokens.PAR_R]):
                        break

                    self.current_token_should_be([Tokens.OP_COMMA])

                # Consume a ')'
                self.current_token_should_be([Tokens.PAR_R])
                self.move_to_next_token()
                define_complex_type = True

                sym.complex_function = (ret_type, args_dict)

            elif token.type == Tokens.HINT_MEM:
                if define_complex_type:
                    self.report_syntax_err('define multiple complex type')

                # Consume a '('
                self.current_token_should_be([Tokens.PAR_L])
                self.move_to_next_token_checked()

                # Parse mem type name
                self.current_token_should_be([Tokens.V_IDENT])
                sym.complex_mem = self.current_token().value
                self.move_to_next_token_checked()

                if sym.complex_mem not in NUMERIC_TO_TYPED_ARRAY_MAP:
                    self.report_syntax_err(f'unexpected memory type for @mem(TYPE): {sym.complex_mem}')

                # Consume a ')'
                self.current_token_should_be([Tokens.PAR_R])
                self.move_to_next_token()
                define_complex_type = True

            elif token.type == Tokens.HINT_GENERIC or token.type == Tokens.HINT_PROMISE:
                if define_complex_type:
                    self.report_syntax_err('define multiple complex type')

                # Consume a '('
                self.current_token_should_be([Tokens.PAR_L])
                self.move_to_next_token_checked()

                generic_type = 0
                if token.type == Tokens.HINT_GENERIC:
                    # Parse the generic type
                    generic_type = self.parse_type_expr()

                    # Consume a ','
                    self.current_token_should_be([Tokens.OP_COMMA])
                    self.move_to_next_token_checked()
                else:
                    promise_pending_sym = PendingTypeSymbol()
                    promise_pending_sym.name = 'Promise'
                    promise_pending_sym.import_from = None
                    generic_type = self.symbol_table.define_pending(promise_pending_sym)

                # Parse generic args
                generic_args = self.parse_type_expr_list()

                # Consume a ')'
                self.current_token_should_be([Tokens.PAR_R])
                self.move_to_next_token()

                sym.complex_generic = (generic_type, generic_args)
                define_complex_type = True

            elif token.type == Tokens.HINT_UNION:
                if define_complex_type:
                    self.report_syntax_err('define multiple complex type')

                # Consume a '('
                self.current_token_should_be([Tokens.PAR_L])
                self.move_to_next_token_checked()

                # Parse the union types list
                sym.complex_union = self.parse_type_expr_list()

                # Consume a ')'
                self.current_token_should_be([Tokens.PAR_R])
                self.move_to_next_token_checked()
                define_complex_type = True

            else:
                self.report_syntax_err('unsupported hint for typename reference')

        # Complex type is defined inplace without any name
        if define_complex_type:
            sym.kind = 'complex'
            return self.symbol_table.define(sym)

        # Non-complex type is a reference of the actual type, which is a pending
        # type before we find its declaration.
        pending_sym = PendingTypeSymbol()
        pending_sym.import_from = import_from

        # Parse type name
        self.current_token_should_be([Tokens.V_IDENT])
        pending_sym.name = self.current_token().value
        self.move_to_next_token()

        return self.symbol_table.define_pending(pending_sym)


class CodeEmitter:
    def __init__(self, statement_list: list[DeclStmt], symbol_table: SymbolTable, printer: HierarchyPrinter):
        self.statement_list: list[DeclStmt] = statement_list
        self.symbol_table: SymbolTable = symbol_table
        self.printer = printer

    def emit_external_imports(self):
        imports: set[str] = set()
        for sym in self.symbol_table.defined_table.values():
            if sym.import_from is None:
                continue
            imports.add(sym.import_from)

        for source in imports:
            self.printer.println(f'import * as {import_url_to_name(source)} from \'{source}\';')

    def emit_builtin_alias(self):
        for alias, origin in BUILTIN_TYPES_MAP.items():
            if alias != origin:
                self.printer.println(f'type {alias} = {origin};')

    def emit_statements(self, kind: str):
        for stmt in self.statement_list:
            if stmt.kind != kind:
                continue
            stmt.emit(self.symbol_table, self.printer)


def parse_single_file(path: str, context: ModuleDeclContext) -> None:
    if not os.path.isfile(path):
        return

    with open(path, 'r') as file:
        lineno = 1
        parse_comment_mode = False
        comment_lines: list[str] = []

        for line in file:
            pos = line.find('//!')
            if pos < 0:
                lineno += 1
                continue
            line = line[pos:]

            if parse_comment_mode:
                if line.startswith(TSDOC_END):
                    parse_comment_mode = False
                    context.tokenize_comment_lines(comment_lines, path, lineno)
                elif line.startswith(TSDOC_BODY_PREFIX):
                    # +1 to skip a space; use `-1` to strip out the '\n' at the end
                    comment_lines.append(line[len(TSDOC_BODY_PREFIX)+1:-1])

            elif line.startswith(TSDECL_PREFIX):
                context.tokenize_line(line[len(TSDECL_PREFIX):], path, lineno)
            elif line.startswith(TSDOC_BEGIN):
                parse_comment_mode = True
                comment_lines = []

            lineno += 1


def scan_dir_files(dirname: str) -> ModuleDeclContext:
    decl_context = ModuleDeclContext()
    for filename in os.listdir(dirname):
        parse_single_file(f'{dirname}/{filename}', decl_context)

    return decl_context


class SymbolTableDerefContext:
    def __init__(self, table: SymbolTable):
        self.table = table
        self.cache: dict[int, int] = {}

    def replace(self, stmt: DeclStmt):

        def callback(typeid: int) -> int:
            if typeid in self.cache:
                return self.cache[typeid]

            result = self.table.deref_ref_chain(typeid)
            self.cache[typeid] = result
            return result

        stmt.replace_typeid(callback)


def run() -> None:
    if len(sys.argv) != 2:
        raise Exception('Missing an argument to specify the directory')

    result = scan_dir_files(sys.argv[1])

    stmt_list = result.parse_statement_list()
    result.symbol_table.resolve_pending()

    deref_ctx = SymbolTableDerefContext(result.symbol_table)
    for stmt in stmt_list:
        deref_ctx.replace(stmt)

    result.symbol_table.eliminate_typerefs()

    printer = HierarchyPrinter(4)
    emitter = CodeEmitter(stmt_list, result.symbol_table, printer)

    emitter.emit_external_imports()
    emitter.emit_builtin_alias()
    emitter.emit_statements('typedef')
    emitter.emit_statements('enum')
    emitter.emit_statements('iface')
    emitter.emit_statements('class')
    emitter.emit_statements('func')


if __name__ == '__main__':
    run()
