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

#ifndef COCOA_UTAU_AVSTREAMDECODER_H
#define COCOA_UTAU_AVSTREAMDECODER_H

#include <unordered_map>
#include <optional>

#include "Core/Data.h"
#include "Utau/Utau.h"
#include "Utau/AudioBuffer.h"
#include "Utau/VideoBuffer.h"
UTAU_NAMESPACE_BEGIN

class AVStreamDecoder
{
public:
    struct DataAVIOContextPriv;
    struct DecoderPriv;

    enum StreamSelector
    {
        kAudio_StreamType,
        kVideo_StreamType,

        kLast_StreamType = kVideo_StreamType
    };

    struct Options
    {
        bool disable_video = false;
        bool disable_audio = false;
        bool use_hw_decode = false;
        std::string video_codec_name;
        std::string audio_codec_name;
    };

    struct StreamInfo
    {
        using MetadataMap = std::unordered_map<std::string, std::string>;

        /* For all streams */
        MetadataMap     metadata;
        Ratio           time_base;
        int64_t         duration;

        /* For audio streams only */
        AudioChannelMode    channel_mode;
        SampleFormat        sample_fmt;
        int32_t             sample_rate;

        /* For video streams only */

        AVPixelFormat       pixel_fmt;
        int32_t             width;
        int32_t             height;
        Ratio               sar;
    };

    /**
     * It wraps `AudioBuffer` and `VideoBuffer` into a single structure,
     * representing an audio buffer OR a video buffer. Valid data is stored
     * in the buffer whose pointer is not nullptr.
     * The necessity of using this structure is that although both `AudioBuffer`
     * and `VideoBuffer` are inherited from `AVGenericBuffer`, they are not
     * polymorphic for performance reasons.
     */
    struct AVGenericDecoded
    {
        enum Type
        {
            kNull,
            kEOF,
            kAudio,
            kVideo
        };

        explicit AVGenericDecoded(Type type)
            : type(type), audio(nullptr) {}

        AVGenericDecoded(AVGenericDecoded&& other) noexcept
            : type(other.type), audio(std::move(other.audio)), video(std::move(other.video))
        {
            other.type = kNull;
        }

        AVGenericDecoded& operator=(AVGenericDecoded&& other) noexcept
        {
            type = other.type;
            other.type = kNull;
            audio = std::move(other.audio);
            video = std::move(other.video);
            return *this;
        }

        Type type;
        std::unique_ptr<AudioBuffer> audio;
        std::unique_ptr<VideoBuffer> video;
    };

    AVStreamDecoder();
    ~AVStreamDecoder();

    static std::unique_ptr<AVStreamDecoder> MakeFromData(const std::shared_ptr<Data>& data,
                                                         const Options& options);

    g_nodiscard g_inline bool HasVideoStream() const {
        return has_video_stream_;
    }

    g_nodiscard g_inline bool HasAudioStream() const {
        return has_audio_stream_;
    }

    g_nodiscard std::optional<StreamInfo> GetStreamInfo(StreamSelector selector);

    bool SeekStreamTo(StreamSelector stream, int64_t ts);
    bool FlushDecoderBuffers(StreamSelector stream);
    AVGenericDecoded DecodeNextFrame();

private:
    std::unique_ptr<DataAVIOContextPriv>    avio_context_priv_;
    std::unique_ptr<DecoderPriv>            decoder_priv_;

    bool        has_video_stream_;
    bool        has_audio_stream_;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_AVSTREAMDECODER_H
