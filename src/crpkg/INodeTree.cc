#include <cstring>

#include "crpkg/INodeTree.h"
CRPKG_BEGIN_NS

INodeBase::INodeBase(std::string name, Kind kind)
    : fName(std::move(name)),
      fKind(kind)
{
}

std::string INodeBase::name() const
{
    return fName;
}

INodeBase::Kind INodeBase::kind() const
{
    return fKind;
}

// ------------------------------------------------------------------------------

INodeDirectory::INodeDirectory(std::string name)
    : INodeBase(std::move(name), Kind::kDirectory)
{
}

INodeDirectory::~INodeDirectory()
{
    for (INodeBase *pNode : fChildren)
        delete pNode;
}

INodeBase *INodeDirectory::append(std::string name, Kind kind)
{
    INodeBase *pBase = nullptr;
    switch (kind)
    {
    case Kind::kDirectory:
        pBase = new INodeDirectory(std::move(name));
        break;

    case Kind::kFile:
        pBase = new INodeFile(std::move(name));
        break;
    }

    fChildren.push_back(pBase);
    return pBase;
}

void INodeDirectory::remove(std::string name)
{
    fChildren.remove_if([&name](INodeBase *pBase) -> bool {
        return pBase->name() == name;
    });
}

std::list<INodeBase*>::iterator INodeDirectory::begin()
{
    return fChildren.begin();
}

std::list<INodeBase *>::iterator INodeDirectory::end()
{
    return fChildren.end();
}

// --------------------------------------------------------------------------------------

INodeRoot::INodeRoot()
    : INodeDirectory("<root>")
{
}

// --------------------------------------------------------------------------------------

INodeFile::Content::Content(Content&& rhs) noexcept
{
    this->perm = rhs.perm;
    this->compressor = std::move(rhs.compressor);
    std::memcpy(this->md5sum, rhs.md5sum, sizeof(this->md5sum));
}

INodeFile::Content& INodeFile::Content::operator=(Content&& rhs) noexcept
{
    this->perm = rhs.perm;
    this->compressor = std::move(rhs.compressor);
    std::memcpy(this->md5sum, rhs.md5sum, sizeof(this->md5sum));

    return *this;
}

INodeFile::INodeFile(std::string name)
    : INodeBase(std::move(name), Kind::kFile)
{
}

CRPKG_END_NS
