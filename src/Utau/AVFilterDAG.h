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

#ifndef COCOA_UTAU_AUDIOFILTERDAG_H
#define COCOA_UTAU_AUDIOFILTERDAG_H

#include <string>
#include <vector>

#include "Utau/Utau.h"
#include "Utau/ffwrappers/libavutil.h"
UTAU_NAMESPACE_BEGIN

class AudioBuffer;
class VideoBuffer;

class AVFilterDAG
{
public:
    struct FilterDAGPriv;

    struct NamedInOutBuffer
    {
        std::string     name;
        MediaType       media_type;

        std::shared_ptr<AudioBuffer> audio_buffer;
        std::shared_ptr<VideoBuffer> video_buffer;
    };

    struct InBufferParameters
    {
        std::string         name;
        MediaType           media_type;

        /* Audio only */
        AudioChannelMode    channel_mode;
        SampleFormat        sample_fmt;
        int32_t             sample_rate;

        /* Video only */
        AVPixelFormat       pixel_fmt;
        AVBufferRef        *hw_frame_ctx;
        int32_t             width;
        int32_t             height;
        Ratio               time_base;
        Ratio               sar;
    };

    struct OutBufferParameters
    {
        std::string                     name;
        MediaType                       media_type;

        std::vector<SampleFormat>       sample_fmts;
        std::vector<int32_t>            sample_rates;
        std::vector<AudioChannelMode>   channel_modes;
    };

    static std::unique_ptr<AVFilterDAG> MakeFromDSL(const std::string& dsl,
                                                    const std::vector<InBufferParameters>& inparams,
                                                    const std::vector<OutBufferParameters>& outparams);

    AVFilterDAG();
    ~AVFilterDAG();

    g_nodiscard g_inline int32_t GetInputsCount() const {
        return inputs_count_;
    }

    g_nodiscard g_inline int32_t GetOutputsCount() const {
        return outputs_count_;
    }

    std::vector<NamedInOutBuffer> Filter(const std::vector<NamedInOutBuffer>& input);

private:
    std::unique_ptr<FilterDAGPriv>  priv_;

    int32_t         inputs_count_;
    int32_t         outputs_count_;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_AUDIOFILTERDAG_H
