#ifndef COCOA_GSKENUMERATIONS_H
#define COCOA_GSKENUMERATIONS_H

#include "Gsk/Gsk.h"
GSK_NAMESPACE_BEGIN

enum class GskModifierType : uint32_t
{
    kShiftMask      = 1 << 0,
    kLockMask       = 1 << 1,
    kControlMask    = 1 << 2,
    kAltMask        = 1 << 3,
    kButton1Mask    = 1 << 8,
    kButton2Mask    = 1 << 9,
    kButton3Mask    = 1 << 10,
    kButton4Mask    = 1 << 11,
    kButton5Mask    = 1 << 12,
    kSuperMask      = 1 << 26,
    kHyperMask      = 1 << 27,
    kMetaMask       = 1 << 28
};

enum class GskEventMask : uint32_t
{
    kExposure       = 1 << 1,
    kPointerMotion  = 1 << 2,
    kButtonMotion   = 1 << 4,
    kButton1Motion  = 1 << 5,
    kButton2Motion  = 1 << 6,
    kButton3Motion  = 1 << 7,
    kButtonPress    = 1 << 8,
    kButtonRelease  = 1 << 9,
    kKeyPress       = 1 << 10,
    kKeyRelease     = 1 << 11,
    kEnterNotify    = 1 << 12,
    kLeaveNotify    = 1 << 13,
    kFocusChange    = 1 << 14,
    kStructure      = 1 << 15,
    kPropertyChange = 1 << 16,
    kProximityIn    = 1 << 18,
    kProximityOut   = 1 << 19,
    kScroll         = 1 << 20,
    kTouch          = 1 << 21,
    kSmoothScroll   = 1 << 22,
    kTouchpadGesture = 1 << 23,
    kTabletPad      = 1 << 24,
    kAllMask         = 0x3fffffe
};

enum class GskGrabStatus
{
    kSuccess,
    kAlreadyGrabbed,
    kInvalidTime,
    kNotViewable,
    kFrozen,
    kFailed
};


enum class GskAxisUse : uint32_t
{
    kIgnore = 0,
    kX,
    kY,
    kDeltaX,
    kDeltaY,
    kPressure,
    kXTilt,
    kYTilt,
    kWheel,
    /* Used for pen/tablet distance information */
    kDistance,
    /* Used for pen rotation information */
    kRotation,
    /* Used for pen slider information */
    kSlider,
    kLast
};


#define _BF(E)   E = (1 << static_cast<std::underlying_type<GskAxisUse>::type>(GskAxisUse::E))

enum class GskAxisFlags
{
    _BF(kX),
    _BF(kY),
    _BF(kDeltaX),
    _BF(kDeltaY),
    _BF(kPressure),
    _BF(kXTilt),
    _BF(kYTilt),
    _BF(kWheel),
    _BF(kDistance),
    _BF(kRotation),
    _BF(kSlider)
};

#undef _BF

enum class TextDirection
{
    kLTR,
    kRTL,
    kWeakLTR,
    kWeakRTL,
    kNeutral
};

enum class SurfaceType
{
    kToplevel,
    kTemp,
    kPopup
};

enum class SeatCapabilities : uint32_t
{
    kNone           = 0,
    kPointer        = 1 << 0,
    kTouch          = 1 << 1,
    kTabletStylus   = 1 << 2,
    kKeyboard       = 1 << 3,
    kTabletPad      = 1 << 4,
    kAllPointing    = kPointer | kTouch | kTabletStylus,
    kAll            = kAllPointing | kKeyboard
};

GSK_NAMESPACE_END
#endif //COCOA_GSKENUMERATIONS_H
