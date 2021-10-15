#ifndef COCOA_LAYERFACTORIES_H
#define COCOA_LAYERFACTORIES_H

#include "Vanilla/Base.h"
#include "Vanilla/RenderKit/ImageAdaptationMethod.h"
VANILLA_NS_BEGIN

class Layer;
class PictureLayer;
class ContentAggregator;

#define LAYER_FACTORY_STD_ARG_LIST  bool _opaque, uint32_t _x, uint32_t _y, uint32_t _w, uint32_t _h
#define LAYER_FACTORY_CTOR_SUPERCLASS LayerFactory(_opaque, _x, _y, _w, _h)

class LayerFactory
{
public:
    LayerFactory(bool opaque, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
        : fOpaque(opaque)
        , fX(x)
        , fY(y)
        , fW(w)
        , fH(h) {}
    virtual ~LayerFactory() = default;

    va_nodiscard virtual Handle<Layer> create(const WeakHandle<ContentAggregator>& aggregator) const;

protected:
    va_nodiscard virtual Handle<Layer> onCreate(const WeakHandle<ContentAggregator>& aggregator) const = 0;

    bool        fOpaque;
    uint32_t    fX, fY, fW, fH;
};

class PictureLayerFactory : public LayerFactory
{
public:
    PictureLayerFactory(LAYER_FACTORY_STD_ARG_LIST)
        : LAYER_FACTORY_CTOR_SUPERCLASS {}
    ~PictureLayerFactory() override = default;

private:
    va_nodiscard Handle<Layer> onCreate(const WeakHandle<ContentAggregator> &aggregator) const override;
};

class ImageLayerFactory : public LayerFactory
{
public:
    ImageLayerFactory(ImageAdaptationMethod adaptation, LAYER_FACTORY_STD_ARG_LIST)
        : LAYER_FACTORY_CTOR_SUPERCLASS
        , fAdapt(adaptation) {}
    ~ImageLayerFactory() override = default;

private:
    va_nodiscard Handle<Layer> onCreate(const WeakHandle<ContentAggregator> &aggregator) const override;
    ImageAdaptationMethod   fAdapt;
};

VANILLA_NS_END
#endif //COCOA_LAYERFACTORIES_H
