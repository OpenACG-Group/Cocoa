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

#ifndef COCOA_GALLIUM_BINDINGS_MULTIMEDIA_ACHANNELLAYOUT_H
#define COCOA_GALLIUM_BINDINGS_MULTIMEDIA_ACHANNELLAYOUT_H

#include "Gallium/bindings/multimedia/Types.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Class.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

//! @tsdocbegin
//! An immutable object holding information about the channel layout of audio data.
//!
//! A channel layout here is defined as a set of channels ordered in a specific
//! way (unless the channel order is unspecified), in which case an `AChannelLayout`
//! carries only the channel count).
//! All orders may be treated as if they were unspecified by ignoring everything
//! but the channel count.
//!
//! Channel names (C=center, R=right, L=left, Rc=right of center, Lc=left of center, Sur=surround):
//!   FrontL FrontR FrontC
//!   LowFreq
//!   BackL BackR FrontLc FrontRc BackC
//!   SideL SideR
//!   TopC TopFrontL TopFrontC TopFrontR TopBackL TopBackC TopBackR
//!   L R (for stereo downmix)
//!   WideL WideR
//!   SurDirectL SurDirectR
//!   LowFreq2
//!   TopSideL TopSideR
//!   BottomFrontC BottomFrontL BottomFrontR
//!
//! Note that these channel names are case-insensitive.
//!
//! @tsdocend
//! TSDecl: @class @nonconstructible AChannelLayout
class AChannelLayout : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)
    
    enum CtorFlags
    {
        kNotFreeOriginal = 0x01
    };
    
    // Copies `ch` using `av_channel_layout_copy()`, and frees its contents
    // using `av_channel_layout_uninit()`. So `ch` will be reset.
    explicit AChannelLayout(AVChannelLayout& ch, int flags = 0);
    ~AChannelLayout() override;

    void CopyLayoutTo(AVChannelLayout& dst);

    g_nodiscard const AVChannelLayout& GetAVChannelLayout() const {
        return layout_;
    }

    //! @tsdocbegin
    //! Creates a channel layout from a specific predefined layout `name`.
    //! Valid layout names are (C=center, R=right, L=left, Rc=right of center, Lc=left of center):
    //!   - mono:           [1ch ]
    //!   - stereo:         [2ch ] FrontL FrontR
    //!   - 2.1:            [3ch ] stereo LowFreq
    //!   - 2-1:            [3ch ] stereo BackC
    //!   - surround:       [3ch ] stereo FrontC
    //!   - 3.1:            [4ch ] surround LowFreq
    //!   - 4.0:            [4ch ] surround BackC
    //!   - 4.1:            [5ch ] 4.0 LowFreq
    //!   - 2-2:            [4ch ] stereo SideL SideR
    //!   - quad:           [4ch ] stereo BackL BackR
    //!   - 5.0:            [5ch ] surround SideL SideR
    //!   - 5.1:            [6ch ] 5.0 LowFreq
    //!   - 5.0-back:       [5ch ] surround BackL BackR
    //!   - 5.1-back:       [6ch ] 5.0-back LowFreq
    //!   - 6.0:            [6ch ] 5.0 BackC
    //!   - 6.0-front:      [6ch ] 2-2 FrontLc FrontRc
    //!   - hexagonal:      [6ch ] 5.0-back BackC
    //!   - 6.1:            [7ch ] 5.1 BackC
    //!   - 6.1-back:       [7ch ] 5.1-back BackC
    //!   - 6.1-front:      [7ch ] 6.0-front LowFreq
    //!   - 7.0:            [7ch ] 5.0 BackL BackR
    //!   - 7.0-front:      [7ch ] 5.0 FrontLc FrontRc
    //!   - 7.1:            [8ch ] 5.1 BackL BackR
    //!   - 7.1-wide:       [8ch ] 5.1 FrontLc FrontRc
    //!   - 7.1-wide-back:  [8ch ] 5.1-back FrontLc FrontRc
    //!   - 7.1-top-back:   [8ch ] 5.1-back TopFrontL TopFrontR
    //!   - octagonal:      [8ch ] 5.0 BackL BackC BackR
    //!   - cube:           [8ch ] quad TopFrontL TopFrontR TopBackL TopBackR
    //!   - hexadecagonal:  [16ch] octagonal WideL WideR TopBackL TopBackR TopBackC TopFrontL TopFrontR
    //!   - stereo-downmix: [2ch ] L R
    //!   - 22.2:           [24ch] 5.1-back FrontLc FrontRc BackC LowFreq2 SideL SideR
    //!                            TopFrontL TopFrontR TopFrontC TopC TopBackL TopBackR
    //!                            TopSideL TopSideR TopBackC BottomFrontC BottomFrontL
    //!                            BottomFrontR
    //!
    //! @tsdocend
    //! TSDecl: @method @static Predefined(name: string): AChannelLayout
    static ffi::RetLocal<v8::Value> Predefined(const std::string& name);

    //! TSDecl: @property @readonly channels: i32
    ffi::Ret<int32_t> getChannels() {
        return layout_.nb_channels;
    }

    //! @tsdocbegin
    //! An ordered list of strings that describes each channel.
    //! If the channel order is unspecified, returns an empty array.
    //! @tsdocend
    //! TSDecl: @property @readonly orderedChannels: @array(string)
    ffi::RetLocal<v8::Value> getOrderedChannels();

    //! TSDecl: @method clone(): AChannelLayout
    ffi::RetLocal<v8::Value> clone();

    //! TSDecl: @method equalTo(other: AChannelLayout): boolean
    ffi::Ret<bool> equalTo(const ffi::Class<AChannelLayout>& other);

    //! @tsdocbegin
    //! Find out what channels from a given set are present in the channel layout,
    //! without regard for their positions.
    //! @tsdocend
    //! TSDecl: @method intersect(ch: @array(string)): @array(string)
    ffi::RetLocal<v8::Value> intersect(const std::vector<std::string>& ch);

private:
    AVChannelLayout layout_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
#endif //COCOA_GALLIUM_BINDINGS_MULTIMEDIA_ACHANNELLAYOUT_H
