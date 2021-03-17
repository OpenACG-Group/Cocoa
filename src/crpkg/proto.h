#ifndef COCOA_PROTO_H
#define COCOA_PROTO_H

#include <cstdint>
#include <string>

#include "crpkg/crpkg.h"
CRPKG_PROTO_BEGIN_NS

/**
 * The structure of .crpkg.blob file:
 *
 * +----------------+
 * |   Header       | ---+
 * +----------------+    | first_inode_offset
 * +----------------+ <--+
 * |  INode #0      |
 * | (Directory)    | ---+
 * +----------------+    | file_inode_offsets
 *  <Other inodes>       |
 * +----------------+ <--+
 * |  INode #N      |
 * |   (File)       | ---+
 * +----------------+    |
 *  <Other inodes>       | data_offset
 *   <Other data>        |
 * +----------------+ <--+
 * | Data header    |
 * +----------------+
 * |    Data        |
 * | (Compressed)   |
 * +----------------+
 *  <Other data>
 */

#define CRPKG_HDR_MAGIC         { 'c', 'r', 'p', 'k', 'g', 0x7f }
#define CRPKG_HDR_MAGIC_SIZE    6
#define CRPKG_HDR_PKGNAME_SIZE  32
#define CRPKG_HDR_CURRENT_MAJOR 1
#define CRPKG_HDR_CURRENT_MINOR 103
struct Header
{
    uint8_t     magic[CRPKG_HDR_MAGIC_SIZE];
    int8_t      package_name[CRPKG_HDR_PKGNAME_SIZE];
    uint8_t     major_version;
    uint8_t     minor_version;
    uint32_t    symbol_count;
    uint32_t    symbol_table_offset;
    uint32_t    inode_count;
    uint32_t    inode_table_offset;
};

struct Symbol
{
    uint32_t    symbol_size;
    int8_t      symbol[];
};

#define CRPKG_INODE_FILE        1
#define CRPKG_INODE_DIRECTORY   0
#define CRPKG_INODE_NAME_SIZE   128
struct INode
{
    uint8_t     inode_type;
    uint32_t    inode_symbol_offset;
    // uint8_t     inode_name[CRPKG_INODE_NAME_SIZE];
};

struct DirectoryINode
{
    INode       base;
    uint32_t    contains_count;
    uint32_t    contains_inode_offsets[];
};

#define CRPKG_INODE_PERMISSION_W    0x01    /* File is readable */
#define CRPKG_INODE_PERMISSION_R    0x02    /* File is writable */
#define CRPKG_INODE_PERMISSION_X    0x04    /* File is executable */
#define CRPKG_INODE_PERMISSION_L    0x08    /* File is loadable */
#define CRPKG_INODE_PERMISSION_U    0x10    /* File shouldn't be verified by MD5 */
struct FileINode
{
    INode       base;
    uint8_t     uncompressed_md5sum[16];
    uint8_t     permission;
    uint32_t    data_offset;
};

#define CRPKG_FILEDATA_MAGIC    0x3a
struct FileData
{
    uint8_t     magic;
    uint64_t    compressed_size;
    uint8_t     compressed_data[];
};

CRPKG_PROTO_END_NS
#endif //COCOA_PROTO_H
