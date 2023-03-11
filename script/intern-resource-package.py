#!/usr/bin/env python

import sys

if len(sys.argv) != 2:
    print('Usage: intern-resource-package.py <crpkg file>')
    exit(1)

print('#include <stdint.h>')
print('#include <stdlib.h>')

print('__attribute__((visibility("default"))) const uint8_t kInternedCRPKGBytes[] = {')

size: int = 0
with open(sys.argv[1], 'rb') as f:
    while byte := f.read(1):
        print('  0x' + byte.hex() + ',')
        size += 1

print('};')

print(f'__attribute__((visibility("default"))) const size_t kInternedCRPKGSize = {size};')
