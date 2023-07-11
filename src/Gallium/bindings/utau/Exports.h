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

#ifndef COCOA_GALLIUM_BINDINGS_UTAU_EXPORTS_H
#define COCOA_GALLIUM_BINDINGS_UTAU_EXPORTS_H

#define GALLIUM_BINDINGS_UTAU_NS_BEGIN   namespace cocoa::gallium::bindings::utau_wrap {
#define GALLIUM_BINDINGS_UTAU_NS_END     }

#include <list>
#include <thread>
#include <queue>
#include <mutex>

#include "uv.h"

#include "Gallium/bindings/Base.h"
#include "Gallium/Gallium.h"
#include "Gallium/binder/Convert.h"
#include "Core/UniquePersistent.h"
#include "Utau/AudioBuffer.h"
#include "Utau/AVFilterDAG.h"
#include "Utau/VideoBuffer.h"
#include "Utau/AVStreamDecoder.h"
#include "Utau/AudioDevice.h"
#include "Utau/AudioSinkStream.h"
GALLIUM_BINDINGS_UTAU_NS_BEGIN

void SetInstanceProperties(v8::Local<v8::Object> instance);

//! TSEnumDecl: Enum<SampleFormat> := Constants.SAMPLE_FORMAT_*
//! TSEnumDecl: Enum<ChannelMode> := Constants.CH_MODE_*
//! TSEnumDecl: Enum<StreamSelector> := Constants.STREAM_SELECTOR_*
//! TSEnumDecl: Enum<DecodeBufferType> := Constants.DECODE_BUFFER_*
//! TSEnumDecl: Enum<MediaType> := Constants.MEDIA_TYPE_*
//! TSEnumDecl: Enum<AudioSinkStreamBufferState> := Constants.AUDIO_SINKSTREAM_BUFFER_*

//! TSDecl:
//! interface Rational {
//!   num: number;
//!   denom: number;
//! }

v8::Local<v8::Object> MakeRational(v8::Isolate *i, int32_t num, int32_t denom);
utau::Ratio ExtractRational(v8::Isolate *i, v8::Local<v8::Value> v);

//! TSDecl: function getCurrentTimestampMs(): number
uint64_t getCurrentTimestampMs();

//! TSDecl: class AudioDevice
class AudioDeviceWrap
{
public:
    explicit AudioDeviceWrap(std::shared_ptr<utau::AudioDevice> device)
        : device_(std::move(device)) {}
    ~AudioDeviceWrap() = default;

    //! TSDecl: function ConnectPipeWire(): AudioDevice
    static v8::Local<v8::Value> ConnectPipeWire();

    //! TSDecl: function unref(): void
    void unref();

    //! TSDecl: function createSinkStream(name: string): AudioSinkStream
    v8::Local<v8::Value> createSinkStream(const std::string& name);

private:
    std::shared_ptr<utau::AudioDevice> device_;
};

//! TSDecl: class AudioSinkStream
class AudioSinkStreamWrap
{
public:
    class JSListener;

    explicit AudioSinkStreamWrap(std::unique_ptr<utau::AudioSinkStream> st);
    ~AudioSinkStreamWrap() = default;

    g_nodiscard g_inline utau::AudioSinkStream *GetStream() const {
        return stream_.get();
    }

    //! TSDecl: function dispose(): void
    void dispose();

    //! TSDecl: function connect(sampleFormat: Enum<SampleFormat>,
    //!                          channelMode: Enum<ChannelMode>,
    //!                          sampleRate: number,
    //!                          realtime: boolean): void
    void connect(int32_t sample_fmt, int32_t ch_mode,
                 int32_t sample_rate, bool realtime);

    //! TSDecl: function disconnect(): void
    void disconnect();

    //! TSDecl: function enqueue(buffer: AudioBuffer): void
    void enqueue(v8::Local<v8::Value> buffer);

    //! TSDecl: function getCurrentDelayInUs(): number
    double getCurrentDelayInUs();

    //! TSDecl: volume: number
    float getVolume();
    void setVolume(float volume);

    //! TSDecl: onVolumeChanged: (volume: number) => void
    void setOnVolumeChanged(v8::Local<v8::Value> value);
    v8::Local<v8::Value> getOnVolumeChanged();

private:
    std::unique_ptr<utau::AudioSinkStream> stream_;
    std::shared_ptr<JSListener> listener_;
};

//! TSDecl: class AudioBuffer
class AudioBufferWrap
{
public:
    explicit AudioBufferWrap(std::shared_ptr<utau::AudioBuffer> buffer);
    ~AudioBufferWrap();

    g_nodiscard g_inline std::shared_ptr<utau::AudioBuffer> GetBuffer() {
        return buffer_;
    }

    //! TSDecl: readonly pts: number
    int64_t getPTS();

    //! TSDecl: readonly sampleFormat: Enum<SampleFormat>
    int32_t getSampleFormat();

    //! TSDecl: readonly channelMode: Enum<ChannelMode>
    int32_t getChannelMode();

    //! TSDecl: readonly sampleRate: number
    int32_t getSampleRate();

    //! TSDecl: readonly samplesCount: number
    int32_t getSamplesCount();

    //! TSDecl: function read(plane: number, sampleCount: number, sampleOffset: number,
    //!                       dstBytesOffset: number, dst: ArrayBuffer): number
    int32_t read(int32_t plane, int32_t sample_count, int32_t sample_offset,
                 size_t dst_bytes_offset, v8::Local<v8::Value> dst);

    //! TSDecl: function readChannel(ch: number, sampleCount: number, sampleOffset: number,
    //!                              dstBytesOffset: number, dst: ArrayBuffer): number
    int32_t readChannel(int32_t ch, int32_t sample_count, int32_t sample_offset,
                        size_t dst_bytes_offset, v8::Local<v8::Value> dst);

    //! TSDecl: function dispose(): void
    void dispose();

private:
    size_t approximate_size_;
    std::shared_ptr<utau::AudioBuffer>    buffer_;
};

//! TSDecl: class AudioFilterDAG
class AVFilterDAGWrap
{
public:
    explicit AVFilterDAGWrap(std::unique_ptr<utau::AVFilterDAG> DAG)
        : DAG_(std::move(DAG)) {}
    ~AVFilterDAGWrap() = default;

    //! TSDecl:
    //! interface InBufferParameters {
    //!   name: string;
    //!   mediaType: Enum<MediaType>;
    //!
    //!   sampleFormat?: Enum<SampleFormat>;
    //!   channelMode?: Enum<ChannelMode>;
    //!   sampleRate?: number;
    //!
    //!   pixelFormat?: number;
    //!   hwFrameContextFrom?: VideoBuffer;
    //!   width?: number;
    //!   height?: number;
    //!   timeBase?: Rational;
    //!   SAR?: Rational;
    //! }

    //! TSDecl:
    //! interface OutBufferParameters {
    //!   name: string;
    //!   mediaType: Enum<MediaType>;
    //! }

    //! TSDecl: function MakeFromDSL(dsl: string,
    //!                              inparams: Array<InBufferParameters>,
    //!                              outparams: Array<OutBufferParameters>): AVFilterDAG
    static v8::Local<v8::Value> MakeFromDSL(const std::string& dsl,
                                            v8::Local<v8::Value> inparams,
                                            v8::Local<v8::Value> outparams);

    //! TSDecl: function filter(inBuffers: Array<DAGNamedInOutBuffer>): Array<DAGNamedInOutBuffer>
    v8::Local<v8::Value> filter(v8::Local<v8::Value> inbuffers);

private:
    std::unique_ptr<utau::AVFilterDAG> DAG_;
};

//! TSDecl: class VideoBuffer
class VideoBufferWrap
{
public:
    explicit VideoBufferWrap(std::shared_ptr<utau::VideoBuffer> buffer);
    ~VideoBufferWrap();

    g_nodiscard g_inline std::shared_ptr<utau::VideoBuffer> GetBuffer() const {
        return buffer_;
    }

    //! TSDecl: function dispose(): void
    void dispose();

    //! TSDecl: readonly pts: number
    g_nodiscard g_inline int64_t getPTS() {
        return buffer_->GetFramePTS();
    }

private:
    size_t approximate_size_;
    std::shared_ptr<utau::VideoBuffer> buffer_;
};

//! TSDecl: class AVStreamDecoder
class AVStreamDecoderWrap
{
public:
    explicit AVStreamDecoderWrap(std::unique_ptr<utau::AVStreamDecoder> decoder)
        : decoder_(std::move(decoder)) {}
    ~AVStreamDecoderWrap() = default;

    g_nodiscard utau::AVStreamDecoder *GetDecoder() const {
        return decoder_.get();
    }

    //! TSDecl:
    //! interface DecoderOptions {
    //!   disableAudio?: boolean;
    //!   disableVideo?: boolean;
    //!   useHWDecoding?: boolean;
    //!   audioCodecName?: string;
    //!   videoCodecName?: string;
    //! }

    //! TSDecl: function MakeFromFile(path: string, options: DecoderOptions): AVStreamDecoder
    static v8::Local<v8::Value> MakeFromFile(const std::string& path,
                                             v8::Local<v8::Value> options);

    //! TSDecl: readonly hasAudioStream: boolean
    bool hasAudioStream();

    //! TSDecl: readonly hasVideoStream: boolean
    bool hasVideoStream();

    //! TSDecl:
    //! interface StreamInfo {
    //!   timeBase: Rational;
    //!   duration: number;
    //!   sampleFormat?: Enum<SampleFormat>;
    //!   channelMode?: Enum<ChannelMode>;
    //!   sampleRate?: number;
    //! }

    //! TSDecl: function getStreamInfo(selector: Enum<StreamSelector>): StreamInfo
    v8::Local<v8::Value> getStreamInfo(int32_t selector);

    //! TSDecl:
    //! interface DecodeBuffer {
    //!   type: Enum<DecodeBufferType>;
    //!   audio?: AudioBuffer;
    //!   video?: VideoBuffer;
    //! }

    //! TSDecl: function decodeNextFrame(): DecodeBuffer
    v8::Local<v8::Value> decodeNextFrame();

    //! TSDecl: function seekStreamTo(selector: Enum<StreamSelector>, ts: number): void
    void seekStreamTo(int32_t selector, int64_t ts);

    //! TSDecl: function flushDecoderBuffers(selector: Enum<StreamSelector>): void
    void flushDecoderBuffers(int32_t selector);

private:
    std::unique_ptr<utau::AVStreamDecoder> decoder_;
};

//! TSDecl: class MediaFramePresentDispatcher
class MediaFramePresentDispatcher : public MaybeGCRootObject<MediaFramePresentDispatcher>
{
public:
    struct PresentThreadContext;
    struct PresentThreadCmd;

    //! TSDecl: constructor(decoder: AVStreamDecoder, audioSinkStream: AudioSinkStream)
    explicit MediaFramePresentDispatcher(v8::Local<v8::Value> decoder,
                                         v8::Local<v8::Value> audioSinkStream);
    ~MediaFramePresentDispatcher();

    //! TSDecl: onPresentVideoBuffer: (buffer: VideoBuffer, ptsInSeconds: number) => void
    void setOnPresentVideoBuffer(v8::Local<v8::Value> func);
    v8::Local<v8::Value> getOnPresentVideoBuffer();

    //! TSDecl: onAudioPresentNotify: (buffer: AudioBuffer, ptsInSeconds: number) => void
    void setOnAudioPresentNotify(v8::Local<v8::Value> func);
    v8::Local<v8::Value> getOnAudioPresentNotify();

    //! TSDecl: onErrorOrEOF: () => void
    void setOnErrorOrEOF(v8::Local<v8::Value> func);
    v8::Local<v8::Value> getOnErrorOrEOF();

    //! TSDecl: play(): void
    void play();

    //! TSDecl: pause(): void
    void pause();

    //! TSDecl: seekTo(tsSeconds: number): void
    void seekTo(double tsSeconds);

    //! TSDecl: dispose(): void
    void dispose();

private:
    void ThreadRoutine();
    static void PresentRequestHandler(uv_async_t *handle);
    static void PresentThreadCmdHandler(uv_async_t *handle);
    static void TimerCallback(uv_timer_t *timer);

    void SendAndWaitForPresentThreadCmd(int verb, const int64_t param[3]);
    void SendPresentRequest(utau::AVStreamDecoder::AVGenericDecoded frame, double pts_seconds);
    void SendErrorOrEOFRequest();

    struct PresentRequest
    {
        bool error_or_eof;
        uint64_t send_timestamp;
        double frame_pts_seconds;
        std::shared_ptr<utau::VideoBuffer> vbuffer;
        std::shared_ptr<utau::AudioBuffer> abuffer;
    };

    v8::Global<v8::Object>          decoder_js_obj_;
    AVStreamDecoderWrap            *decoder_wrap_;
    v8::Global<v8::Object>          asinkstream_js_obj_;
    AudioSinkStreamWrap            *asinkstream_wrap_;
    v8::Global<v8::Function>        cb_present_video_buffer_;
    v8::Global<v8::Function>        cb_audio_present_notify_;
    v8::Global<v8::Function>        cb_error_or_eof_;

    bool                            disposed_;
    bool                            paused_;
    bool                            has_audio_;
    bool                            has_video_;
    utau::AVStreamDecoder::StreamInfo audio_stinfo_;
    utau::AVStreamDecoder::StreamInfo video_stinfo_;

    std::thread                     mp_thread_;
    uv_async_t                     *host_notifier_;
    std::mutex                      present_queue_lock_;
    std::queue<PresentRequest>      present_queue_;
    std::unique_ptr<PresentThreadContext> thread_ctx_;
};

GALLIUM_BINDINGS_UTAU_NS_END
#endif //COCOA_GALLIUM_BINDINGS_UTAU_EXPORTS_H
