#ifndef COCOA_LAYER_H
#define COCOA_LAYER_H

#include <list>
#include "Core/Errors.h"

#include "include/core/SkImage.h"

#include "Vanilla/Base.h"
#include "Vanilla/RenderKit/LayerPropertiesGroup.h"
VANILLA_NS_BEGIN

class ContentAggregator;

#define T_LAYER_OBJECT(type) \
static Handle<type> Cast(const Handle<Layer>& ptr) { \
    return std::dynamic_pointer_cast<type>(ptr);     \
}

class Layer : public std::enable_shared_from_this<Layer>
{
public:
    virtual ~Layer() = default;
    void use();

    va_nodiscard inline UniqueHandle<LayerPropertiesGroup>& getProperties() {
        return fProperties;
    }

    va_nodiscard inline uint32_t getLayerId() const {
        return fId;
    }

    va_nodiscard inline Handle<ContentAggregator> getContentAggregator() {
        CHECK(!fAggregator.expired());
        return fAggregator.lock();
    }

    va_nodiscard inline bool isDisposed() const {
        return fDisposed;
    }

    void dispose();

    virtual sk_sp<SkImage> onGetImage(const SkIRect& bounds) = 0;
    virtual void onPreComposite() = 0;
    virtual void onPostComposite() = 0;

protected:
    virtual void onDispose() = 0;

    Layer(const WeakHandle<ContentAggregator>& aggregator,
          bool opaque,
          uint32_t x, uint32_t  y, uint32_t w, uint32_t h);

private:
    uint32_t                fId;
    bool                    fDisposed;
    UniqueHandle<LayerPropertiesGroup>
                            fProperties;
    WeakHandle<ContentAggregator>
                            fAggregator;
};

VANILLA_NS_END
#endif //COCOA_LAYER_H
