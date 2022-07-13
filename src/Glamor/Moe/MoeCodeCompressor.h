#ifndef COCOA_GLAMOR_MOE_MOECODECOMPRESSOR_H
#define COCOA_GLAMOR_MOE_MOECODECOMPRESSOR_H

#include "Core/Data.h"
#include "Glamor/Glamor.h"
#include "Glamor/Moe/MoeByteStreamReader.h"
GLAMOR_NAMESPACE_BEGIN


class MoeCodeCompressor
{
public:
    static Shared<Data> Compress(Unique<MoeByteStreamReader> reader);
    static Shared<Data> Decompress(const Shared<Data>& compressedData);
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_MOE_MOECODECOMPRESSOR_H
