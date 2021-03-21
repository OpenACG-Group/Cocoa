#ifndef COCOA_UNIXSIGNALCATCHER_H
#define COCOA_UNIXSIGNALCATCHER_H

#include "Core/BaseEventProcessor.h"
#include "Core/UniquePersistent.h"

namespace cocoa {

class UnixSignalCatcher : public BaseEventProcessor,
                          public UniquePersistent<UnixSignalCatcher>
{
public:
    UnixSignalCatcher();
    ~UnixSignalCatcher() override;

    void processEvent() override;
    void setCaughtSignal(int signum);

private:
    int     fCaughtSignal;
};

}

#endif //COCOA_UNIXSIGNALCATCHER_H
