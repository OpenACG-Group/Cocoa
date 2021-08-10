#ifndef COCOA_AUBUFFER_H
#define COCOA_AUBUFFER_H

#include <future>

#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class AuBuffer
{
public:
    enum class State
    {
        kReady      = 0,
        kQueued     = 1,
        kPlaying    = 2,
        kFinish     = 3
    };

    explicit AuBuffer(size_t size);
    AuBuffer(const AuBuffer&) = delete;
    AuBuffer operator=(const AuBuffer&) = delete;
    ~AuBuffer();

    va_nodiscard inline uint8_t *data()
    { return fData; }
    va_nodiscard inline size_t size() const
    { return fSize; }

    State getState();
    void waitUtil(State state);

    void _setPlaying();
    void _setFinish();

private:
    uint8_t                *fData;
    size_t                  fSize;
    std::promise<void>      fPlayingPromise;
    std::promise<void>      fFinishPromise;
};

VANILLA_NS_END
#endif //COCOA_AUBUFFER_H
