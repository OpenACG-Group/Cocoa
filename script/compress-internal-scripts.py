#!/usr/bin/env python3

import sys
import os


if len(sys.argv) != 2:
    print("Invalid number of arguments")
    exit(1)

internals_dir = sys.argv[1]
if not os.path.isdir(internals_dir):
    print("Internal directory not found")
    exit(1)

if os.path.isfile('internals.sfs'):
    os.remove('internals.sfs')

compress_cmd = f'mksquashfs {internals_dir}/* internals.sfs -no-progress -comp gzip >mksquashfs_log.txt 2>&1'
if os.system(compress_cmd) != 0:
    print("Failed to compress internal script files, see mksquashfs_log.txt for more details")
    exit(1)

if not os.path.isfile('internals.sfs'):
    print("Could not find compressed internal script files")
    exit(1)


print(r'#include <cstdint>')
print(r'extern "C" const uint8_t __koi_internals_sfs_compressed[] = {')
size: int = 0
with open('internals.sfs', 'rb') as f:
    while (byte := f.read(1)):
        print('  0x' + byte.hex() + ',')
        size += 1
print(r'};')
print(r'extern "C" const std::size_t __koi_internals_sfs_size = ' + str(size) + ';')
