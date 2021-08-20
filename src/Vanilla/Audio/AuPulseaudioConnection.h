#ifndef COCOA_AUPULSEAUDIOCONNECTION_H
#define COCOA_AUPULSEAUDIOCONNECTION_H

#include "Vanilla/Base.h"
#include "Vanilla/Audio/AuCommon.h"
VANILLA_NS_BEGIN

/* Vanilla Audio Subsystem: pulseaudio backend */
class AuPulseaudioConnection
{
public:
    struct PrivateField;

    explicit AuPulseaudioConnection(UniqueHandle<PrivateField>&& data);
    AuPulseaudioConnection(const AuPulseaudioConnection&) = delete;
    AuPulseaudioConnection& operator=(const AuPulseaudioConnection&) = delete;
    ~AuPulseaudioConnection();

    static Handle<AuPulseaudioConnection> Make();

    void close();
    va_nodiscard bool isOpen() const;
    bool open(const AuSampleSpec& spec, uint32_t latency);
    uint32_t play(const uint8_t *data, size_t size);

private:
    UniqueHandle<PrivateField>      fData;
};

VANILLA_NS_END
#endif //COCOA_AUPULSEAUDIOCONNECTION_H
