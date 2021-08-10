#include <thread>

#include <pulse/pulseaudio.h>

#include "Core/Journal.h"
#include "Vanilla/Base.h"
#include "Vanilla/Audio/PulseConnection.h"
VANILLA_NS_BEGIN

#define THIS_FILE_MODULE    COCOA_MODULE_NAME(Vanilla.Audio)

struct PulseConnection::Private
{
    ::pa_mainloop           *mainloop;
    ::pa_context            *context;
    ::pa_sample_spec         defaultSampleSpec;
    ::pa_stream             *stream;
};

Handle<PulseConnection> PulseConnection::Make()
{
    enum class ServerState
    {
        kPending,
        kReady,
        kFailed
    };

    ::pa_mainloop *mainloop;
    ::pa_mainloop_api *mainloopApi;
    ::pa_context *context;

    mainloop = ::pa_mainloop_new();
    mainloopApi = ::pa_mainloop_get_api(mainloop);
    context = ::pa_context_new(mainloopApi, COCOA_AUDIO_SERVICE_APPNAME);
    ::pa_context_connect(context, nullptr, PA_CONTEXT_NOFLAGS, nullptr);

    ServerState status = ServerState::kPending;
    ::pa_context_set_state_callback(context, [](::pa_context *ctx, void *closure) -> void {
        auto st = reinterpret_cast<ServerState*>(closure);
        switch (::pa_context_get_state(ctx))
        {
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;
        case PA_CONTEXT_READY:
            *st = ServerState::kReady;
            break;
        case PA_CONTEXT_FAILED:
        case PA_CONTEXT_TERMINATED:
            *st = ServerState::kFailed;
            break;
        }
    }, &status);

    while (status == ServerState::kPending)
        ::pa_mainloop_iterate(mainloop, 1, nullptr);

    if (status == ServerState::kFailed)
    {
        ::pa_context_disconnect(context);
        ::pa_context_unref(context);
        ::pa_mainloop_free(mainloop);
        LOGW(LOG_ERROR, "Could not connect to pulseaudio server")
        return nullptr;
    }

    ::pa_sample_spec sampleSpec;
    ::pa_operation *infoOperation = ::pa_context_get_server_info(context,
                                                                 [](::pa_context *ctx,
                                                                    const ::pa_server_info *info,
                                                                    void *closure) -> void {
        *reinterpret_cast<::pa_sample_spec*>(closure) = info->sample_spec;
        LOGF(LOG_INFO, "Connected to pulseaudio server \"{}\" version \"{}\"",
             info->server_name,
             info->server_version)
    }, &sampleSpec);

    while (::pa_operation_get_state(infoOperation) != PA_OPERATION_DONE)
        ::pa_mainloop_iterate(mainloop, 1, nullptr);

    /* Then we create a single stream for playback */
    ::pa_stream *stream = ::pa_stream_new(context, "Playback", &sampleSpec, nullptr);

    ::pa_buffer_attr attrs;
    std::memset(&attrs, 0, sizeof(attrs));
    /* Latency and process time features are disabled */
    attrs.fragsize = static_cast<uint32_t>(-1);
    attrs.maxlength = static_cast<uint32_t>(-1);
    attrs.prebuf = static_cast<uint32_t>(-1);
    attrs.tlength = static_cast<uint32_t>(-1);
    attrs.minreq = static_cast<uint32_t>(-1);

    int ret = ::pa_stream_connect_playback(stream,
                                           nullptr,
                                           &attrs,
                                           static_cast<::pa_stream_flags_t>(
                                                   PA_STREAM_INTERPOLATE_TIMING |
                                                   PA_STREAM_AUTO_TIMING_UPDATE),
                                           nullptr,
                                           nullptr);
    if (ret < 0)
    {
        ::pa_context_disconnect(context);
        ::pa_context_unref(context);
        ::pa_mainloop_free(mainloop);
        LOGF(LOG_ERROR, "Failed to create an audio stream: {}",
             ::pa_strerror(::pa_context_errno(context)))
        return nullptr;
    }

    auto data = new Private{
        mainloop,
        context,
        sampleSpec,
        stream
    };
    return std::make_shared<PulseConnection>(data);
}

PulseConnection::PulseConnection(Private *pData)
    : fData(pData),
      fPaused(false),
      fThread(&PulseConnection::routine, this)
{
}

PulseConnection::~PulseConnection()
{
    ::pa_mainloop_quit(fData->mainloop, 0);

    if (fThread.joinable())
        fThread.join();

    ::pa_stream_disconnect(fData->stream);
    ::pa_stream_unref(fData->stream);
    ::pa_context_disconnect(fData->context);
    ::pa_context_unref(fData->context);
    ::pa_mainloop_free(fData->mainloop);
    delete fData;
}

void PulseConnection::pause()
{
    if (fPaused)
        return;
    fPaused = true;
    ::pa_mainloop_wakeup(fData->mainloop);
}

void PulseConnection::resume()
{
    if (!fPaused)
        return;
    fPaused = false;
    ::pa_mainloop_wakeup(fData->mainloop);
}

uint32_t PulseConnection::sampleRate()
{
    return fData->defaultSampleSpec.rate;
}

int PulseConnection::channels()
{
    return fData->defaultSampleSpec.channels;
}

PaSampleFormat PulseConnection::format()
{
    switch (fData->defaultSampleSpec.format)
    {
    case PA_SAMPLE_U8:          return PaSampleFormat::kU8;
    case PA_SAMPLE_S16LE:       return PaSampleFormat::kS16_LE;
    case PA_SAMPLE_S16BE:       return PaSampleFormat::kS16_BE;
    case PA_SAMPLE_S24LE:       return PaSampleFormat::kS24_LE;
    case PA_SAMPLE_S24BE:       return PaSampleFormat::kS24_BE;
    case PA_SAMPLE_S32LE:       return PaSampleFormat::kS32_LE;
    case PA_SAMPLE_S32BE:       return PaSampleFormat::kS32_BE;
    case PA_SAMPLE_S24_32LE:    return PaSampleFormat::kS24_32_LE;
    case PA_SAMPLE_S24_32BE:    return PaSampleFormat::kS24_32_BE;
    case PA_SAMPLE_ALAW:        return PaSampleFormat::kAlaw;
    case PA_SAMPLE_ULAW:        return PaSampleFormat::kUlaw;
    case PA_SAMPLE_FLOAT32LE:   return PaSampleFormat::kFloat32_LE;
    case PA_SAMPLE_FLOAT32BE:   return PaSampleFormat::kFloat32_BE;
    default:
        throw RuntimeException(__func__, "Unknown audio format");
    }
}

namespace {

void streamWriteCallback(::pa_stream *stream, size_t length, void *closure)
{
    auto self = reinterpret_cast<PulseConnection::Private*>(closure);
    fmt::print("write: length={}\n", length);
}

void streamBufferUnderflow(::pa_stream *stream, void *closure)
{
    fmt::print("underflow\n");
}

} // namespace anonymous

void PulseConnection::routine()
{
#ifdef __linux__
    ::pthread_setname_np(::pthread_self(), COCOA_AUDIO_THREAD_NAME);
#endif // __linux__

    ::pa_stream_set_write_callback(fData->stream, streamWriteCallback, fData);
    ::pa_stream_set_underflow_callback(fData->stream, streamBufferUnderflow, fData);

    while (true)
    {
        bool savedPaused = fPaused;
        int ret = ::pa_mainloop_iterate(fData->mainloop, 1, nullptr);
        if (ret < 0)
            break;

        bool paused = fPaused;
        if (paused != savedPaused)
        {
            auto op = ::pa_stream_cork(fData->stream, paused, nullptr, nullptr);
            ::pa_operation_unref(op);
        }
    }
    LOGW(LOG_INFO, "Pulseaudio thread exited normally")
}

VANILLA_NS_END
