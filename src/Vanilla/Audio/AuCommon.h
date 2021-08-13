#ifndef COCOA_AUCOMMON_H
#define COCOA_AUCOMMON_H

#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

struct AuSampleSpec
{
    enum class Format
    {
        kPCM,
        kALAW,
        kULAW
    };

    /**
     * @param rate      Samples per second.
     * @param bpp       Bits per sample (8, 16 or 32).
     * @param channels  Guess what?
     * @param f         Format specifier.
     */
    AuSampleSpec(int rate, int bpp, int channels, Format f = Format::kPCM)
        : samplesPerSec(rate),
          bitsPerSample(bpp),
          channels(channels),
          format(f) {}

    int     samplesPerSec;
    int     bitsPerSample;
    int     channels;
    Format  format;
};

VANILLA_NS_END
#endif //COCOA_AUCOMMON_H
