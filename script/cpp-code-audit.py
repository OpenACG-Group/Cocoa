#!/usr/bin/env python

import sys
import os
import re as regex


class Audit:
    total_lines = 0
    comment_lines = 0
    preprocessor_lines = 0
    blank_lines = 0


class Patterns:
    cpp_postfix = regex.compile("\.(cc|cpp|h|hpp)$")
    single_line_comment = regex.compile(r"^[\t ]*//.*$")
    preprocessor_line = regex.compile(r"^[\t ]*#.*$")
    blank_line = regex.compile(r"^[\t ]*$")


def analyze_file(file: str):
    if regex.search(Patterns.cpp_postfix, file) is None:
        return

    print("Analyzing file " + file)
    fs = open(file, "r")
    for line in fs.readlines():
        Audit.total_lines += 1
        if regex.search(Patterns.single_line_comment, line) is not None:
            Audit.comment_lines += 1
        elif regex.search(Patterns.preprocessor_line, line) is not None:
            Audit.preprocessor_lines += 1
        elif regex.search(Patterns.blank_line, line) is not None:
            Audit.blank_lines += 1


def readdir_recursive(path: str):
    file_list = os.listdir(path)
    for sub_path in file_list:
        full_path = path + '/' + sub_path
        if os.path.isdir(full_path):
            readdir_recursive(full_path)
        else:
            analyze_file(full_path)


readdir_recursive(sys.argv[1])

print("Total lines: " + str(Audit.total_lines))
print("Blank lines: " + str(Audit.blank_lines))
print("Preprocessor lines: " + str(Audit.preprocessor_lines))
print("Pure comment lines: " + str(Audit.comment_lines))
