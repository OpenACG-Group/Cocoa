#ifndef COCOA_INODETREE_H
#define COCOA_INODETREE_H

#include <string>
#include <list>
#include <memory>
#include <vector>
#include <functional>
#include <cstring>

#include "crpkg/crpkg.h"
#include "crpkg/Compression.h"
CRPKG_BEGIN_NS

class INodeBase
{
public:
    enum class Kind
    {
        kFile,
        kDirectory
    };

    virtual ~INodeBase() = default;

    std::string name() const;
    Kind kind() const;

    template<typename T>
    inline T *cast()
    {
        return dynamic_cast<T*>(this);
    }

protected:
    INodeBase(std::string name, Kind kind);

private:
    std::string     fName;
    Kind            fKind;
};

using FileDataReader = std::function<ssize_t(void*, size_t)>;
class INodeFile : public INodeBase
{
    friend class INodeDirectory;

public:
    struct Content
    {
        Content() = default;
        Content(const Content& other) = delete;
        Content& operator=(const Content& other) = delete;
        Content(Content&& rhs) noexcept;
        Content& operator=(Content&& rhs) noexcept;

        inline Content& setCompressor(CompressContext::PromiseResult val)
        {
            this->compressor = std::move(val);
            return *this;
        }

        inline Content& setPerm(uint16_t val)
        {
            this->perm = val;
            return *this;
        }

        inline Content& copyMD5(const uint8_t *val)
        {
            std::memcpy(this->md5sum, val, sizeof(md5sum));
            return *this;
        }

        CompressContext::PromiseResult compressor;
        uint16_t perm = 0;
        uint8_t md5sum[16] {0};
    };

    ~INodeFile() override = default;

    inline Content& content()
    {
        return fContent;
    }

private:
    explicit INodeFile(std::string name);

    Content     fContent;
};

class INodeDirectory : public INodeBase
{
public:
    ~INodeDirectory() override;

    INodeBase *append(std::string name, Kind kind);
    void remove(std::string name);

    inline INodeFile *appendFile(std::string name)
    {
        return this->append(std::move(name), Kind::kFile)->cast<INodeFile>();
    }

    inline INodeDirectory *appendDir(std::string name)
    {
        return this->append(std::move(name), Kind::kDirectory)->cast<INodeDirectory>();
    }

    std::list<INodeBase*>::iterator begin();
    std::list<INodeBase*>::iterator end();

protected:
    explicit INodeDirectory(std::string name);

private:
    std::list<INodeBase*>   fChildren;
};

class INodeRoot : public INodeDirectory
{
public:
    INodeRoot();
    ~INodeRoot() override = default;
};

struct FlatINode
{
    using Ptr = std::unique_ptr<FlatINode>;

    INodeBase::Kind     kind;
    std::vector<int>    childrenIdx;
    std::string         name;
    CompressContext::PromiseResult fCompressor;
    uint8_t             perm;
    uint8_t             md5sum[16];
};

CRPKG_END_NS
#endif //COCOA_INODETREE_H
