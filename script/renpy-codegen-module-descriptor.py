#!/usr/bin/env python3
import os.path

import sys
import re
import random


def print_err(msg):
    sys.stderr.write(msg + '\n')

def report_redundant_use(verb, what):
    print_err('\'' + verb + '\' is used before. Specify more than one ' + what + ' is unacceptable')
    exit(1)

def report_syntax_error(verb, example):
    print_err('\'' + verb + '\' has wrong syntax, using [' + example + '] instead')
    exit(1)


if len(sys.argv) != 2:
    print_err("Requires 1 argument to specify MDS file path")
    exit(1)

header_lines = []
namespace_id = ''
class_name = ''
insert_lines = []
export_symbols = []

expr_pattern = re.compile("^%export(_const|_static)?[ ]+[a-zA-Z_][a-zA-Z0-9_]+[ ]+(.*)$")

mds_fp = open(sys.argv[1], 'r')

insert_state = False
header_state = False
for line in mds_fp.readlines():
    if len(line) == 0:
        continue
    line = line[:-1]
    split_arr = line.split(' ')
    verb = split_arr[0]
    if insert_state:
        if verb == '%insert_end':
            if len(split_arr) != 1:
                report_syntax_error('%insert_end', '%insert_end')
            insert_state = False
        else:
            insert_lines.append(line)
    elif header_state:
        if verb == '%header_end':
            if len(split_arr) != 1:
                report_syntax_error('%header_end', '%header_end')
            header_state = False
        else:
            header_lines.append(line)
    elif verb == '%namespace':
        if len(namespace_id) != 0:
            report_redundant_use('%namespace', 'namespace')
        if len(split_arr) != 2:
            report_syntax_error('%namespace', '%namespace <identifier>')
        namespace_id = split_arr[1]
    elif verb == '%class':
        if len(class_name) != 0:
            report_redundant_use('%class', 'class name')
        if len(split_arr) != 2:
            report_syntax_error('%class', '%class name')
        class_name = split_arr[1]
    elif verb == '%insert_begin':
        if len(split_arr) != 1:
            report_syntax_error('%insert_begin', '%insert_begin')
        insert_state = True
    elif verb == '%insert_end':
        print_err('%insert_end only can be used with %insert_begin')
        exit(1)
    elif verb == '%header_begin':
        if len(split_arr) != 1:
            report_syntax_error('%header_begin', '%header_begin')
        header_state = True
    elif verb == '%header_end':
        print_err('%header_end only can be used with %header_begin')
        exit(1)
    elif verb == '%export' or verb == '%export_const' or verb == '%export_static':
        if len(split_arr) < 3:
            report_syntax_error(verb, verb + ' <name> <cpp inline expr>')
        export_symbols.append({'name': split_arr[1],
                               'expr': expr_pattern.match(line)[2],
                               'const': verb == '%export_const',
                               'static': verb == '%export_static'})
    elif verb == '%export_decl':
        if len(split_arr) != 2:
            report_syntax_error('%export_decl', '%report_decl <name>')
        export_symbols.append({'name': split_arr[1],
                               'expr': None,
                               'const': None})
    elif len(verb) != 0:
        print_err("Unexpected instruction " + verb)
        exit(1)


hex_transition = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                  'a', 'b', 'c', 'd', 'e', 'f']


def calc_unique_id():
    src_bytes = random.randbytes(8)
    result = ''
    for byte in src_bytes:
        result += hex_transition[byte >> 4]
        result += hex_transition[byte & 0x0f]
    return result


def produce_exports_impl():
    print(f'void {class_name}::onGetModule(binder::Module& __mod) {{')
    for line in insert_lines:
        print(line)
    for export in export_symbols:
        if export['expr'] == None:
            continue
        setter = 'set'
        if export['const'] == True:
            setter = 'set_const'
        elif export['static'] == True:
            setter = 'set_static'
        name = export['name']
        expr = export['expr']
        print(f'    __mod.{setter}("{name}", {expr});')
    print('}')

    print('namespace {')
    print('const char *__g_this_exports[] = {')
    for export in export_symbols:
        name = export['name']
        print(f'   "{name}",')
    print('   nullptr')
    print('};')
    print(f'const char *__g_unique_id = "{calc_unique_id()}";')
    print('} // namespace anonymous')
    print(f'const char **{class_name}::onGetExports() {{ return __g_this_exports; }}')
    print(f'const char *{class_name}::onGetUniqueId() {{ return __g_unique_id; }}')


for header in header_lines:
    print(header)
print(f'namespace {namespace_id} {{')
produce_exports_impl()
print(f'}} // namespace {namespace_id}')
