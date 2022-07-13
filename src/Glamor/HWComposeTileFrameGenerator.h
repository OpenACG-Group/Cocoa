#ifndef COCOA_GLAMOR_HWCOMPOSETILEFRAMEGENERATOR_H
#define COCOA_GLAMOR_HWCOMPOSETILEFRAMEGENERATOR_H

#include <future>
#include <optional>

#include "include/core/SkDeferredDisplayList.h"
#include "include/core/SkSurface.h"
#include "include/core/SkImage.h"

#include "Glamor/Glamor.h"
#include "Glamor/FrameGeneratorBase.h"
GLAMOR_NAMESPACE_BEGIN

class HWComposeTileFrameGenerator : public FrameGeneratorBase
{
public:
    explicit HWComposeTileFrameGenerator(const Shared<Blender>& blender);
    ~HWComposeTileFrameGenerator() override = default;

    void OnPaint(SkSurface *surface, const sk_sp<SkPicture> &picture,
                 const SkIRect &rect) override;

private:
    struct TileBlock
    {
        using Future = std::future<sk_sp<SkDeferredDisplayList>>;

        SkIRect             tile_rect;
        sk_sp<SkSurface>    backend_texture;
        std::optional<Future> future_dlist;
    };

    std::vector<TileBlock>      tile_blocks_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_HWCOMPOSETILEFRAMEGENERATOR_H
