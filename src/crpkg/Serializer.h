#ifndef COCOA_SERIALIZER_H
#define COCOA_SERIALIZER_H

#include <string>
#include <vector>
#include <memory>

#include "crpkg/crpkg.h"
#include "crpkg/INodeTree.h"
CRPKG_BEGIN_NS

class Serializer
{
public:
    explicit Serializer(INodeRoot *root);
    void write(const std::string& file, const std::string& packageName);

private:
    std::vector<FlatINode::Ptr>     fFlattened;
};

CRPKG_END_NS
#endif //COCOA_SERIALIZER_H
