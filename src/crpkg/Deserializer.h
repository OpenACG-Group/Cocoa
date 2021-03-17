#ifndef COCOA_DESERIALIZER_H
#define COCOA_DESERIALIZER_H

#include <fstream>
#include <string>
#include <stack>
#include <functional>

#include "Core/PropertyTree.h"
#include "crpkg/crpkg.h"
#include "crpkg/proto.h"
CRPKG_BEGIN_NS

#define CRPKG_DESERIALIZE_DATA_READ_ONCE_SIZE   4096
class CompressedData
{
public:
    using FileDataBlockReceiver = std::function<void(void*, size_t)>;

    CompressedData(uint8_t *md5sum, uint16_t perm,
                   std::streampos dataPos,
                   size_t dataSize,
                   std::shared_ptr<std::istream> sharedStream);
    CompressedData(const CompressedData& other);
    CompressedData(CompressedData&& other) noexcept;

    bool checkMD5();
    void read(FileDataBlockReceiver receiver);

private:
    uint8_t                     fMd5sum[16];
    uint16_t                    fPermission;
    std::streampos              fDataPos;
    size_t                      fDataSize;
    std::shared_ptr<std::istream> fSharedStream;
};

class Deserializer
{
    friend class ScopedSaveAndRestore;

public:
    class ScopedSaveAndRestore
    {
    public:
        explicit ScopedSaveAndRestore(Deserializer *pDeserializer);
        ~ScopedSaveAndRestore();

    private:
        Deserializer    *fpDeserializer;
    };

    explicit Deserializer(const std::string& file);
    ~Deserializer() = default;

    void extractTo(PropertyTreeDirNode *node);

private:
    std::string extractSymbol(uint64_t offset_in_table);
    void extractRoot(uint32_t inodeOffset, PropertyTreeDirNode *node);
    void extractINode(uint32_t inodeOffset, PropertyTreeDirNode *node);
    void extractFileData(proto::FileINode& inode, const std::string& name, PropertyTreeDirNode *node);

    void save();
    void restore();
    void checkSuffix();
    void tryRead(void *buf, size_t size);

    std::shared_ptr<std::ifstream>  fStream;
    std::stack<std::streampos>      fReadPtrStack;
    std::string                     fFile;
};

CRPKG_END_NS
#endif //COCOA_DESERIALIZER_H
