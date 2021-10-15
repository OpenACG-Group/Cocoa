#ifndef COCOA_CONTENTAGGREGATOR_H
#define COCOA_CONTENTAGGREGATOR_H

#include <list>
#include <map>
#include <optional>
#include <chrono>

#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSurface.h"
#include "include/core/SkImageInfo.h"

#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class Layer;
class DrawContext;
class LayerFactory;

class ContentAggregator : public std::enable_shared_from_this<ContentAggregator>
{
public:
    explicit ContentAggregator(Handle<DrawContext> ctx);
    ~ContentAggregator();
    static Handle<ContentAggregator> Make(const Handle<DrawContext>& ctx);

    Handle<Layer> pushLayer(const LayerFactory& factory);
    Handle<Layer> insertLayerBefore(uint32_t id, const LayerFactory& factory);
    Handle<Layer> getLayerById(uint32_t id);
    void removeLayer(const Handle<Layer>& layer);

    inline void setBackgroundFillColor(SkColor color) {
        fBGFillColor = color;
    }

    inline void setForceSamplingOptions(const SkSamplingOptions& options) {
        fForceSamplingOptions = options;
    }

    inline void resetForceSamplingOptions() {
        fForceSamplingOptions.reset();
    }

    va_nodiscard inline Handle<DrawContext> getDrawContext() const {
        return fDrawContext;
    }

    va_nodiscard SkColorType getScreenColorType();

    sk_sp<SkSurface> requestBackendSurface(const Handle<Layer>& layer,
                                           const SkImageInfo& info, SkBudgeted budgeted);
    void releaseBackendSurface(const Handle<Layer>& layer,
                               const sk_sp<SkSurface>& surface);

    va_nodiscard inline float getFps() const {
        return fFps;
    }

    void update(const SkRect& region);
    void dispose();

private:
    template<typename T>
    struct SkSpHash
    {
        size_t operator()(const sk_sp<T>& val) const {
            return reinterpret_cast<size_t>(val.get());
        }
    };

    bool                        fDisposed;
    std::list<Handle<Layer>>    fLayerList;
    std::unordered_map<sk_sp<SkSurface>, uint32_t, SkSpHash<SkSurface>>
                                fLayerOwnedSurfacesMap;
    Handle<DrawContext>         fDrawContext;
    SkColor                     fBGFillColor;
    std::optional<SkSamplingOptions>
                                fForceSamplingOptions;
    std::chrono::steady_clock::time_point
                                fLastUpdateTp;
    uint32_t                    fFrameCounter;
    float                       fFps;
};

VANILLA_NS_END
#endif //COCOA_CONTENTAGGREGATOR_H
