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

#include "Gallium/Gallium.h"
#include "Gallium/binder/Convert.h"

#include "Core/UniquePersistent.h"
#include "Utau/AudioBuffer.h"
#include "Utau/AudioSink.h"
#include "Utau/AudioFilterDAG.h"
#include "Utau/AVStreamDecoder.h"
GALLIUM_BINDINGS_UTAU_NS_BEGIN

void SetInstanceProperties(v8::Local<v8::Object> instance);

//! TSEnumDecl: Enum<SampleFormat> := Constants.SAMPLE_FORMAT_*
//! TSEnumDecl: Enum<ChannelMode> := Constants.CH_MODE_*
//! TSEnumDecl: Enum<StreamSelector> := Constants.STREAM_SELECTOR_*
//! TSEnumDecl: Enum<DecodeBufferType> := Constants.DECODE_BUFFER_*

//! TSDecl:
//! interface Rational {
//!   num: number;
//!   denom: number;
//! }

v8::Local<v8::Object> MakeRational(v8::Isolate *i, int32_t num, int32_t denom);
utau::Ratio ExtractRational(v8::Isolate *i, v8::Local<v8::Value> v);

class AudioSinkContext : public UniquePersistent<AudioSinkContext>
{
public:
    //! TSDecl: function Initialize(): void
    static void Initialize();

    //! TSDecl: function Dispose(call_from_listener: boolean): void
    static void Dispose(bool call_from_listener);

    //! TSDecl: function Enqueue(buffer: AudioBuffer): number
    static int32_t Enqueue(v8::Local<v8::Value> buffer);

    //! TSDecl: type BufferEventListenerCallback = (id: number) => void;

    //! TSDecl:
    //! interface BufferEventListener {
    //!   playing?: BufferEventListenerCallback;
    //!   cancelled?: BufferEventListenerCallback;
    //!   consumed?: BufferEventListenerCallback;
    //! }

    //! TSDecl: function AddBufferEventListener(listener: BufferEventListener): number
    static int32_t AddBufferEventListener(v8::Local<v8::Value> listener);

    //! TSDecl: function RemoveBufferEventListener(): void
    static void RemoveBufferEventListener(int32_t listenerId);

    struct JSBufferEventListener
            : public utau::AudioSink::BufferEventListener
    {
        using BufferWithId = utau::AudioSink::BufferWithId;

        explicit JSBufferEventListener(v8::Isolate *i);
        ~JSBufferEventListener() override = default;

        void OnPlaying(const BufferWithId &buf) override {
            InvokeJS(callbacks[0], buf.id);
        }

        void OnCancelled(const BufferWithId &buf) override {
            InvokeJS(callbacks[1], buf.id);
        }

        void OnConsumed(const BufferWithId &buf) override {
            InvokeJS(callbacks[2], buf.id);
        }

        void InvokeJS(v8::Global<v8::Function>& func, int32_t id) const;

        int32_t                     listener_id;
        v8::Isolate                *isolate;

        // [0] playing, [1] cancelled, [2] consumed
        v8::Global<v8::Function>    callbacks[3];
    };

    std::unique_ptr<utau::AudioSink> audio_sink_;
    std::list<JSBufferEventListener> js_buffer_listeners_;
};

//! TSDecl: class AudioBuffer
class AudioBufferWrap
{
public:
    explicit AudioBufferWrap(std::shared_ptr<utau::AudioBuffer> buffer)
        : buffer_(std::move(buffer)) {}
    ~AudioBufferWrap() = default;

    g_nodiscard g_inline std::shared_ptr<utau::AudioBuffer> GetBuffer() {
        return buffer_;
    }

    //! TSDecl: readonly sampleFormat: Enum<SampleFormat>
    int32_t getSampleFormat();

    //! TSDecl: readonly channelMode: Enum<ChannelMode>
    int32_t getChannelMode();

    //! TSDecl: readonly sampleRate: number
    int32_t getSampleRate();

    //! TSDecl: readonly samplesCount: number
    int32_t getSamplesCount();

private:
    std::shared_ptr<utau::AudioBuffer>    buffer_;
};

//! TSDecl: class AudioFilterDAG
class AudioFilterDAGWrap
{
public:
    explicit AudioFilterDAGWrap(std::unique_ptr<utau::AudioFilterDAG> DAG)
        : DAG_(std::move(DAG)) {}
    ~AudioFilterDAGWrap() = default;

    //! TSDecl:
    //! interface InBufferParameters {
    //!   name: string;
    //!   sampleFormat: Enum<SampleFormat>;
    //!   channelMode: Enum<ChannelMode>;
    //!   sampleRate: number;
    //! }

    //! TSDecl:
    //! interface OutBufferParameters {
    //!   name: string;
    //!   sampleFormats?: Array<Enum<SampleFormat>>;
    //!   channelModes?: Array<Enum<ChannelMode>>;
    //!   sampleRates?: Array<number>;
    //! }

    //! TSDecl: function MakeFromDSL(dsl: string,
    //!                              inparams: Array<InBufferParameters>,
    //!                              outparams: Array<OutBufferParameters>): AudioFilterDAG
    static v8::Local<v8::Value> MakeFromDSL(const std::string& dsl,
                                            v8::Local<v8::Value> inparams,
                                            v8::Local<v8::Value> outparams);

    //! TSDecl: function filter(inBuffers: Array<DAGNamedInOutBuffer>): Array<DAGNamedInOutBuffer>
    v8::Local<v8::Value> filter(v8::Local<v8::Value> inbuffers);

private:
    std::unique_ptr<utau::AudioFilterDAG> DAG_;
};

//! TSDecl: class AVStreamDecoder
class AVStreamDecoderWrap
{
public:
    explicit AVStreamDecoderWrap(std::unique_ptr<utau::AVStreamDecoder> decoder)
        : decoder_(std::move(decoder)) {}
    ~AVStreamDecoderWrap() = default;

    //! TSDecl:
    //! interface DecoderOptions {
    //!   disableAudio?: boolean;
    //!   disableVideo?: boolean;
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

private:
    std::unique_ptr<utau::AVStreamDecoder> decoder_;
};

GALLIUM_BINDINGS_UTAU_NS_END
#endif //COCOA_GALLIUM_BINDINGS_UTAU_EXPORTS_H
