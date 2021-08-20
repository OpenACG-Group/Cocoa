#include <pulse/pulseaudio.h>

#include "Core/Journal.h"
#include "Vanilla/Base.h"
#include "Vanilla/Audio/AuPulseaudioConnection.h"
VANILLA_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Vanilla.Audio)

struct AuPulseaudioConnection::PrivateField
{
    PrivateField()
        : mainloop(nullptr), context(nullptr), stream(nullptr), latency(0) {}
    ~PrivateField()
    {
        /* @a stream field should be managed by PaConnection class */
        if (mainloop)
            ::pa_threaded_mainloop_stop(mainloop);
        if (context)
        {
            ::pa_context_disconnect(context);
            ::pa_context_unref(context);
        }
        if (mainloop)
            ::pa_threaded_mainloop_free(mainloop);
    }
    ::pa_threaded_mainloop  *mainloop;
    ::pa_context            *context;
    ::pa_stream             *stream;
    uint32_t                 latency{};
};

namespace {

void context_state_callback(::pa_context *context, void *userdata)
{
    auto pd = reinterpret_cast<AuPulseaudioConnection::PrivateField*>(userdata);
    switch (::pa_context_get_state(context))
    {
    case PA_CONTEXT_READY:
    case PA_CONTEXT_FAILED:
    case PA_CONTEXT_TERMINATED:
        ::pa_threaded_mainloop_signal(pd->mainloop, 0);
        break;

    default:
        break;
    }
}

} // namespace anonymous

Handle<AuPulseaudioConnection> AuPulseaudioConnection::Make()
{
    UniqueHandle<PrivateField> pd = std::make_unique<PrivateField>();
    pd->mainloop = ::pa_threaded_mainloop_new();
    if (!pd->mainloop)
    {
        LOGW(LOG_ERROR, "Failed to create a mainloop for pulseaudio")
        return nullptr;
    }

    ::pa_mainloop_api *mapi = ::pa_threaded_mainloop_get_api(pd->mainloop);
    pd->context = ::pa_context_new(mapi, "Cocoa Engine");
    if (!pd->context)
    {
        LOGW(LOG_ERROR, "Failed to create a pulseaudio context")
        return nullptr;
    }

    ::pa_context_set_state_callback(pd->context, context_state_callback, pd.get());

    if (::pa_context_connect(pd->context, nullptr, PA_CONTEXT_NOFLAGS, nullptr))
    {
        LOGW(LOG_ERROR, "Failed to connect to pulseaudio server")
        return nullptr;
    }

    ::pa_threaded_mainloop_lock(pd->mainloop);
    if (::pa_threaded_mainloop_start(pd->mainloop) < 0)
    {
        LOGW(LOG_ERROR, "Failed to start threaded pulseaudio mainloop")
        ::pa_threaded_mainloop_unlock(pd->mainloop);
        return nullptr;
    }

    while (true)
    {
        auto state = ::pa_context_get_state(pd->context);
        if (state == PA_CONTEXT_READY || !PA_CONTEXT_IS_GOOD(state))
            break;
        ::pa_threaded_mainloop_wait(pd->mainloop);
    }

    // TODO: Volume control
    ::pa_threaded_mainloop_unlock(pd->mainloop);
    return std::make_shared<AuPulseaudioConnection>(std::move(pd));
}

AuPulseaudioConnection::AuPulseaudioConnection(UniqueHandle<PrivateField>&& data)
    : fData(std::move(data))
{
}

AuPulseaudioConnection::~AuPulseaudioConnection()
{
    this->close();
}

namespace {

void wait_for_operation(AuPulseaudioConnection::PrivateField *pd, ::pa_operation *op)
{
    if (!op)
        return;
    while (::pa_operation_get_state(op) == PA_OPERATION_RUNNING)
        ::pa_threaded_mainloop_wait(pd->mainloop);
    ::pa_operation_unref(op);
}

void stream_success_callback(::pa_stream *stream, int success, void *userdata)
{
    auto pd = reinterpret_cast<AuPulseaudioConnection::PrivateField*>(userdata);
    ::pa_threaded_mainloop_signal(pd->mainloop, 0);
}

bool is_valid_sample_spec(const AuSampleSpec& spec)
{
    switch (spec.format)
    {
    case AuSampleSpec::Format::kPCM:
        if (spec.samplesPerSec <= PA_RATE_MAX &&
            (spec.bitsPerSample == 8 || spec.bitsPerSample == 16) &&
            spec.channels >= 1 && spec.channels <= PA_CHANNELS_MAX)
        {
            return true;
        }
    case AuSampleSpec::Format::kALAW:
    case AuSampleSpec::Format::kULAW:
        if (spec.samplesPerSec <= PA_RATE_MAX &&
            spec.bitsPerSample == 8 &&
            spec.channels >= 1 && spec.channels <= PA_CHANNELS_MAX)
        {
            return true;
        }
    }
    return false;
}

bool get_pod_pa_sample_spec(const AuSampleSpec& spec, ::pa_sample_spec& out)
{
    if (!is_valid_sample_spec(spec))
        return false;
    out.rate = spec.samplesPerSec;
    out.channels = spec.channels;
    switch (spec.format)
    {
    case AuSampleSpec::Format::kPCM:
        if (spec.bitsPerSample == 8)
            out.format = PA_SAMPLE_U8;
        else if (spec.bitsPerSample == 16)
            out.format = PA_SAMPLE_S16LE;
        else
            return false;
        break;
    case AuSampleSpec::Format::kALAW:
        out.format = PA_SAMPLE_ALAW;
        break;
    case AuSampleSpec::Format::kULAW:
        out.format = PA_SAMPLE_ULAW;
        break;
    }
    return true;
}

void stream_state_callback(::pa_stream *stream, void *userdata)
{
    auto pd = reinterpret_cast<AuPulseaudioConnection::PrivateField*>(userdata);
    switch (::pa_stream_get_state(stream))
    {
    case PA_STREAM_READY:
    case PA_STREAM_FAILED:
    case PA_STREAM_TERMINATED:
        ::pa_threaded_mainloop_signal(pd->mainloop, 0);
        break;
    default:
        break;
    }
}

void stream_request_callback(::pa_stream *stream, size_t length, void *userdata)
{
    auto pd = reinterpret_cast<AuPulseaudioConnection::PrivateField*>(userdata);
    ::pa_threaded_mainloop_signal(pd->mainloop, 0);
}

template<typename T, typename R>
T force_pod_cast(const R& v)
{
    return *reinterpret_cast<const T*>(static_cast<const void*>(&v));
}

} // namespace anonymous

bool AuPulseaudioConnection::open(const AuSampleSpec& spec, uint32_t latency)
{
    ::pa_sample_spec podSpec;
    if (!get_pod_pa_sample_spec(spec, podSpec))
        return false;
    fData->latency = latency;

    if (!::pa_sample_spec_valid(&podSpec))
        return false;

    ::pa_threaded_mainloop_lock(fData->mainloop);
    fData->stream = ::pa_stream_new(fData->context, "Playback", &podSpec, nullptr);
    if (!fData->stream)
    {
        ::pa_threaded_mainloop_unlock(fData->mainloop);
        return false;
    }

    ::pa_stream_set_state_callback(fData->stream, stream_state_callback, fData.get());
    ::pa_stream_set_write_callback(fData->stream, stream_request_callback, fData.get());
    uint32_t flags = PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_AUTO_TIMING_UPDATE;

    ::pa_buffer_attr attrs{};
    if (latency > 0)
    {
        attrs.maxlength = ::pa_usec_to_bytes(latency * 2 * 1000, &podSpec);
        attrs.tlength = ::pa_usec_to_bytes(latency * 1000, &podSpec);
        attrs.prebuf = static_cast<uint32_t>(-1);
        attrs.minreq = static_cast<uint32_t>(-1);
        attrs.fragsize = static_cast<uint32_t>(-1);
        flags = PA_STREAM_ADJUST_LATENCY;
    }

    int ret = ::pa_stream_connect_playback(fData->stream,
                                           nullptr,
                                           latency > 0 ? &attrs : nullptr,
                                           (::pa_stream_flags_t)flags,
                                           nullptr,
                                           nullptr);
    if (ret < 0)
    {
        ::pa_threaded_mainloop_unlock(fData->mainloop);
        return true;
    }

    ::pa_stream_state_t state;
    while (true)
    {
        state = ::pa_stream_get_state(fData->stream);
        if (state == PA_STREAM_READY || !PA_STREAM_IS_GOOD(state))
            break;
        ::pa_threaded_mainloop_wait(fData->mainloop);
    }

    ::pa_threaded_mainloop_unlock(fData->mainloop);
    if (state == PA_STREAM_READY)
        return true;
    this->close();
    return false;
}

void AuPulseaudioConnection::close()
{
    if (!fData->stream || !fData->context)
        return;
    ::pa_threaded_mainloop_lock(fData->mainloop);
    wait_for_operation(fData.get(),
                       ::pa_stream_drain(fData->stream, stream_success_callback, fData.get()));
    ::pa_stream_disconnect(fData->stream);
    ::pa_stream_unref(fData->stream);
    fData->stream = nullptr;
    ::pa_threaded_mainloop_unlock(fData->mainloop);
}

bool AuPulseaudioConnection::isOpen() const
{
    return (fData->stream != nullptr);
}

uint32_t AuPulseaudioConnection::play(const uint8_t *data, size_t size)
{
    if (!fData->stream || !data || size == 0)
        return 0;

    ::pa_threaded_mainloop_lock(fData->mainloop);
    while (size > 0)
    {
        size_t length;
        while ((length = ::pa_stream_writable_size(fData->stream)) == 0)
            ::pa_threaded_mainloop_wait(fData->mainloop);
        if (length == static_cast<size_t>(-1))
            break;
        length = std::min(length, size);

        int ret = ::pa_stream_write(fData->stream, data, length, nullptr, 0LL, PA_SEEK_RELATIVE);
        if (ret < 0)
            break;
        data += length;
        size -= length;
    }

    ::pa_usec_t latency;
    int negative;
    if (::pa_stream_get_latency(fData->stream, &latency, &negative) != 0)
        latency = 0;
    ::pa_threaded_mainloop_unlock(fData->mainloop);
    return latency / 1000;
}

VANILLA_NS_END
