import re
from enum import Enum
import io


class FragmentType(Enum):
    LBRACKET = 1        # [
    RBRACKET = 2        # ]
    INSTRUCTION = 3     # [ INSTRUCTION ]
    TEXT = 4
    EOF = 5             # End of file


class Fragment:
    def __init__(self, type_: FragmentType, content: str, line: int) -> None:
        self.type = type_
        self.content = content
        self.line = line
        self.tokenized: list[Token] = []


class TokenType(Enum):
    NUMERIC = 3
    TRUE = 4
    FALSE = 5
    STRING = 6
    IDENT = 7
    ASSIGN = 8
    EQUAL = 9
    NOT_EQUAL = 10
    ADD = 11
    SUB = 12
    DIV = 13
    MUL = 14
    LT = 15
    BT = 16
    LE = 17
    BE = 18
    COMMA = 19
    COLON = 20
    WHITESPACE_ = 21


__token_pattern_map: dict[TokenType, str] = {
    TokenType.ASSIGN: r'=',
    TokenType.EQUAL: r'==',
    TokenType.NOT_EQUAL: r'!=',
    TokenType.ADD: r'\+',
    TokenType.SUB: r'-',
    TokenType.DIV: r'/',
    TokenType.MUL: r'\*',
    TokenType.LT: r'<',
    TokenType.BT: r'>',
    TokenType.LE: r'<=',
    TokenType.BE: r'>=',
    TokenType.COMMA: r',',
    TokenType.NUMERIC: r'[0-9]+(\.[0-9]+)?',
    TokenType.TRUE: r'true',
    TokenType.FALSE: r'false',
    TokenType.STRING: r'\"(\\.|[^"\\])*\"',
    TokenType.IDENT: r'[a-zA-Z_][0-9a-zA-Z_]*',
    TokenType.COLON: r':',
    TokenType.WHITESPACE_: r'[\ \t]+'
}


class Token:
    def __init__(self, type_: TokenType, content: str, line: int):
        self.type = type_
        self.content = content
        self.line = line


class TokenError(Exception):
    def __init__(self, what, line) -> None:
        self.what = what
        self.line = line

    def __str__(self) -> str:
        return f'{self.what} at line {self.line}'


class Preprocessor:
    def __init__(self, path: str) -> None:
        with open(path, encoding='utf-8') as fp:
            fp.seek(0, io.SEEK_END)
            size = fp.tell()
            fp.seek(0, io.SEEK_SET)
            self.buf = fp.read(size)
        self.buf_pos = 0
        self.in_brackets = False
        self.line = 1

    def peek_character(self) -> str | None:
        if self.buf_pos >= len(self.buf):
            return None
        return self.buf[self.buf_pos]

    def lookahead(self):
        self.buf_pos += 1

    def next_in_bracket(self) -> Fragment:
        if self.peek_character() is None:
            return Fragment(FragmentType.EOF, '', self.line)
        if self.peek_character() == ']':
            self.in_brackets = False
            self.lookahead()
            return Fragment(FragmentType.RBRACKET, ']', self.line)
        text = ''
        while self.peek_character() is not None and self.peek_character() != ']':
            text += self.peek_character()
            self.lookahead()
        if self.peek_character() is None:
            return Fragment(FragmentType.EOF, '', self.line)
        return Fragment(FragmentType.INSTRUCTION, text, self.line)

    def next_not_in_bracket(self) -> Fragment:
        if self.peek_character() is None:
            return Fragment(FragmentType.EOF, '', self.line)
        if self.peek_character() == '[':
            self.in_brackets = True
            self.lookahead()
            return Fragment(FragmentType.LBRACKET, '[', self.line)
        escape_next = False
        start_line = self.line
        text = ''
        while self.peek_character() is not None:
            match self.peek_character():
                case '\n':
                    self.line += 1
                case ch if escape_next:
                    text += ch
                    escape_next = False
                case '[':
                    break
                case '\\':
                    escape_next = True
                    text += self.peek_character()
                case _:
                    text += self.peek_character()
            self.lookahead()
        if self.peek_character() is None:
            return Fragment(FragmentType.EOF, '', self.line)
        return Fragment(FragmentType.TEXT, text, start_line)

    def next(self) -> Fragment:
        if self.in_brackets:
            return self.next_in_bracket()
        else:
            return self.next_not_in_bracket()

    def separate(self) -> list[Fragment]:
        f = self.next()
        result = []
        while f.type != FragmentType.EOF:
            result.append(f)
            f = self.next()
        return result


def __try_match_token(content: str, line: int) -> tuple[Token, int] | None:
    for pat in __token_pattern_map:
        m = re.match(__token_pattern_map[pat], content)
        if m is None or m.start() != 0:
            continue
        return Token(pat, content[0:m.end()], line), m.end()
    return None


def __do_tokenize(f: Fragment):
    pos = 0
    while pos < len(f.content):
        ret = __try_match_token(f.content[pos:], f.line)
        if ret is None:
            raise TokenError(f'Unrecognized instruction: {f.content}', f.line)
        pos += ret[1]
        if ret[0].type is TokenType.WHITESPACE_:
            continue
        f.tokenized.append(ret[0])


def __tokenize(fragments: list[Fragment]) -> list[Fragment]:
    for f in fragments:
        if f.type is not FragmentType.INSTRUCTION:
            continue
        __do_tokenize(f)
    return fragments


def parse_from_file(path: str):
    fragments = Preprocessor(path).separate()
    for fragment in __tokenize(fragments):
        print(f'{fragment.type}<{fragment.content}>')
        for tok in fragment.tokenized:
            print(f'{tok.type}{{{tok.content}}}')
