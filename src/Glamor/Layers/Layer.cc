/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

#include "Glamor/Layers/Layer.h"
GLAMOR_NAMESPACE_BEGIN

namespace {

uint32_t get_next_unique_id()
{
    static std::atomic<uint32_t> counter = 1;
    return counter++;
}

} // namespace cocoa

Layer::Layer(Type type)
    : layer_type_(type)
    , paint_bounds_(SkRect::MakeEmpty())
    , unique_id_(get_next_unique_id())
    , generation_id_(0)
{
}

uint64_t Layer::IncreaseGenerationId()
{
    return ++generation_id_;
}

bool Layer::IsComparableWith(Layer *other) const
{
    return other->layer_type_ == layer_type_;
}

void Layer::Preroll(PrerollContext *context, const SkMatrix& matrix) {}

void Layer::ToString(std::ostream& out)
{
    out << "(unknown-layer)";
}

GLAMOR_NAMESPACE_END
