#include "Glamor/Layers/Layer.h"
GLAMOR_NAMESPACE_BEGIN

namespace {

uint32_t get_next_unique_id()
{
    static uint32_t counter = 1;
    return counter++;
}

} // namespace cocoa

Layer::Layer()
    : paint_bounds_(SkRect::MakeEmpty())
    , unique_id_(get_next_unique_id())
{
}

void Layer::Preroll(PrerollContext *context, const SkMatrix& matrix) {}

GLAMOR_NAMESPACE_END
