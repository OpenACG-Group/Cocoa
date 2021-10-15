#include "Vanilla/Base.h"
#include "Vanilla/RenderKit/LayerPropertiesGroup.h"
#include "Vanilla/RenderKit/Layer.h"
#include "Vanilla/RenderKit/ContentAggregator.h"
VANILLA_NS_BEGIN

namespace {
uint32_t gLayerIdCounter = 1;
} // namespace anonymous

Layer::Layer(const WeakHandle<ContentAggregator>& aggregator, bool opaque,
             uint32_t x, uint32_t y, uint32_t w, uint32_t h)
    : fId(gLayerIdCounter++)
    , fDisposed(false)
    , fProperties(std::make_unique<LayerPropertiesGroup>(WeakHandle<Layer>(), opaque, x, y, w, h))
    , fAggregator(aggregator)
{
}

void Layer::use()
{
    fProperties->setAttachedLayer(shared_from_this());
}

void Layer::dispose()
{
    if (!fDisposed)
    {
        fDisposed = true;
        this->onDispose();
    }
}

VANILLA_NS_END
