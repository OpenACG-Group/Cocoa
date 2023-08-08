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

#ifndef COCOA_MEDIAFRAMEPRESENTDISPATCHER_H
#define COCOA_MEDIAFRAMEPRESENTDISPATCHER_H

#include <list>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "uv.h"

#include "Gallium/bindings/utau/Exports.h"
#include "Gallium/bindings/ExportableObjectBase.h"
GALLIUM_BINDINGS_UTAU_NS_BEGIN

//! TSDecl: class MediaFramePresentDispatcher
class MediaFramePresentDispatcher : public ExportableObjectBase,
                                    public MaybeGCRootObject<MediaFramePresentDispatcher>
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

    void PresentQueueFullHostCheckpoint();
    void PresentQueueFullPresentThreadCheckpoint();

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

    std::condition_variable         present_queue_full_cond_;
    bool                            present_queue_full_;
};

GALLIUM_BINDINGS_UTAU_NS_END
#endif //COCOA_MEDIAFRAMEPRESENTDISPATCHER_H
