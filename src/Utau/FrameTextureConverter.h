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

#ifndef COCOA_UTAU_GLEXTENSIONVIDEOLAYER_H
#define COCOA_UTAU_GLEXTENSIONVIDEOLAYER_H

#define FFWRAP_AVUTIL_USE_HWCONTEXT_VAAPI

#include <unordered_map>

#include "Core/UniquePersistent.h"
#include "Glamor/Layers/ExternalTextureLayer.h"
#include "Utau/Utau.h"
#include "Utau/ffwrappers/libavutil.h"
#include "Utau/PixelFormatUtils.h"
UTAU_NAMESPACE_BEGIN

std::unique_ptr<gl::ExternalTextureLayer>
WrapFrameToGLExternalLayer(AVFrame *frame, const SkPoint& offset,
                           const SkISize& dimensions, ScaleResampler resampler);

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_GLEXTENSIONVIDEOLAYER_H
