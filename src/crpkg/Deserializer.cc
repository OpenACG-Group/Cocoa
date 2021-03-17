#include <cstring>
#include <iostream>

#include "Core/Exception.h"
#include "crpkg/crpkg.h"
#include "crpkg/proto.h"
#include "crpkg/Deserializer.h"
CRPKG_BEGIN_NS

CompressedData::CompressedData(uint8_t *md5sum, uint16_t perm,
                               std::streampos dataPos,
                               size_t dataSize,
                               std::shared_ptr<std::istream> sharedStream)
        : fMd5sum{0},
          fPermission(perm),
          fDataPos(dataPos),
          fDataSize(dataSize),
          fSharedStream(std::move(sharedStream))
{
    // Copy MD5 manually
    std::memcpy(fMd5sum, md5sum, sizeof(fMd5sum));
}

CompressedData::CompressedData(CompressedData&& other) noexcept
        : fMd5sum{0},
          fPermission(other.fPermission),
          fDataPos(other.fDataPos),
          fDataSize(other.fDataSize),
          fSharedStream(std::move(other.fSharedStream))
{
    std::memcpy(fMd5sum, other.fMd5sum, sizeof(fMd5sum));
}

CompressedData::CompressedData(const CompressedData& other)
        : fMd5sum{0},
          fPermission(other.fPermission),
          fDataPos(other.fDataPos),
          fDataSize(other.fDataSize),
          fSharedStream(other.fSharedStream)
{
    std::memcpy(fMd5sum, other.fMd5sum, sizeof(fMd5sum));
}

// -----------------------------------------------------------------------------

Deserializer::ScopedSaveAndRestore::ScopedSaveAndRestore(Deserializer *pDeserializer)
    : fpDeserializer(pDeserializer)
{
    fpDeserializer->save();
}

Deserializer::ScopedSaveAndRestore::~ScopedSaveAndRestore()
{
    fpDeserializer->restore();
}

// -------------------------------------------------------------------------------

Deserializer::Deserializer(const std::string& file)
    : fFile(file)
{
    checkSuffix();

    fStream = std::make_shared<std::ifstream>(file, std::ios::binary);
    if (!fStream->is_open())
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("crpkg: Failed to open file: ")
                .append(fFile)
                .make<RuntimeException>();
    }
}

void Deserializer::checkSuffix()
{
    static const std::string suffix(".crpkg.blob");
    if (fFile.size() <= suffix.size())
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("crpkg: Bad suffix (extension) name: ")
                .append(fFile)
                .make<RuntimeException>();
    }

    auto suffixItr = suffix.crbegin();
    auto fileItr = fFile.crbegin();
    for (; suffixItr != suffix.crend(); suffixItr++, fileItr++)
    {
        if (*suffixItr != *fileItr)
        {
            throw RuntimeException::Builder(__FUNCTION__)
                    .append("crpkg: Bad suffix (extension) name: ")
                    .append(fFile)
                    .make<RuntimeException>();
        }
    }
}

void Deserializer::tryRead(void *buf, size_t size)
{
    fStream->read(reinterpret_cast<char*>(buf), size);
    if (!fStream->good())
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("crpkg: Failed to read, still needs more than ")
                .append(size)
                .append(" bytes: ")
                .make<RuntimeException>();
    }
}

void Deserializer::save()
{
    fReadPtrStack.push(fStream->tellg());
}

void Deserializer::restore()
{
    if (fReadPtrStack.empty())
        return;
    fStream->seekg(fReadPtrStack.top());
    fReadPtrStack.pop();
}

std::string Deserializer::extractSymbol(uint64_t offset_in_table)
{
    ScopedSaveAndRestore saved(this);
    fStream->seekg(offset_in_table);

    ScopedSaveAndRestore saved2(this);
    proto::Symbol sym{};
    tryRead(&sym, sizeof(sym));

    int8_t name[sym.symbol_size];
    tryRead(name, sym.symbol_size);

    return std::string(reinterpret_cast<char*>(name));
}

void Deserializer::extractRoot(uint32_t inodeOffset, PropertyTreeDirNode *node)
{
    ScopedSaveAndRestore save(this);
    fStream->seekg(inodeOffset);

    proto::DirectoryINode inode{};
    tryRead(&inode, sizeof(inode));

    if (inode.base.inode_type != CRPKG_INODE_DIRECTORY)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("crpkg: Root inode should be a directory")
                .make<RuntimeException>();
    }

    std::string sym = extractSymbol(inode.base.inode_symbol_offset);
    if (sym != "<root>")
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("crpkg: Root inode should be named <root>")
                .make<RuntimeException>();
    }

    uint32_t offsets[inode.contains_count];
    tryRead(offsets, inode.contains_count * sizeof(uint32_t));

    for (int32_t i = 0; i < inode.contains_count; i++)
        extractINode(offsets[i], node);
}

void Deserializer::extractINode(uint32_t inodeOffset, PropertyTreeDirNode *node)
{
    ScopedSaveAndRestore save(this);
    fStream->seekg(inodeOffset);

    proto::INode inode{};
    std::string name;
    {
        ScopedSaveAndRestore save2(this);
        tryRead(&inode, sizeof(inode));
        name = extractSymbol(inode.inode_symbol_offset);
    }

    std::cout << "inode: " << name << std::endl;

    if (inode.inode_type == CRPKG_INODE_DIRECTORY)
    {
        auto *dirNode = PropertyTreeNode::NewDirNode(node, name)->cast<PropertyTreeDirNode>();

        proto::DirectoryINode dirINode{};
        tryRead(&dirINode, sizeof(dirINode));

        uint32_t offsets[dirINode.contains_count];
        tryRead(offsets, dirINode.contains_count * sizeof(uint32_t));

        for (int32_t i = 0; i < dirINode.contains_count; i++)
            extractINode(offsets[i], dirNode);
    }
    else if (inode.inode_type == CRPKG_INODE_FILE)
    {
        proto::FileINode fileINode{};
        tryRead(&fileINode, sizeof(fileINode));

        extractFileData(fileINode, name, node);
    }
}

void Deserializer::extractFileData(proto::FileINode& inode, const std::string& name,
                                   PropertyTreeDirNode *node)
{
    ScopedSaveAndRestore save(this);
    fStream->seekg(inode.data_offset);

    proto::FileData fileData{};
    tryRead(&fileData, sizeof(fileData));
    if (fileData.magic != CRPKG_FILEDATA_MAGIC)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("crpkg: Invalid data structure [Bad magic number]")
                .make<RuntimeException>();
    }

    CompressedData data(inode.uncompressed_md5sum,
                        inode.permission,
                        fStream->tellg(),
                        fileData.compressed_size,
                        fStream);
    PropertyTreeNode::NewDataNode(node, name, data);
}

void Deserializer::extractTo(PropertyTreeDirNode *node)
{
    fStream->seekg(0);

    proto::Header hdr{};
    tryRead(&hdr, sizeof(hdr));

    static const uint8_t expectMagic[] = CRPKG_HDR_MAGIC;
    if (std::strncmp((const char*)hdr.magic, (const char *)expectMagic,
                     CRPKG_HDR_MAGIC_SIZE) != 0)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("crpkg: Not a crpkg blob file: ")
                .append(fFile)
                .make<RuntimeException>();
    }

    if (hdr.major_version != CRPKG_HDR_CURRENT_MAJOR ||
        hdr.minor_version != CRPKG_HDR_CURRENT_MINOR)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("crpkg: Uncompatible version of file: ")
                .append(fFile)
                .make<RuntimeException>();
    }

    auto *packageNode = PropertyTreeNode::NewDirNode(
            node,reinterpret_cast<char*>(hdr.package_name))->cast<PropertyTreeDirNode>();
    extractRoot(hdr.inode_table_offset, packageNode);
}

CRPKG_END_NS
