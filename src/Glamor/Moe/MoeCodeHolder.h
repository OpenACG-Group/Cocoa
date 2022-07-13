#ifndef COCOA_GLAMOR_MOECODEHOLDER_H
#define COCOA_GLAMOR_MOECODEHOLDER_H

#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

class MoeCodeHolder
{
public:
    virtual ~MoeCodeHolder() = default;

    virtual const uint8_t *GetStartAddress() = 0;
    virtual size_t GetLength() = 0;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_MOECODEHOLDER_H
