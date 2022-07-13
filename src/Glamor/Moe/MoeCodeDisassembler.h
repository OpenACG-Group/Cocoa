#ifndef COCOA_GLAMOR_MOE_MOECODEDISASSEMBLER_H
#define COCOA_GLAMOR_MOE_MOECODEDISASSEMBLER_H

#include "Glamor/Glamor.h"
#include "Glamor/Moe/MoeByteStreamReader.h"
GLAMOR_NAMESPACE_BEGIN

class MoeCodeDisassembler
{
public:
    static std::string Disassemble(Unique<MoeByteStreamReader> reader);
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_MOE_MOECODEDISASSEMBLER_H
