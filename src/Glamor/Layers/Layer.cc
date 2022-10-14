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

void Layer::ToString(std::ostream& out)
{
    out << "(unknown-layer)";
}

GLAMOR_NAMESPACE_END
