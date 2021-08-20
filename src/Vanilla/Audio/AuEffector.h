#ifndef COCOA_AUEFFECTOR_H
#define COCOA_AUEFFECTOR_H

#include "Vanilla/Audio/AuCommon.h"

struct AVFilter;
struct AVFilterContext;

VANILLA_NS_BEGIN

#define VAAU_INFINITE_CONNECTOR     static_cast<int>(-1)

class AuEffector
{
public:
    enum class Kind
    {
        kBufferSrc,
        kBufferSink,
        kMixer,
        kResample
    };

    AuEffector(Kind kind, int inputConnectors, int outputConnectors);
    AuEffector(const AuEffector& lhs);
    virtual ~AuEffector() = default;

    va_nodiscard inline int inputConnectors() const
    { return fInputConnectors; }
    va_nodiscard inline int outputConnectors() const
    { return fOutputConnectors; }
    va_nodiscard inline Kind kind() const
    { return fKind; }

    ::AVFilterContext *_getFilter() const;

private:
    Kind        fKind;
    int         fInputConnectors;
    int         fOutputConnectors;
};

VANILLA_NS_END
#endif //COCOA_AUEFFECTOR_H
