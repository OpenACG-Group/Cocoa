#!/usr/bin/env python

# Usage: intern.py <namespace> <symbol name> <crpkg file>
import os.path
import sys

if len(sys.argv) == 1:
    print('Usage: intern.py <crpkg file...>')
    exit(1)

interned_syms = []
interned_sym_sizes = []


def intern_single_file(file_name: str):
    sym = os.path.basename(file_name).replace('.', '_')
    interned_syms.append(sym)

    print(f'static uint8_t {sym}[] = {{')
    size: int = 0
    with open(file_name, 'rb') as f:
        while byte := f.read(1):
            print('  0x' + byte.hex() + ',')
            size += 1
    print(f'}}; // {sym}')
    interned_sym_sizes.append(size)


print('#include <cstdint>')
print('namespace cocoa {')

for file in sys.argv[1:]:
    intern_single_file(file)

print('uint8_t *qresource_autoload_tbl[] = {')
for s in interned_syms:
    print(f'  {s},')
print('}; // qresource_autoload_tbl')

print('std::size_t qresource_autoload_size_tbl[] = {')
for s in interned_sym_sizes:
    print(f'  {s},')
print('}; // qresource_autoload_size_tbl')
print(f'std::size_t qresource_autoload_tbl_size = {len(interned_syms)};')
print('} // namespace cocoa')
