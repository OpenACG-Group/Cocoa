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

import { Constants } from 'glamor';

export enum GskBlendMode {
    kClear = Constants.BLEND_MODE_CLEAR,
    kSrc = Constants.BLEND_MODE_SRC,
    kDst = Constants.BLEND_MODE_DST,
    kSrcOver = Constants.BLEND_MODE_SRC_OVER,
    kDstOver = Constants.BLEND_MODE_DST_OVER,
    kSrcIn = Constants.BLEND_MODE_SRC_IN,
    kDstIn = Constants.BLEND_MODE_DST_IN,
    kSrcOut = Constants.BLEND_MODE_SRC_OUT,
    kDstOut = Constants.BLEND_MODE_DST_OUT,
    kSrcATop = Constants.BLEND_MODE_SRC_ATOP,
    kDstATop = Constants.BLEND_MODE_DST_ATOP,
    kXor = Constants.BLEND_MODE_XOR,
    kPlus = Constants.BLEND_MODE_PLUS,
    kModulate = Constants.BLEND_MODE_MODULATE,
    kScreen = Constants.BLEND_MODE_SCREEN,
    kOverlay = Constants.BLEND_MODE_OVERLAY,
    kDarken = Constants.BLEND_MODE_DARKEN,
    kLighten = Constants.BLEND_MODE_LIGHTEN,
    kColorDodge = Constants.BLEND_MODE_COLOR_DODGE,
    kColorBurn = Constants.BLEND_MODE_COLOR_BURN,
    kHardLight = Constants.BLEND_MODE_HARD_LIGHT,
    kSoftLight = Constants.BLEND_MODE_SOFT_LIGHT,
    kDifference = Constants.BLEND_MODE_DIFFERENCE,
    kExclusion = Constants.BLEND_MODE_EXCLUSION,
    kHue = Constants.BLEND_MODE_HUE,
    kSaturation = Constants.BLEND_MODE_SATURATION,
    kColor = Constants.BLEND_MODE_COLOR,
    kLuminosity = Constants.BLEND_MODE_LUMINOSITY
}
