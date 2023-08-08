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

//! TSDecl:
//! interface Rational {
//!   num: number;
//!   denom: number;
//! }

v8::Local<v8::Object> MakeRational(v8::Isolate *i, int32_t num, int32_t denom);
utau::Ratio ExtractRational(v8::Isolate *i, v8::Local<v8::Value> v);

//! TSDecl: function getCurrentTimestampMs(): number
uint64_t getCurrentTimestampMs();

//! TSDecl:
//! interface PixelFormatDescriptor {
//!   name: string;
//!   components: Array<{
//!     plane: number;
//!     step: number;
//!     offset: number;
//!     shift: number;
//!     depth: number;
//!   }>;
//!   hasPalette: boolean;
//!   isHWAccel: boolean;
//!   isPlanar: boolean;
//!   isRGBLike: boolean;
//!   isBayer: boolean;
//!   hasAlpha: boolean;
//!   isFloat: boolean;
//!   planes: number;
//!   bitsPerPixel: number;
//!   chromaWidthRShift: number;
//!   chromaHeightRShift: number;
//! }

//! TSDecl: function getPixelFormatDescriptor(fmt: Enum<PixelFormat>): PixelFormatDescriptor
v8::Local<v8::Value> getPixelFormatDescriptor(int32_t fmt);

//! TSDecl: class AudioDevice
class AudioDeviceWrap : public ExportableObjectBase
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
class AudioSinkStreamWrap : public ExportableObjectBase
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
class AudioBufferWrap : public ExportableObjectBase
{
public:
    explicit AudioBufferWrap(std::shared_ptr<utau::AudioBuffer> buffer);
    ~AudioBufferWrap();

    g_nodiscard g_inline std::shared_ptr<utau::AudioBuffer> GetBuffer() {
        return buffer_;
    }

    //! TSDecl: readonly pts: number
    v8::Local<v8::Value> getPTS() {
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        return v8::BigInt::New(isolate, buffer_->GetPresentationTimestamp());
    }

    //! TSDecl: readonly duration: number
    v8::Local<v8::Value> getDuration() {
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        return v8::BigInt::New(isolate, buffer_->GetDuration());
    }

    //! TSDecl: readonly sampleFormat: Enum<SampleFormat>
    int32_t getSampleFormat() {
        return static_cast<int32_t>(buffer_->GetInfo().GetSampleFormat());
    }

    //! TSDecl: readonly channelMode: Enum<ChannelMode>
    int32_t getChannelMode() {
        return static_cast<int32_t>(buffer_->GetInfo().GetChannelMode());
    }

    //! TSDecl: readonly sampleRate: number
    int32_t getSampleRate() {
        return buffer_->GetInfo().GetSampleRate();
    }

    //! TSDecl: readonly samplesCount: number
    int32_t getSamplesCount() {
        return buffer_->GetInfo().GetSamplesCount();
    }

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

    //! TSDecl: function clone(): AudioBuffer
    v8::Local<v8::Value> clone();

private:
    size_t approximate_size_;
    std::shared_ptr<utau::AudioBuffer>    buffer_;
};

//! TSDecl: class AudioFilterDAG
class AVFilterDAGWrap : public ExportableObjectBase
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
    //!   hwFramesContext?: HWFramesContextRef;
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

    //! TSDecl: function sendFrame(name: string, frame: AudioBuffer | VideoBuffer): void
    void sendFrame(const std::string& name, v8::Local<v8::Value> frame);

    //! TSDecl:
    //! interface DAGOutputInfo {
    //!   status: Enum<DAGReceiveStatus>;
    //!   name?: string;
    //!   mediaType?: Enum<MediaType>;
    //!   audio?: AudioBuffer;
    //!   video?: VideoBuffer;
    //! }

    //! TSDecl: function tryReceiveFrame(name: string): DAGOutputInfo
    v8::Local<v8::Value> tryReceiveFrame(const std::string& name);

private:
    std::unique_ptr<utau::AVFilterDAG> DAG_;
};

enum class ComponentSelector
{
    kLuma,
    kChromaU,
    kChromaV,
    kR,
    kG,
    kB,
    kAlpha
};

//! TSDecl: class VideoBuffer
class VideoBufferWrap : public ExportableObjectBase
{
public:
    explicit VideoBufferWrap(std::shared_ptr<utau::VideoBuffer> buffer);
    ~VideoBufferWrap();

    g_nodiscard g_inline std::shared_ptr<utau::VideoBuffer> GetBuffer() const {
        return buffer_;
    }

    //! TSDecl: function dispose(): void
    void dispose();

    //! TSDecl: function clone(): VideoBuffer
    v8::Local<v8::Value> clone();

    //! TSDecl: readonly disposed: boolean
    bool getDisposed() {
        return !buffer_;
    }

    //! TSDecl: readonly pts: number
    v8::Local<v8::Value> getPTS() {
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        return v8::BigInt::New(isolate, buffer_->GetPresentationTimestamp());
    }

    //! TSDecl: readonly duration: number
    v8::Local<v8::Value> getDuration() {
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        return v8::BigInt::New(isolate, buffer_->GetDuration());
    }

    //! TSDecl: readonly width: number
    int32_t getWidth() {
        return buffer_->GetInfo().GetWidth();
    }

    //! TSDecl: readonly height: number
    int32_t getHeight() {
        return buffer_->GetInfo().GetHeight();
    }

    //! TSDecl: readonly hwframe: boolean
    bool getHwframe() {
        return buffer_->GetInfo().GetColorInfo().FormatIsHWAccel();
    }

    //! TSDecl: readonly frameType: Enum<VideoFrameType>
    int32_t getFrameType() {
        return static_cast<int32_t>(buffer_->GetInfo().GetFrameType());
    }

    //! TSDecl: readonly format: Enum<PixelFormat>
    int32_t getFormat() {
        return buffer_->GetInfo().GetColorInfo().GetFormat();
    }

    //! TSDecl: readonly formatName: string
    const char *getFormatName() {
        return buffer_->GetInfo().GetColorInfo().GetFormatName();
    }

    //! TSDecl: readonly strides: Array<number>
    v8::Local<v8::Value> getStrides();

    //! TSDecl: function readComponent(component: Enum<ComponentSelector>,
    //!                                dst: TypedArray,
    //!                                sliceW: number,
    //!                                sliceH: number,
    //!                                srcX: number,
    //!                                srcY: number,
    //!                                dstStrideInElements: number): void
    void readComponent(int32_t component,
                       v8::Local<v8::Value> dst,
                       int32_t slice_w,
                       int32_t slice_h,
                       int32_t src_x,
                       int32_t src_y,
                       int32_t dst_stride_in_elements);

    //! TSDecl: function readComponentAsync(component: Enum<ComponentSelector>,
    //!                                     dst: TypedArray,
    //!                                     sliceW: number,
    //!                                     sliceH: number,
    //!                                     srcX: number,
    //!                                     srcY: number,
    //!                                     dstStrideInElements: number): Promise<void>
    v8::Local<v8::Value> readComponentAsync(int32_t component,
                                            v8::Local<v8::Value> dst,
                                            int32_t slice_w,
                                            int32_t slice_h,
                                            int32_t src_x,
                                            int32_t src_y,
                                            int32_t dst_stride_in_elements);

    //! TSDecl: function readGrayscale(dst: Uint8Array,
    //!                                sliceW: number,
    //!                                sliceH: number,
    //!                                srcX: number,
    //!                                srcY: number,
    //!                                dstStride: number): void
    void readGrayscale(v8::Local<v8::Value> dst,
                       int32_t slice_w,
                       int32_t slice_h,
                       int32_t src_x,
                       int32_t src_y,
                       int32_t dst_stride);

    //! TSDecl: function transferHardwareFrameDataTo(expectFormat: Enum<PixelFormat>): VideoBuffer
    v8::Local<v8::Value> transferHardwareFrameDataTo(int32_t expect_format);

    //! TSDecl: function queryHardwareTransferableFormats(): Array<Enum<PixelFormat>>
    v8::Local<v8::Value> queryHardwareTransferableFormats();

private:
    size_t approximate_size_;
    std::shared_ptr<utau::VideoBuffer> buffer_;
};

//! TSDecl: class HWFramesContextRef
class HWFramesContextRef : public ExportableObjectBase
{
public:
    explicit HWFramesContextRef(AVBufferRef *ref);
    ~HWFramesContextRef();

    g_nodiscard AVBufferRef *Get() const {
        return ref_;
    }

    //! TSDecl: function clone(): HWFramesContextRef
    v8::Local<v8::Value> clone();

    //! TSDecl: function dispose(): void
    void dispose();
    
private:
    AVBufferRef *ref_;
};

//! TSDecl: class AVStreamDecoder
class AVStreamDecoderWrap : public ExportableObjectBase
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

    //! TSDecl: function refHWFramesContext(): HWFramesContextRef | null
    v8::Local<v8::Value> refHWFramesContext();

private:
    std::unique_ptr<utau::AVStreamDecoder> decoder_;
};

GALLIUM_BINDINGS_UTAU_NS_END
#endif //COCOA_GALLIUM_BINDINGS_UTAU_EXPORTS_H
