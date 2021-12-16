#ifndef COCOA_LAYERPROPERTIESGROUP_H
#define COCOA_LAYERPROPERTIESGROUP_H

#include <tuple>
#include "Core/Errors.h"

#include "include/core/SkRect.h"
#include "include/core/SkRRect.h"
#include "include/core/SkM44.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkPath.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkShader.h"
#include "include/core/SkClipOp.h"

#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class Layer;

class LayerPropertiesGroup
{
public:
    enum class Clipping
    {
        kNone,
        kRRect,     /* Round rectangle clipping */
        kRect,      /* Normal rectangle clipping */
        kPath       /* Path clipping */
    };

    LayerPropertiesGroup(WeakHandle<Layer>&& layer, bool opaque, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
        : fLayer(layer)
        , fX(x)
        , fY(y)
        , fWidth(w)
        , fHeight(h)
        , fOpaque(opaque)
        , fAlpha(1.0f)
        , fClipping(Clipping::kNone)
        , fClipOp(SkClipOp::kIntersect)
        , fClipAA(false)
        , fClipRect(SkRect::MakeEmpty())
        , fClipRRect(SkRRect::MakeEmpty())
        , fHasMatrix(false)
        , fMatrixAA(false)
        , fImageFilter(nullptr)
        , fColorFilter(nullptr)
        , fShader(nullptr) {}
    ~LayerPropertiesGroup() = default;

    inline void setAttachedLayer(WeakHandle<Layer>&& layer) {
        fLayer = layer;
    }

    va_nodiscard inline Handle<Layer> getAttachedLayer() {
        return fLayer.lock();
    }

    va_nodiscard inline std::tuple<uint32_t, uint32_t> getPosition() {
        return {fX, fY};
    }

    va_nodiscard inline std::tuple<uint32_t, uint32_t> getDimension() {
        return {fWidth, fHeight};
    }

    va_nodiscard inline bool isOpaque() const {
        return fOpaque;
    }

    va_nodiscard SkScalar getAlphaValue() const {
        return fAlpha;
    }

    va_nodiscard inline Clipping getClippingType() const {
        return fClipping;
    }

    va_nodiscard inline SkClipOp getClipOp() const {
        return fClipOp;
    }

    va_nodiscard inline bool isClipAA() const {
        return fClipAA;
    }

    va_nodiscard inline const SkRect& getClipRect() const {
        return fClipRect;
    }

    va_nodiscard inline const SkRRect& getClipRRect() const {
        return fClipRRect;
    }

    va_nodiscard inline const SkPath& getClipPath() const {
        return fClipPath;
    }

    va_nodiscard inline bool hasMatrix() const {
        return fHasMatrix;
    }

    va_nodiscard inline const SkM44& getMatrix() const {
        return fMatrix;
    }

    va_nodiscard inline bool isMatrixAA() const {
        return fMatrixAA;
    }

    va_nodiscard inline const SkSamplingOptions& getSamplingOptions() {
        return fSamplingOptions;
    }

    va_nodiscard inline sk_sp<SkImageFilter> getImageFilter() const {
        return fImageFilter;
    }

    va_nodiscard inline sk_sp<SkColorFilter> getColorFilter() const {
        return fColorFilter;
    }

    va_nodiscard inline sk_sp<SkShader> getShader() const {
        return fShader;
    }

    inline void setPosition(uint32_t x, uint32_t y) {
        fX = x;
        fY = y;
    }

    inline void setDimension(uint32_t w, uint32_t h) {
        fWidth = w;
        fHeight = h;
    }

    inline void setAlphaValue(SkScalar alpha) {
        fAlpha = alpha;
    }

    inline void setClipOp(SkClipOp clipOp) {
        fClipOp = clipOp;
    }

    inline void setClipAA(bool aa) {
        fClipAA = aa;
    }

    inline void setClip(const SkRect& rect) {
        fClipping = Clipping::kRect;
        fClipRect = rect;
    }

    inline void setClip(const SkRRect& rect) {
        fClipping = Clipping::kRRect;
        fClipRRect = rect;
    }

    inline void setClip(const SkPath& path) {
        fClipping = Clipping::kPath;
        fClipPath = path;
    }

    inline void resetClip() {
        fClipping = Clipping::kNone;
    }

    inline void setMatrix(const SkM44& matrix) {
        fHasMatrix = true;
        fMatrix = matrix;
    }

    inline void setMatrixAA(bool aa) {
        fMatrixAA = aa;
    }

    inline void resetMatrix() {
        fHasMatrix = false;
    }

    inline void setSamplingOptions(const SkSamplingOptions& opts) {
        fSamplingOptions = opts;
    }

    inline void setImageFilter(const sk_sp<SkImageFilter>& filter) {
        fImageFilter = filter;
    }

    /**
     * Use @p filter as SkColorFilter while ContentAggregator is compositing layers.
     * If a shader is used by @p setShader(), any SkColorFilter will be ignored.
     */
    inline void setColorFilter(const sk_sp<SkColorFilter>& filter) {
        fColorFilter = filter;
    }

    /**
     * Use @p shader as SkShader while ContentAggregator is compositing layers.
     * If a shader is used, any SkColorFilter ( @p setColorFilter() ) will be ignored.
     */
    inline void setShader(const sk_sp<SkShader>& shader) {
        fShader = shader;
    }

private:
    WeakHandle<Layer>        fLayer;
    uint32_t                 fX;
    uint32_t                 fY;
    uint32_t                 fWidth;
    uint32_t                 fHeight;
    bool                     fOpaque;
    SkScalar                 fAlpha;
    Clipping                 fClipping;
    SkClipOp                 fClipOp;
    bool                     fClipAA;
    SkRect                   fClipRect;
    SkRRect                  fClipRRect;
    SkPath                   fClipPath;
    bool                     fHasMatrix;
    SkM44                    fMatrix;
    bool                     fMatrixAA;
    SkSamplingOptions        fSamplingOptions;
    sk_sp<SkImageFilter>     fImageFilter;
    sk_sp<SkColorFilter>     fColorFilter;
    sk_sp<SkShader>          fShader;
};

VANILLA_NS_END
#endif //COCOA_LAYERPROPERTIESGROUP_H
