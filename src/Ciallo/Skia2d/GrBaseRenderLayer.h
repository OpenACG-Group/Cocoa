#ifndef __BASE_RENDERLAYER_H__
#define __BASE_RENDERLAYER_H__

#include <memory>
#include <ostream>
#include <string>
#include <Poco/UUID.h>

#include "include/core/SkPicture.h"
#include "include/core/SkPaint.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRect.h"

#include "Core/Exception.h"
#include "Ciallo/Ciallo.h"
CIALLO_BEGIN_NS

class GrBaseCompositor;

class GrBaseRenderLayer
{
    friend GrBaseCompositor;

    struct DirtyBoundary
    {
        int32_t    fLeft = 0;
        int32_t    fTop = 0;
        int32_t    fBottom = 0;
        int32_t    fRight = 0;
    };

    struct LayerProperties
    {
        int32_t    fWidth = 0;
        int32_t    fHeight = 0;
        int32_t    fLeft = 0;
        int32_t    fTop = 0;
        int        fZIndex;
        bool       fVisible = false;
        Poco::UUID fUUID;
    };

public:
    virtual ~GrBaseRenderLayer() = default;

    inline int32_t top() const      { return fProperties.fTop; }
    inline int32_t left() const     { return fProperties.fLeft; }
    inline int32_t width() const    { return fProperties.fWidth; }
    inline int32_t height() const   { return fProperties.fHeight; }
    inline int32_t zindex() const   { return fProperties.fZIndex; }
    inline bool visible() const     { return fProperties.fVisible; }
    inline Poco::UUID uuid() const  { return fProperties.fUUID; }

    /**
     * @brief  Increase or decrease the Z-index value of this layer.
     * 
     * The Z-index value doesn't change continuously. For example,
     * supposing the Z-index is -3 now, and it may not change to -2
     * after calling elevate(). It may be -2, -1, 0 and so on.
     * 
     * Note that it also changes other layer's Z-index value.
     * @return The new Z-index value of this.
     */
    int elevate();
    int depress();

    /**
     * @brief Set the layer's visibility.
     * 
     * The visibility of a layer just describes whether the content
     * of layer will be composited. It doesn't effect any other things.
     * Caller can still paint pictures on the layer and but they won't
     * be displayed utill you make the layer visible.
     * 
     * @param visible: A boolean value. True means visible,
     *                 false means invisible.
     */
    void setVisibility(bool visible);

    /**
     * @brief Change the position of layer, can be out of screen.
     * 
     * It effects nothing until you call update().
     *
     * @param px: x coordinate.
     * @param py: y coordinate.
     */
    void moveTo(int32_t px, int32_t py);

    /**
     * @brief Make a snapshot and submit it to compositor.
     * 
     * Each layer has its own Skia surface object and canvas
     * object. All the drawing operations are done on this
     * surface. After you drawing something on the layer,
     * you must call update() to submit these changes to compositor,
     * or the new content will never be displayed.
     * But this doesn't mean the new content will be displayed
     * as soon as you call update(). To display it, call
     * present() method of compositor.
     */
    void update();

    /**
     * @brief Playback drawing operations by SkPicture.
     * 
     * @param picture: SkPicture object.
     * @param left: Offset on x coordinate.
     * @param top: Offset on y coordinate.
     * @param paint: SkPaint to apply transparency, filtering and so on.
     *               Maybe nullptr.
    */
    void paint(const sk_sp<SkPicture>& picture,
               int32_t left,
               int32_t top,
               const SkPaint *paint = nullptr);

    void drawImageFile(const std::string& file,
                       int32_t x = 0,
                       int32_t y = 0,
                       int32_t width = -1,
                       int32_t height = -1,
                       const SkPaint *paint = nullptr);

protected:
    GrBaseRenderLayer(GrBaseCompositor *compositor,
                      int32_t x, int32_t y, int32_t z,
                      int32_t w, int32_t h);

    virtual sk_sp<SkImage> onLayerResult() = 0;
    virtual SkCanvas *onCreateCanvas() = 0;

    inline SkIRect dirtyBoundary() const
    {
        return SkIRect::MakeLTRB(fDirtyBoundary.fLeft,
                                 fDirtyBoundary.fTop,
                                 fDirtyBoundary.fRight,
                                 fDirtyBoundary.fBottom);
    }

private:
    void setZIndex(int z);
    void updateDirtyBoundary(const SkIRect& rect);
    SkCanvas *getCanvas();

private:
    GrBaseCompositor        *fCompositor;
    LayerProperties          fProperties;
    DirtyBoundary            fDirtyBoundary;
    SkCanvas                *fCanvas;
};

CIALLO_END_NS
#endif // __BASE_RENDERLAYER_H__
