#ifndef COCOA_GALLIUM_BINDINGS_CORE_FDRANDOMIZE_H
#define COCOA_GALLIUM_BINDINGS_CORE_FDRANDOMIZE_H

#include "Gallium/Gallium.h"
GALLIUM_BINDINGS_NS_BEGIN

/**
 * FDLR (File Descriptor Layout Randomization):
 * JavaScript can NOT access system's file descriptor (real file descriptor)
 * for safety.
 */
struct FDLRTable
{
    using Closer = void (*)(int32_t);
    enum OwnerType
    {
        kSystem,
        kUser,
        kUnknown
    };

    struct Target
    {
        int32_t     fd;
        OwnerType   owner;
        Closer      closer;
        bool        used : 1;
    } *pMap;
    size_t allocatedCount;
};

#define FDLR_MAP_SIZE           sizeof(FDLRTable::Target)
#define FDLR_MAP_INITIAL_SIZE   128

int32_t FDLRNewRandomizedDescriptor(int32_t realFd, FDLRTable::OwnerType owner, FDLRTable::Closer closer);
void FDLRMarkUnused(int32_t rfd);
FDLRTable::Target *FDLRGetUnderlyingDescriptor(int32_t rfd);
void FDLRInitialize();
void FDLRCollectAndSweep();
void FDLRDumpMappingInfo();


GALLIUM_BINDINGS_NS_END
#endif //COCOA_GALLIUM_BINDINGS_CORE_FDRANDOMIZE_H
