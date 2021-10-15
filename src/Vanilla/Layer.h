#ifndef COCOA_VANILLA_PIPELINE_LAYER_H
#define COCOA_VANILLA_PIPELINE_LAYER_H

#include "include/core/SkRect.h"

#include "Vanilla/Base.h"
#include "Vanilla/KeySymbols.h"
#include "Vanilla/Typetraits.h"
VANILLA_NS_BEGIN

class Layer
{
public:
    enum class Propagate
    {
        kSwallowed,     /* The event shouldn't be passed to lower layers */
        kContinue       /* The event can be passed to lower layers */
    };

    Layer(const SkRect& region, bool visible);
    virtual ~Layer() = default;

    va_nodiscard inline const SkRect& getRegion() const
    { return fRegion; }
    va_nodiscard inline bool isVisible() const
    { return fVisible; }
    inline void setVisible(bool visible)
    { fVisible = visible; }

    virtual bool isOpaque() = 0;

    virtual Propagate onWindowMap() { return Propagate::kContinue; }
    virtual Propagate onWindowUnmap() { return Propagate::kContinue; }
    virtual Propagate onWindowClose() { return Propagate::kContinue; }
    virtual Propagate onWindowConfigure(const SkRect& geometry) { return Propagate::kContinue; }
    virtual Propagate onRepaint(const SkRect& rect) { return Propagate::kContinue; }
    virtual Propagate onButtonPress(Button btn, vec::float2 pos) { return Propagate::kContinue; }
    virtual Propagate onButtonRelease(Button btn, vec::float2 pos) { return Propagate::kContinue; }
    virtual Propagate onMotion(vec::float2 pos) { return Propagate::kContinue; }
    virtual Propagate onTouchBegin(vec::float2 pos) { return Propagate::kContinue; }
    virtual Propagate onTouchEnd(vec::float2 pos) { return Propagate::kContinue; }
    virtual Propagate onTouchUpdate(vec::float2 pos) { return Propagate::kContinue; }
    virtual Propagate onKeyPress(KeySymbol key, Bitfield<KeyModifier> mod) { return Propagate::kContinue; }
    virtual Propagate onKeyRelease(KeySymbol key, Bitfield<KeyModifier> mod) { return Propagate::kContinue; }

private:
    SkRect      fRegion;
    bool        fVisible;
};

VANILLA_NS_END
#endif //COCOA_VANILLA_PIPELINE_LAYER_H
