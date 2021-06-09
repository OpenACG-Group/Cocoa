#!/usr/bin/env python3
import os
import sys
import re

corejs_prologue = '''
/**
 * CocoaJs Core Object, Javascript ES6 Standard.
 * Copyright(C) 2021, Jingheng Luo (masshiro.io@qq.com)
 */
let Cocoa;
(function (Cocoa) {
    "use strict";
'''

corejs_epilogue = '''
    /* Wrapper functions for performing Op */
    Cocoa.vmcall = (name, ...args) => {
        return __do_vmcall(name, ...args);
    };
    Cocoa.vmcallAsync = (name, ...args) => {
        return __do_vmcall_async(name + "_async", ...args);
    };
    
    Cocoa.vminfo = () => {
        return {
            version: {corejs_codegen_version}
        };
    };
})(Cocoa || (Cocoa = {}));
'''


class CodeGenerator:
    def __init__(self):
        self.in_directive = None
        self.insertions = []

    def emit_code(self, decl: str, value: str):
        self.insertions += "    Cocoa.{} = {} /* {} */\n".format(decl, value, self.in_directive)

    def cpp_simple_preprocessor(self, line: str):
        result = re.search(R"^#define[\t ]+([a-zA-Z_][a-zA-Z0-9_]*)[\t ]+(.*)$", line)
        if result is None:
            return
        self.emit_code(result.group(1), result.group(2))

    def parse_pragma(self, pragma: str):
        directives = pragma.split('#')
        section_mark, begin_or_end = directives[0], directives[1]
        if begin_or_end == "begin":
            if self.in_directive is not None:
                raise Exception("Syntax error (Directive " + pragma + "), unexpected 'begin'")
            self.in_directive = section_mark
        elif begin_or_end == "end":
            if self.in_directive is None:
                raise Exception("Syntax error (Directive " + pragma + "), unexpected 'end'")
            if self.in_directive != section_mark:
                raise Exception("Syntax error (Directive " + pragma + "), mismatched directive")
            self.in_directive = None

    def parse_source_cpp_file(self, filepath: str):
        file = open(filepath, "r")
        pattern = re.compile("^(__js_codegen_pragma)\\((.*)\\)$")
        for line in file:
            matcher = pattern.search(line)
            if matcher is not None:
                pragma = matcher.group(2)
                self.parse_pragma(pragma)
            else:
                if self.in_directive is None:
                    continue
                self.cpp_simple_preprocessor(line)

    def generated(self):
        res = corejs_prologue
        for line in self.insertions:
            res += line
        res += corejs_epilogue
        return res


class OptionsParser:
    def __init__(self):
        self.options = {}
        self.free_arguments = []

    def parse(self, argv: list):
        eat_next = False
        eat_next_name = ""
        for arg in argv:
            if eat_next:
                self.options[eat_next_name] = arg
                eat_next = False
            elif arg.startswith("--"):
                assert not eat_next
                eat_next_name = arg[2:]
                eat_next = True
            else:
                self.free_arguments.append(arg)


def print_usage_and_exit():
    print('''
Usage: {} [options...] <source file>
Options:
    --output <string>       Specify the directory of generated files.
    --version <string>      Specify the version number of VM.
    --manufacture <string>  Specify the name of manufacture of VM.
    --codegen <string>      Specify the name of CodeGenerator.
    '''.format(sys.argv[0]))
    sys.exit(1)


options = OptionsParser()
options.parse(sys.argv)
if len(options.options) == 0:
    print_usage_and_exit()

#gen = CodeGenerator()
#gen.parse_source_cpp_file("./Op.h")
#print(gen.generated())
