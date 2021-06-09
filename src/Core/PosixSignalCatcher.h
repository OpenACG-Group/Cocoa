#ifndef COCOA_POSIXSIGNALCATCHER_H
#define COCOA_POSIXSIGNALCATCHER_H

#include <csignal>

#include "Core/UniquePersistent.h"

namespace cocoa {

class PosixSignalCatcher : public UniquePersistent<PosixSignalCatcher>
{
public:
    PosixSignalCatcher();
    ~PosixSignalCatcher();

    void setCaughtSignal(int signum);

private:
    int                 fCaughtSignal;
    struct sigaction    fOldAction;
    stack_t             fOldSigStack;
    void               *fAlternateStack;
};

}

#endif //COCOA_POSIXSIGNALCATCHER_H
