#include <random>
#include <map>
#include <cassert>
#include <sstream>

#include "Core/Journal.h"
#include "Core/CpuInfo.h"
#include "Koi/lang/CoreFDLR.h"

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Koi.lang)

KOI_LANG_NS_BEGIN

struct FDLRTable gFDLRTable;

int32_t FDLRNewRandomizedDescriptor(int32_t realFd, FDLRTable::OwnerType owner, FDLRTable::Closer closer)
{
    std::vector<std::array<size_t, 2>> unusedRegions;
    for (size_t i = 0; i < gFDLRTable.allocatedCount; i++)
    {
        if (gFDLRTable.pMap[i].used)
            continue;
        size_t l = i, r = i;
        while (!gFDLRTable.pMap[r].used && r < gFDLRTable.allocatedCount)
            r++;
        unusedRegions.push_back({l, r - 1});
        i = r - 1;
    }

    if (unusedRegions.empty())
    {
        gFDLRTable.allocatedCount *= 2;
        gFDLRTable.pMap = static_cast<FDLRTable::Target*>(std::realloc(gFDLRTable.pMap,
                                                                       gFDLRTable.allocatedCount * FDLR_MAP_SIZE));
        assert(gFDLRTable.pMap);
        for (size_t i = gFDLRTable.allocatedCount / 2; i < gFDLRTable.allocatedCount; i++)
        {
            gFDLRTable.pMap[i].used = false;
            gFDLRTable.pMap[i].closer = nullptr;
        }
        LOGF(LOG_DEBUG, "Expanding FDLR allocation map memory to {} bytes", gFDLRTable.allocatedCount)
        unusedRegions.push_back({gFDLRTable.allocatedCount / 2, gFDLRTable.allocatedCount - 1});
    }

    unsigned long long seed;
    if (CpuInfo::Ref().getX86Info().features.rdrnd)
    {
        __builtin_ia32_rdrand64_step(&seed);
    }
    else
    {
        seed = 2233;
    }
    std::mt19937 twisterEngine(seed);
    std::uniform_int_distribution<size_t> generator(0, unusedRegions.size() - 1);
    std::array<size_t, 2> targetRange{};
    {
        size_t idx = generator(twisterEngine);
        targetRange = unusedRegions[idx];
    }

    generator = std::uniform_int_distribution<size_t>(targetRange[0], targetRange[1]);
    auto finalFd = static_cast<int32_t>(generator(twisterEngine));
    gFDLRTable.pMap[finalFd].used = true;
    gFDLRTable.pMap[finalFd].fd = realFd;
    gFDLRTable.pMap[finalFd].owner = owner;
    gFDLRTable.pMap[finalFd].closer = closer;
    return finalFd;
}

void FDLRMarkUnused(int32_t rfd)
{
    if (rfd >= gFDLRTable.allocatedCount)
        return;
    gFDLRTable.pMap[rfd].used = false;
    gFDLRTable.pMap[rfd].closer = nullptr;
}

FDLRTable::Target *FDLRGetUnderlyingDescriptor(int32_t rfd)
{
    if (rfd >= gFDLRTable.allocatedCount || !gFDLRTable.pMap[rfd].used)
    {
        return nullptr;
    }
    return &gFDLRTable.pMap[rfd];
}

void FDLRInitialize()
{
    gFDLRTable.pMap = static_cast<FDLRTable::Target*>(std::malloc(FDLR_MAP_SIZE * FDLR_MAP_INITIAL_SIZE));
    gFDLRTable.allocatedCount = FDLR_MAP_INITIAL_SIZE;
    for (size_t i = 0; i < gFDLRTable.allocatedCount; i++)
    {
        gFDLRTable.pMap[i].used = false;
        gFDLRTable.pMap[i].closer = nullptr;
    }
}

void FDLRCollectAndSweep()
{
    for (size_t i = 0; i < gFDLRTable.allocatedCount; i++)
    {
        if (gFDLRTable.pMap[i].used && gFDLRTable.pMap[i].closer)
        {
            LOGF(LOG_WARNING, "Unclosed (randomized) file descriptor #{}", i)
            gFDLRTable.pMap[i].closer(gFDLRTable.pMap[i].fd);
        }
    }
    std::free(gFDLRTable.pMap);
}

void FDLRDumpMappingInfo()
{
    size_t usedCount = 0;
    for (size_t i = 0; i < gFDLRTable.allocatedCount; i++)
    {
        if (gFDLRTable.pMap[i].used)
            usedCount++;
    }
    LOGF(LOG_DEBUG, "%fg<hl>FDLR subsystem statistics: {} entries, {} used%reset", gFDLRTable.allocatedCount, usedCount)

    size_t p = 0;
    for (size_t i = 0; i < gFDLRTable.allocatedCount; i++)
    {
        if (gFDLRTable.pMap[i].used)
        {
            std::ostringstream os;
            if (gFDLRTable.pMap[i].owner == FDLRTable::kSystem)
                os << "%fg<re,hl>system%reset<>,";
            else if (gFDLRTable.pMap[i].closer)
                os << "%fg<gr,hl>closable%reset<>,";

            if (gFDLRTable.pMap[i].fd == STDIN_FILENO)
                os << "%fg<bl,hl>stdin%reset<>,";
            else if (gFDLRTable.pMap[i].fd == STDOUT_FILENO)
                os << "%fg<bl,hl>stdout%reset<>,";
            else if (gFDLRTable.pMap[i].fd == STDERR_FILENO)
                os << "%fg<bl,hl>stderr%reset<>,";

            std::string_view str = os.view();
            if (str.length() > 0)
                str.remove_suffix(1);
            LOGF(LOG_DEBUG, "  Entry#{} %fg<ma,hl>{:03d}%reset -> %fg<cy,hl>{:03d}%reset [{}]",
                 p++, i, gFDLRTable.pMap[i].fd, str)
        }
    }
}


KOI_LANG_NS_END

