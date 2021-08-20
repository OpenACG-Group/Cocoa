#include "Core/Project.h"
#include "Core/Exception.h"
#include "Vanilla/Base.h"
#include "Vanilla/Audio/AuEffectorsDAG.h"
#include "Vanilla/Audio/AuEffector.h"

co_cdecl_begin
#include <libavfilter/avfilter.h>
co_cdecl_end

VANILLA_NS_BEGIN

AuEffectorsDAG::AuEffectorsDAG()
    : fGraph(nullptr)
{
    fGraph = ::avfilter_graph_alloc();
    if (!fGraph)
        throw VanillaException(__func__, "Failed to allocate a filter graph");
}

AuEffectorsDAG::~AuEffectorsDAG()
{
    if (fGraph)
        ::avfilter_graph_free(&fGraph);
}

bool AuEffectorsDAG::instantiate()
{
    int ret = ::avfilter_graph_config(fGraph, nullptr);
    return (ret >= 0);
}

bool AuEffectorsDAG::link(const AuEffector& src, int srcOutConnector,
                          const AuEffector& dst, int dstInConnector)
{
    if (srcOutConnector >= src.outputConnectors() ||
        dstInConnector >= dst.inputConnectors())
    {
        return false;
    }

    int ret = ::avfilter_link(src._getFilter(),
                              srcOutConnector,
                              dst._getFilter(),
                              dstInConnector);
    return (ret == 0);
}

VANILLA_NS_END
