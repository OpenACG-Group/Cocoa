"""
A Kaleidoscopic Text Description Language is a specification which is
a part of Cocoa Project. This specification is usually used to
display awesome text effects in Cocoa.
This file is a simple implementation of KTDL language, only
supports effects that can be displayed in Linux terminal.
"""

import re
from enum import Enum, unique


@unique
class LexTokenType(Enum):
    TOKEN_INNER_TEXT = 0
    TOKEN_BRACE_OPEN = 1
    TOKEN_BRACE_CLOSE = 2
    TOKEN_DOLLAR = 3
    TOKEN_EQUAL = 4
    TOKEN_INTEGER = 5
    TOKEN_FLOAT = 6
    TOKEN_PARENTHESES_OPEN = 7
    TOKEN_PARENTHESES_CLOSE = 8
    TOKEN_SCOPE_RESOLUTION = 9
    TOKEN_WORD = 10


class Scanner:
    def __init__(self):
        self.brace_open_pattern = re.compile(r"{")
        self.brace_close_pattern = re.compile(r"}")
        self.dollar_pattern = re.compile(r"\$")
        self.equal_pattern = re.compile(r"=")
        self.integer_pattern = re.compile(r"[0-9]+")
        self.float_pattern = re.compile(r"[0-9]*\.[0-9]+")
        self.parentheses_open_pattern = re.compile(r"\(")
        self.parentheses_close_pattern = re.compile(r"\)")
        self.scope_resolution_pattern = re.compile(r"::")
        self.word_pattern = re.compile(r"[a-zA-Z][a-zA-Z0-9]*")
