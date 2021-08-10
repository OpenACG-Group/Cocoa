#ifndef COCOA_PULSECONNECTION_H
#define COCOA_PULSECONNECTION_H

#include <thread>
#include <mutex>
#include <atomic>
#include <future>

#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

#define COCOA_AUDIO_SERVICE_APPNAME     "Cocoa Engine Audio Service"
#define COCOA_AUDIO_THREAD_NAME         "pulseaudio"

enum class PaSampleFormat
{
    kU8,
    kAlaw,
    kUlaw,
    kS16_LE,
    kS16_BE,
    kS32_LE,
    kS32_BE,
    kS24_LE,
    kS24_BE,
    kS24_32_LE,
    kS24_32_BE,
    kFloat32_LE,
    kFloat32_BE
};

class PulseConnection
{
public:
    struct Private;

    explicit PulseConnection(Private *pData);
    PulseConnection(const PulseConnection&) = delete;
    PulseConnection& operator=(const PulseConnection&) = delete;
    ~PulseConnection();

    static Handle<PulseConnection> Make();

    uint32_t sampleRate();
    PaSampleFormat format();
    int channels();

    void pause();
    void resume();

    std::future<bool> queueBuffer();

private:
    void routine();

    Private         *fData;
    std::atomic_bool fPaused;
    std::thread      fThread;
};

VANILLA_NS_END
#endif //COCOA_PULSECONNECTION_H
