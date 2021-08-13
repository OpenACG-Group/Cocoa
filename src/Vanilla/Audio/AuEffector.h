#ifndef COCOA_AUEFFECTOR_H
#define COCOA_AUEFFECTOR_H

#include "Vanilla/Audio/AuCommon.h"
VANILLA_NS_BEGIN

#define VAAU_INFINITE_CONNECTOR     static_cast<int>(-1)

class AuEffector
{
public:
    enum class Kind
    {
        kBufferSrc,
        kBufferSink
        kMixer,
        kResample
    };

    AuEffector(Kind kind, int inputConnectors, int outputConnectors);
    virtual ~AuEffector() = default;

    static Handle<AuEffector> MakeBufferSrc();
    static Handle<AuEffector> MakeBufferSink();
    static Handle<AuEffector> MakeMixer();
    static Handle<AuEffector> MakeResampler();

    static bool Link(const Handle<AuEffector>& src, int srcOutConnector,
                     const Handle<AuEffector>& dst, int dstInConnector);

    va_nodiscard inline int inputConnectors() const
    { return fInputConnectors; }
    va_nodiscard inline int outputConnectors() const
    { return fOutputConnectors; }
    va_nodiscard inline Kind kind() const
    { return fKind; }

private:
    Kind        fKind;
    int         fInputConnectors;
    int         fOutputConnectors;
};

VANILLA_NS_END
#endif //COCOA_AUEFFECTOR_H
