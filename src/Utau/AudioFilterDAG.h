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
UTAU_NAMESPACE_BEGIN

class AudioBuffer;

class AudioFilterDAG
{
public:
    struct FilterDAGPriv;

    struct NamedInOutBuffer
    {
        std::string name;
        std::shared_ptr<AudioBuffer> buffer;
    };

    struct InBufferParameters
    {
        std::string         name;
        AudioChannelMode    channel_mode;
        SampleFormat        sample_fmt;
        int32_t             sample_rate;
    };

    struct OutBufferParameters
    {
        std::string                     name;
        std::vector<SampleFormat>       sample_fmts;
        std::vector<int32_t>            sample_rates;
        std::vector<AudioChannelMode>   channel_modes;
    };

    static std::unique_ptr<AudioFilterDAG> MakeFromDSL(const std::string& dsl,
                                                       const std::vector<InBufferParameters>& inparams,
                                                       const std::vector<OutBufferParameters>& outparams);

    AudioFilterDAG();
    ~AudioFilterDAG();

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
