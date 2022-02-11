#include <cmath>
#include <utility>

#include "Core/Errors.h"
#include "Gsk/GskEvent.h"

GSK_NAMESPACE_BEGIN

namespace {

g_inline Maybe<std::tuple<double, double, double, double>> extract_components(const Handle<GskEvent>& e1,
                                                                              const Handle<GskEvent>& e2)
{
    auto p1 = e1->getPosition(), p2 = e2->getPosition();
    if (!p1 || !p2)
        return {};
    return std::make_tuple((*p1)[0], (*p1)[1], (*p2)[0], (*p2)[1]);
}

} // namespace anonymous

Maybe<double> GskEvent::Distance(const Handle<GskEvent>& e1, const Handle<GskEvent>& e2)
{
    if (auto pack = extract_components(e1, e2))
    {
        auto [x1, y1, x2, y2] = *pack;
        double dx = x2 - x1, dy = y2 - y1;
        return std::sqrt((dx * dx) + (dy * dy));
    }
    return {};
}

Maybe<double> GskEvent::Angle(const Handle<GskEvent>& e1, const Handle<GskEvent>& e2)
{
    if (auto pack = extract_components(e1, e2))
    {
        auto [x1, y1, x2, y2] = *pack;
        double r;

        r = std::atan2(x2 - x1, y2 - y1);   /* θ = atan(Δx / Δy)            */
        r = (2 * M_PI) - r;                 /* Invert angle: θ = 2π - θ     */
        r += M_PI / 2.0;                    /* Shift it π/2: θ = θ + π/2    */
        r = std::fmod(r, 2 * M_PI);         /* And constraint it to [0, 2π] */

        return r;
    }
    return {};
}

Maybe<Vec2d> GskEvent::Center(const Handle<GskEvent>& e1, const Handle<GskEvent>& e2)
{
    if (auto pack = extract_components(e1, e2))
    {
        auto [x1, y1, x2, y2] = *pack;
        return Vec2d((x1 + x2) / 2.0, (y1 + y2) / 2.0);
    }
    return {};
}

GskEvent::GskEvent(GskEventType type, Handle<GskSurface> surface,
                   Handle<GskDevice> device, uint32_t time)
    : fType(type)
    , fSurface(std::move(surface))
    , fDevice(std::move(device))
    , fTime(time)
    , fFlags(0)
{
}

uint8_t& GskEvent::getFlagsRef()
{
    return fFlags;
}

Maybe<double> GskEvent::getAxis(GskAxisUse use)
{
    if (use == GskAxisUse::kX || use == GskAxisUse::kY)
    {
        auto pos = this->getPosition();
        if (!pos)
            return {};
        if (use == GskAxisUse::kX)
            return pos->operator[](0);
        else
            return pos->operator[](1);
    }
    auto axes = this->getAxes();
    if (axes.size() < static_cast<uint32_t>(use))
        return false;
    return axes[static_cast<uint32_t>(use)];
}

bool GskEvent::getPointerEmulated()
{
    return false;
}

Maybe<Vec2d> GskEvent::getPosition()
{
    return {};
}

Handle<GskEventSequence> GskEvent::getSequence()
{
    return nullptr;
}

Bitfield<GskModifierType> GskEvent::getState()
{
    return {};
}

namespace {
GskEvent::AxesVec empty_axes_vec_{}; // NOLINT
}

const GskEvent::AxesVec& GskEvent::getAxes()
{
    return empty_axes_vec_;
}

GskEventType GskEvent::getEventType() const
{
    return fType;
}

Handle<GskSurface> GskEvent::getSurface() const
{
    return fSurface;
}

Handle<GskDevice> GskEvent::getDevice() const
{
    return fDevice;
}

uint32_t GskEvent::getTime() const
{
    return fTime;
}

GskButtonEvent::GskButtonEvent(GskEventType type,
                               Handle<GskSurface> surface,
                               Handle<GskDevice> device,
                               uint32_t time,
                               Bitfield<GskModifierType> state,
                               uint32_t button,
                               Vec2d pos,
                               AxesVec axes)
    : GskEvent(type, std::move(surface), std::move(device), time)
    , fState(state)
    , fButton(button)
    , fPosition(pos)
    , fAxes(std::move(axes))
{
    CHECK(type == GskEventType::kButtonPress ||
          type == GskEventType::kButtonRelease);
}

Maybe<Vec2d> GskButtonEvent::getPosition()
{
    return fPosition;
}

Bitfield<GskModifierType> GskButtonEvent::getState()
{
    return fState;
}

const GskButtonEvent::AxesVec& GskButtonEvent::getAxes()
{
    return fAxes;
}

uint32_t GskButtonEvent::getButton() const
{
    return fButton;
}

GskMotionEvent::GskMotionEvent(Handle<GskSurface> surface,
                               Handle<GskDevice> device,
                               uint32_t time,
                               Bitfield<GskModifierType> state,
                               Vec2d pos,
                               AxesVec axes)
    : GskEvent(GskEventType::kMotionNotify, std::move(surface), std::move(device), time)
    , fState(state)
    , fPosition(pos)
    , fAxes(std::move(axes))
{
}

Bitfield<GskModifierType> GskMotionEvent::getState()
{
    return fState;
}

Maybe<Vec2d> GskMotionEvent::getPosition()
{
    return fPosition;
}

const GskMotionEvent::AxesVec& GskMotionEvent::getAxes()
{
    return fAxes;
}

GskCrossingEvent::GskCrossingEvent(GskEventType type,
                                   Handle<GskSurface> surface,
                                   Handle<GskDevice> device,
                                   uint32_t time,
                                   Bitfield<GskModifierType> state,
                                   Vec2d pos,
                                   GskCrossingMode mode,
                                   GskNotifyType notify)
    : GskEvent(type, std::move(surface), std::move(device), time)
    , fState(state)
    , fMode(mode)
    , fPosition(pos)
    , fDetail(notify)
    , fFocus(false)
{
    CHECK(type == GskEventType::kEnterNotify ||
          type == GskEventType::kLeaveNotify);
}

Bitfield<GskModifierType> GskCrossingEvent::getState()
{
    return fState;
}

Maybe<Vec2d> GskCrossingEvent::getPosition()
{
    return fPosition;
}

GskCrossingMode GskCrossingEvent::getMode() const
{
    return fMode;
}

GskNotifyType GskCrossingEvent::getDetail() const
{
    return fDetail;
}

bool GskCrossingEvent::isFocused() const
{
    return fFocus;
}

GskProximityEvent::GskProximityEvent(GskEventType type,
                                     Handle<GskSurface> surface,
                                     Handle<GskDevice> device,
                                     uint32_t time)
    : GskEvent(type, std::move(surface), std::move(device), time)
{
    CHECK(type == GskEventType::kProximityIn ||
          type == GskEventType::kProximityOut);
}

GskKeyEvent::GskKeyEvent(GskEventType type,
                         Handle<GskSurface> surface,
                         Handle<GskDevice> device,
                         uint32_t time,
                         uint32_t keycode,
                         Bitfield<GskModifierType> modifiers,
                         bool isModifier,
                         const Translated& translated,
                         const Translated& noLock)
    : GskEvent(type, std::move(surface), std::move(device), time)
    , fState(modifiers)
    , fKeycode(keycode)
    , fKeyIsModifier(isModifier)
    , fTranslated{translated, noLock}
{
    CHECK(type == GskEventType::kKeyPress ||
          type == GskEventType::kKeyRelease);
}

Bitfield<GskModifierType> GskKeyEvent::getState()
{
    return fState;
}

uint32_t GskKeyEvent::getKeyval() const
{
    return fTranslated[0].keyval;
}

uint32_t GskKeyEvent::getKeycode() const
{
    return fKeycode;
}

uint32_t GskKeyEvent::getLevel() const
{
    return fTranslated[0].level;
}

uint32_t GskKeyEvent::getLayout() const
{
    return fTranslated[0].layout;
}

Bitfield<GskModifierType> GskKeyEvent::getConsumedModifiers() const
{
    return fTranslated[0].consumed;
}

bool GskKeyEvent::isModifier() const
{
    return fKeyIsModifier;
}

GskFocusEvent::GskFocusEvent(Handle<GskSurface> surface,
                             Handle<GskDevice> device,
                             bool focusIn)
    : GskEvent(GskEventType::kFocusChange, std::move(surface), std::move(device), 0)
    , fFocusIn(focusIn)
{
}

bool GskFocusEvent::isFocusIn() const
{
    return fFocusIn;
}

GskDeleteEvent::GskDeleteEvent(Handle<GskSurface> surface)
    : GskEvent(GskEventType::kDelete, std::move(surface), nullptr, 0)
{
}

GskScrollEvent::GskScrollEvent(Handle<GskSurface> surface,
                               Handle<GskDevice> device,
                               uint32_t time,
                               Bitfield<GskModifierType> state,
                               Vec2d deltas,
                               bool isStop)
    : GskEvent(GskEventType::kScroll, std::move(surface), std::move(device), time)
    , fState(state)
    , fDirection(GskScrollDirection::kSmooth)
    , fDeltas(deltas)
    , fPointerEmulated(false)
    , fIsStop(isStop)
{
}

GskScrollEvent::GskScrollEvent(Handle<GskSurface> surface,
                               Handle<GskDevice> device,
                               uint32_t time,
                               Bitfield<GskModifierType> state,
                               GskScrollDirection direction,
                               bool emulated)
    : GskEvent(GskEventType::kScroll, std::move(surface), std::move(device), time)
    , fState(state)
    , fDirection(direction)
    , fDeltas()
    , fPointerEmulated(emulated)
    , fIsStop(false)
{
}

Bitfield<GskModifierType> GskScrollEvent::getState()
{
    return fState;
}

bool GskScrollEvent::getPointerEmulated()
{
    return fPointerEmulated;
}

GskScrollDirection GskScrollEvent::getDirection() const
{
    return fDirection;
}

Vec2d GskScrollEvent::getDeltas() const
{
    return fDeltas;
}

bool GskScrollEvent::isStop() const
{
    return fIsStop;
}

GskTouchEvent::GskTouchEvent(GskEventType type,
                             Handle<GskEventSequence> sequence,
                             Handle<GskSurface> surface,
                             Handle<GskDevice> device,
                             uint32_t time,
                             Bitfield<GskModifierType> state,
                             Vec2d pos,
                             AxesVec axes,
                             bool emulating)
    : GskEvent(type, std::move(surface), std::move(device), time)
    , fState(state)
    , fPosition(pos)
    , fAxes(std::move(axes))
    , fSequence(std::move(sequence))
    , fPointerEmulating(emulating)
{
    CHECK(type == GskEventType::kTouchBegin ||
          type == GskEventType::kTouchEnd ||
          type == GskEventType::kTouchUpdate ||
          type == GskEventType::kTouchCancel);
}

Bitfield<GskModifierType> GskTouchEvent::getState()
{
    return fState;
}

Maybe<Vec2d> GskTouchEvent::getPosition()
{
    return fPosition;
}

const GskTouchEvent::AxesVec& GskTouchEvent::getAxes()
{
    return fAxes;
}

Handle<GskEventSequence> GskTouchEvent::getSequence()
{
    return fSequence;
}

bool GskTouchEvent::getPointerEmulated()
{
    return fPointerEmulating;
}

GskTouchpadEvent::GskTouchpadEvent(Handle<GskSurface> surface,
                                   Handle<GskEventSequence> sequence,
                                   Handle<GskDevice> device,
                                   uint32_t time,
                                   Bitfield<GskModifierType> state,
                                   GskTouchpadGesturePhase phase,
                                   Vec2d pos,
                                   int fingers,
                                   Vec2d deltas)
    : GskEvent(GskEventType::kTouchpadSwipe, std::move(surface), std::move(device), time)
    , fSequence(std::move(sequence))
    , fState(state)
    , fPhase(phase)
    , fFingers(fingers)
    , fPosition(pos)
    , fDeltas(deltas)
    , fAngleDelta(0)
    , fScale(1)
{
}

GskTouchpadEvent::GskTouchpadEvent(Handle<GskSurface> surface,
                                   Handle<GskEventSequence> sequence,
                                   Handle<GskDevice> device,
                                   uint32_t time,
                                   Bitfield<GskModifierType> state,
                                   GskTouchpadGesturePhase phase,
                                   Vec2d pos,
                                   int fingers,
                                   Vec2d deltas,
                                   double scale,
                                   double angleDelta)
    : GskEvent(GskEventType::kTouchpadPinch, std::move(surface), std::move(device), time)
    , fSequence(std::move(sequence))
    , fState(state)
    , fPhase(phase)
    , fFingers(fingers)
    , fPosition(pos)
    , fDeltas(deltas)
    , fAngleDelta(angleDelta)
    , fScale(scale)
{
}

Handle<GskEventSequence> GskTouchpadEvent::getSequence()
{
    return fSequence;
}

Bitfield<GskModifierType> GskTouchpadEvent::getState()
{
    return fState;
}

Maybe<Vec2d> GskTouchpadEvent::getPosition()
{
    return fPosition;
}

GskTouchpadGesturePhase GskTouchpadEvent::getGesturePhase() const
{
    return fPhase;
}

int GskTouchpadEvent::getFingers() const
{
    return fFingers;
}

Vec2d GskTouchpadEvent::getDeltas() const
{
    return fDeltas;
}

double GskTouchpadEvent::getPinchAngleDelta() const
{
    return fAngleDelta;
}

double GskTouchpadEvent::getPinchScale() const
{
    return fScale;
}

GskPadEvent::GskPadEvent(GskEventType type,
                         Handle<GskSurface> surface,
                         Handle<GskDevice> device,
                         uint32_t time,
                         uint32_t group,
                         uint32_t index,
                         uint32_t mode,
                         double value)
    : GskEvent(type, std::move(surface), std::move(device), time)
    , fGroup(group)
    , fMode(mode)
    , fButton(0)
    , fIndex(index)
    , fValue(value)
{
    CHECK(type == GskEventType::kPadRing ||
          type == GskEventType::kPadStrip);
}

GskPadEvent::GskPadEvent(GskEventType type,
                         Handle<GskSurface> surface,
                         Handle<GskDevice> device,
                         uint32_t time,
                         uint32_t group,
                         uint32_t button,
                         uint32_t mode)
    : GskEvent(type, std::move(surface), std::move(device), time)
    , fGroup(group)
    , fMode(mode)
    , fButton(button)
    , fIndex(0)
    , fValue(0)
{
    CHECK(type == GskEventType::kPadButtonPress ||
          type == GskEventType::kPadButtonRelease);
}

GskPadEvent::GskPadEvent(Handle<GskSurface> surface,
                         Handle<GskDevice> device,
                         uint32_t time,
                         uint32_t group,
                         uint32_t mode)
    : GskEvent(GskEventType::kPadGroupMode, std::move(surface), std::move(device), time)
    , fGroup(group)
    , fMode(mode)
    , fButton(0)
    , fIndex(0)
    , fValue(0)
{
}

uint32_t GskPadEvent::getButton() const
{
    return fButton;
}

std::tuple<uint32_t, double> GskPadEvent::getAxisValue() const
{
    return {fIndex, fValue};
}

std::tuple<uint32_t, uint32_t> GskPadEvent::getGroupMode() const
{
    return {fGroup, fMode};
}

GskDNDEvent::GskDNDEvent(GskEventType type,
                         Handle<GskSurface> surface,
                         Handle<GskDevice> device,
                         Handle<GskDrop> drop,
                         uint32_t time,
                         Vec2d pos)
    : GskEvent(type, std::move(surface), std::move(device), time)
    , fDrop(std::move(drop))
    , fPosition(pos)
{
    CHECK(type == GskEventType::kDragEnter ||
          type == GskEventType::kDragMotion ||
          type == GskEventType::kDragLeave ||
          type == GskEventType::kDropStart);
}

Maybe<Vec2d> GskDNDEvent::getPosition()
{
    return fPosition;
}

Handle<GskDrop> GskDNDEvent::getDrop() const
{
    return fDrop;
}

GskGrabBrokenEvent::GskGrabBrokenEvent(Handle<GskSurface> surface,
                                       Handle<GskDevice> device,
                                       Handle<GskSurface> grabSurface,
                                       bool implicit)
    : GskEvent(GskEventType::kGrabBroken, std::move(surface), std::move(device), 0)
    , fImplicit(implicit)
    , fGrabSurface(std::move(grabSurface))
{
}

Handle<GskSurface> GskGrabBrokenEvent::getGrabSurface() const
{
    return fGrabSurface;
}

bool GskGrabBrokenEvent::isImplicit() const
{
    return fImplicit;
}

GSK_NAMESPACE_END
