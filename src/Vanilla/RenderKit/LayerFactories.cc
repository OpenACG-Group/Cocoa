#include "Vanilla/Base.h"
#include "Vanilla/RenderKit/LayerFactories.h"
#include "Vanilla/RenderKit/Layer.h"
#include "Vanilla/RenderKit/PictureLayer.h"
#include "Vanilla/RenderKit/ImageLayer.h"
VANILLA_NS_BEGIN

#define RESTARG(aggregator) aggregator, fOpaque, fX, fY, fW, fH

Handle<Layer> LayerFactory::create(const WeakHandle<ContentAggregator>& aggregator) const
{
    Handle<Layer> result = this->onCreate(aggregator);
    if (result)
        result->use();
    return result;
}

Handle<Layer> PictureLayerFactory::onCreate(const WeakHandle<ContentAggregator>& aggregator) const
{
    return std::make_shared<PictureLayer>(RESTARG(aggregator));
}

Handle<Layer> ImageLayerFactory::onCreate(const WeakHandle<ContentAggregator>& aggregator) const
{
    return std::make_shared<ImageLayer>(fAdapt, RESTARG(aggregator));
}

VANILLA_NS_END
