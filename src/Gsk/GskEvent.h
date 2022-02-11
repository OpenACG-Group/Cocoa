#ifndef COCOA_GSKEVENT_H
#define COCOA_GSKEVENT_H

#include "Core/EnumClassBitfield.h"

#include "Gsk/Gsk.h"
#include "Gsk/GskEnumerations.h"
GSK_NAMESPACE_BEGIN

enum class GskEventType
{
    /* The window manager has requested that the toplevel surface be
       hidden or destroyed, usually when the user clicks on a special
       icon in the title bar. */
    kDelete,
    /* The pointer (usually a mouse) has moved */
    kMotionNotify,
    /* A mouse button has been pressed */
    kButtonPress,
    /* A mouse button has been released */
    kButtonRelease,
    /* A key has been pressed */
    kKeyPress,
    /* A key has been released */
    kKeyRelease,
    /* The pointer has entered the surface */
    kEnterNotify,
    /* The pointer has left the surface */
    kLeaveNotify,
    /* The keyboard focus has entered or left the surface */
    kFocusChange,
    /* An input device has moved into contact with a sensing surface.
      (for example, a touchscreen or graphics tablet) */
    kProximityIn,
    /* An input device has moved out of contact with a sensing surface */
    kProximityOut,
    /* The mouse has entered the surface while a drag is in progress */
    kDragEnter,
    /* The mouse has left the surface while a drag is in progress */
    kDragLeave,
    /* The mouse has moved in the surface while a drag is in progress */
    kDragMotion,
    /* A drop operation onto the surface has started */
    kDropStart,
    /* The scroll wheel was turned */
    kScroll,
    /* A pointer or keyboard grab was broken */
    kGrabBroken,
    /* A new touch event sequence has just started */
    kTouchBegin,
    /* A touch event sequence has been updated */
    kTouchUpdate,
    /* A touch event sequence has finished */
    kTouchEnd,
    /* A touch event sequence has been cancelled */
    kTouchCancel,
    /* A touchpad swipe gesture event, the current state is
       determined by its phase field */
    kTouchpadSwipe,
    /* A touchpad pinch gesture event, the current state is
      determined by its phase field */
    kTouchpadPinch,
    /* A tablet pad button press event */
    kPadButtonPress,
    /* A tablet pad button release event */
    kPadButtonRelease,
    /* A tablet pad axis event from a "ring" */
    kPadRing,
    /* A tablet pad axis event from a "strip" */
    kPadStrip,
    /* A tablet pad group mode change */
    kPadGroupMode
};

enum class GskTouchpadGesturePhase
{
    /* The gesture has begun */
    kBegin,
    /* The gesture has been updated */
    kUpdate,
    /* The gesture was finished, changes should be permanently applied */
    kEnd,
    /* The gesture was cancelled, all changes should be undone */
    kCancel
};

enum class GskScrollDirection
{
    kUp,
    kDown,
    kLeft,
    kRight,
    /* The scrolling is determined by the delta values in scroll events */
    kSmooth
};

enum class GskNotifyType
{
    /* The surface is entered from an ancestor or left towards an ancestor */
    kAncestor,
    /* The pointer moves between an ancestor and an inferior of the surface */
    kVirtual,
    /* The surface is entered from an inferior or left towards an inferior */
    kInferior,
    /* The surface is entered from or left towards a surface which is neither
       an ancestor nor an inferior. */
    kNonlinear,
    /* The pointer moves between two surfaces which are not ancestors of
       each other and the surface is part of the ancestor chain between
       one of these surfaces and their least common ancestor. */
    kNonlinearVirtual,
    kUnknown
};

enum class GskCrossingMode
{
    kNormal,
    kGrab,
    kUnGrab,
    kStateChanged,
    kTouchBegin,
    kTouchEnd,
    kDeviceSwitch
};

class GskSurface;
class GskDevice;
class GskDrop;

// Opaque data structure
class GskEventSequence;

#define GSK_EVENT_FLAG_PENDING      (1 << 0)
#define GSK_EVENT_FLAG_FLUSHED      (1 << 1)

class GskEvent
{
public:
    using AxesVec = std::vector<double>;

    virtual ~GskEvent() = default;

    template<typename T>
    static Handle<T> As(const Handle<GskEvent>& event)
    {
        return std::dynamic_pointer_cast<T>(event);
    }

    static Maybe<double> Distance(const Handle<GskEvent>& e1, const Handle<GskEvent>& e2);
    static Maybe<double> Angle(const Handle<GskEvent>& e1, const Handle<GskEvent>& e2);
    static Maybe<Vec2d> Center(const Handle<GskEvent>& e1, const Handle<GskEvent>& e2);

    g_nodiscard uint8_t& getFlagsRef();
    g_nodiscard GskEventType getEventType() const;
    g_nodiscard Handle<GskSurface> getSurface() const;
    g_nodiscard Handle<GskDevice> getDevice() const;
    g_nodiscard uint32_t getTime() const;

    // TODO: Get GskSeat, GskDeviceTool, GskDisplay

    g_nodiscard Maybe<double> getAxis(GskAxisUse use);
    g_nodiscard virtual bool getPointerEmulated();
    g_nodiscard virtual Maybe<Vec2d> getPosition();
    g_nodiscard virtual Handle<GskEventSequence> getSequence();
    g_nodiscard virtual Bitfield<GskModifierType> getState();
    g_nodiscard virtual const AxesVec& getAxes();

protected:
    GskEvent(GskEventType type,
             Handle<GskSurface> surface,
             Handle<GskDevice> device,
             uint32_t time);

private:
    GskEventType        fType;
    Handle<GskSurface>  fSurface;
    Handle<GskDevice>   fDevice;
    uint32_t            fTime;
    uint8_t             fFlags;
};

class GskButtonEvent : public GskEvent
{
public:
    GskButtonEvent(GskEventType type,
                   Handle<GskSurface> surface,
                   Handle<GskDevice> device,
                   uint32_t time,
                   Bitfield<GskModifierType> state,
                   uint32_t button,
                   Vec2d pos,
                   AxesVec axes);
    ~GskButtonEvent() override = default;

    Maybe<Vec2d> getPosition() override;
    Bitfield<GskModifierType> getState() override;
    const AxesVec& getAxes() override;

    g_nodiscard uint32_t getButton() const;

private:
    Bitfield<GskModifierType>   fState;
    uint32_t                    fButton;
    Vec2d                       fPosition;
    AxesVec                     fAxes;
};

class GskCrossingEvent : public GskEvent
{
public:
    GskCrossingEvent(GskEventType type,
                     Handle<GskSurface> surface,
                     Handle<GskDevice> device,
                     uint32_t time,
                     Bitfield<GskModifierType> state,
                     Vec2d pos,
                     GskCrossingMode mode,
                     GskNotifyType notify);
    ~GskCrossingEvent() override = default;

    Bitfield<GskModifierType> getState() override;
    Maybe<Vec2d> getPosition() override;

    g_nodiscard GskCrossingMode getMode() const;
    g_nodiscard GskNotifyType getDetail() const;
    g_nodiscard bool isFocused() const;

private:
    Bitfield<GskModifierType>   fState;
    GskCrossingMode             fMode;
    Vec2d                       fPosition;
    GskNotifyType               fDetail;
    bool                        fFocus;
    Handle<GskSurface>          fChildSurface;
};

class GskDeleteEvent : public GskEvent
{
public:
    explicit GskDeleteEvent(Handle<GskSurface> surface);
    ~GskDeleteEvent() override = default;
};

class GskDNDEvent : public GskEvent
{
public:
    GskDNDEvent(GskEventType type,
                Handle<GskSurface> surface,
                Handle<GskDevice> device,
                Handle<GskDrop> drop,
                uint32_t time,
                Vec2d pos);
    ~GskDNDEvent() override = default;

    Maybe<Vec2d> getPosition() override;

    g_nodiscard Handle<GskDrop> getDrop() const;

private:
    Handle<GskDrop> fDrop;
    Vec2d           fPosition;
};

class GskFocusEvent : public GskEvent
{
public:
    GskFocusEvent(Handle<GskSurface> surface,
                  Handle<GskDevice> device,
                  bool focusIn);
    ~GskFocusEvent() override = default;

    g_nodiscard bool isFocusIn() const;

private:
    bool    fFocusIn;
};

class GskGrabBrokenEvent : public GskEvent
{
public:
    GskGrabBrokenEvent(Handle<GskSurface> surface,
                       Handle<GskDevice> device,
                       Handle<GskSurface> grabSurface,
                       bool implicit);
    ~GskGrabBrokenEvent() override = default;

    g_nodiscard Handle<GskSurface> getGrabSurface() const;
    g_nodiscard bool isImplicit() const;

private:
    bool        fImplicit;
    Handle<GskSurface> fGrabSurface;
};

class GskKeyEvent : public GskEvent
{
public:
    struct Translated
    {
        uint32_t keyval{0};
        Bitfield<GskModifierType> consumed;
        uint32_t layout{0};
        uint32_t level{0};
    };

    GskKeyEvent(GskEventType type,
                Handle<GskSurface> surface,
                Handle<GskDevice> device,
                uint32_t time,
                uint32_t keycode,
                Bitfield<GskModifierType> modifiers,
                bool isModifier,
                const Translated& translated,
                const Translated& noLock);
    ~GskKeyEvent() override = default;

    Bitfield<GskModifierType> getState() override;

    g_nodiscard uint32_t getKeyval() const;
    g_nodiscard uint32_t getKeycode() const;
    g_nodiscard Bitfield<GskModifierType> getConsumedModifiers() const;
    g_nodiscard uint32_t getLayout() const;
    g_nodiscard uint32_t getLevel() const;
    g_nodiscard bool isModifier() const;

private:
    Bitfield<GskModifierType>   fState;
    uint32_t                    fKeycode;
    bool                        fKeyIsModifier;
    /* The result of translation `fKeycode`. First with the full state,
       then while ignoring CapsLock. */
    Translated                  fTranslated[2];
};

class GskMotionEvent : public GskEvent
{
public:
    GskMotionEvent(Handle<GskSurface> surface,
                   Handle<GskDevice> device,
                   uint32_t time,
                   Bitfield<GskModifierType> state,
                   Vec2d pos,
                   AxesVec axes);
    ~GskMotionEvent() override = default;

    Maybe<Vec2d> getPosition() override;
    Bitfield<GskModifierType> getState() override;
    const AxesVec& getAxes() override;

private:
    Bitfield<GskModifierType>   fState;
    /* coordinates of the pointer relative to the surface */
    Vec2d                       fPosition;
    /* (x, y) translated to the axes of device.
       Empty if device os the mouse */
    AxesVec                     fAxes;
};

class GskPadEvent : public GskEvent
{
public:
    /* ring/strip constructor */
    GskPadEvent(GskEventType type,
                Handle<GskSurface> surface,
                Handle<GskDevice> device,
                uint32_t time,
                uint32_t group,
                uint32_t index,
                uint32_t mode,
                double value);

    /* Button constructor */
    GskPadEvent(GskEventType type,
                Handle<GskSurface> surface,
                Handle<GskDevice> device,
                uint32_t time,
                uint32_t group,
                uint32_t button,
                uint32_t mode);

    /* Group mode constructor */
    GskPadEvent(Handle<GskSurface> surface,
                Handle<GskDevice> device,
                uint32_t time,
                uint32_t group,
                uint32_t mode);

    ~GskPadEvent() override = default;

    g_nodiscard uint32_t getButton() const;
    g_nodiscard std::tuple<uint32_t, double> getAxisValue() const;
    g_nodiscard std::tuple<uint32_t, uint32_t> getGroupMode() const;

private:
    uint32_t    fGroup;
    uint32_t    fMode;
    uint32_t    fButton;
    uint32_t    fIndex;
    double      fValue;
};

class GskProximityEvent : public GskEvent
{
private:
    GskProximityEvent(GskEventType type,
                      Handle<GskSurface> surface,
                      Handle<GskDevice> device,
                      uint32_t time);
    ~GskProximityEvent() override = default;
};

class GskScrollEvent : public GskEvent
{
public:
    /* Smooth constructor */
    GskScrollEvent(Handle<GskSurface> surface,
                   Handle<GskDevice> device,
                   uint32_t time,
                   Bitfield<GskModifierType> state,
                   Vec2d deltas,
                   bool isStop);

    /* Discrete constructor */
    GskScrollEvent(Handle<GskSurface> surface,
                   Handle<GskDevice> device,
                   uint32_t time,
                   Bitfield<GskModifierType> state,
                   GskScrollDirection direction,
                   bool emulated);

    ~GskScrollEvent() override = default;

    Bitfield<GskModifierType> getState() override;
    bool getPointerEmulated() override;

    g_nodiscard GskScrollDirection getDirection() const;
    g_nodiscard Vec2d getDeltas() const;
    g_nodiscard bool isStop() const;

private:
    Bitfield<GskModifierType>   fState;
    GskScrollDirection          fDirection;
    Vec2d                       fDeltas;
    bool                        fPointerEmulated;
    bool                        fIsStop;
};

class GskTouchEvent : public GskEvent
{
public:
    GskTouchEvent(GskEventType type,
                  Handle<GskEventSequence> sequence,
                  Handle<GskSurface> surface,
                  Handle<GskDevice> device,
                  uint32_t time,
                  Bitfield<GskModifierType> state,
                  Vec2d pos,
                  AxesVec axes,
                  bool emulating);
    ~GskTouchEvent() override = default;

    Bitfield<GskModifierType> getState() override;
    Maybe<Vec2d> getPosition() override;
    const AxesVec& getAxes() override;
    Handle<GskEventSequence> getSequence() override;
    bool getPointerEmulated() override;

private:
    Bitfield<GskModifierType>   fState;
    Vec2d                       fPosition;
    AxesVec                     fAxes;
    Handle<GskEventSequence>    fSequence;
    bool                        fPointerEmulating;
};

class GskTouchpadEvent : public GskEvent
{
public:
    /* Swipe constructor */
    GskTouchpadEvent(Handle<GskSurface> surface,
                     Handle<GskEventSequence> sequence,
                     Handle<GskDevice> device,
                     uint32_t time,
                     Bitfield<GskModifierType> state,
                     GskTouchpadGesturePhase phase,
                     Vec2d pos,
                     int fingers,
                     Vec2d deltas);

    /* Pinch constructor */
    GskTouchpadEvent(Handle<GskSurface> surface,
                     Handle<GskEventSequence> sequence,
                     Handle<GskDevice> device,
                     uint32_t time,
                     Bitfield<GskModifierType> state,
                     GskTouchpadGesturePhase phase,
                     Vec2d pos,
                     int fingers,
                     Vec2d deltas,
                     double scale,
                     double angleDelta);

    ~GskTouchpadEvent() override = default;

    Handle<GskEventSequence> getSequence() override;
    Bitfield<GskModifierType> getState() override;
    Maybe<Vec2d> getPosition() override;

    g_nodiscard GskTouchpadGesturePhase getGesturePhase() const;
    g_nodiscard int getFingers() const;
    g_nodiscard Vec2d getDeltas() const;
    g_nodiscard double getPinchAngleDelta() const;
    g_nodiscard double getPinchScale() const;

private:
    Handle<GskEventSequence>    fSequence;
    Bitfield<GskModifierType>   fState;
    GskTouchpadGesturePhase     fPhase;
    int                         fFingers;
    Vec2d                       fPosition;
    Vec2d                       fDeltas;
    double                      fAngleDelta;
    double                      fScale;
};

GSK_NAMESPACE_END
#endif //COCOA_GSKEVENT_H
