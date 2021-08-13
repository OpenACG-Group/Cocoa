#ifndef COCOA_PACONNECTION_H
#define COCOA_PACONNECTION_H

#include "Vanilla/Base.h"
#include "Vanilla/Audio/AuCommon.h"
VANILLA_NS_BEGIN

/* Vanilla Audio Subsystem: pulseaudio backend */
class PaConnection
{
public:
    struct PrivateField;

    explicit PaConnection(UniqueHandle<PrivateField>&& data);
    PaConnection(const PaConnection&) = delete;
    PaConnection& operator=(const PaConnection&) = delete;
    ~PaConnection();

    static Handle<PaConnection> Make();

    void close();
    va_nodiscard bool isOpen() const;
    bool open(const AuSampleSpec& spec, uint32_t latency);
    uint32_t play(const uint8_t *data, size_t size);

private:
    UniqueHandle<PrivateField>      fData;
};

VANILLA_NS_END
#endif //COCOA_PACONNECTION_H
