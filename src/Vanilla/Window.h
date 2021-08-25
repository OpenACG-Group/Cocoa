#ifndef COCOA_WINDOW_H
#define COCOA_WINDOW_H

#include <cstring>
#include <sigc++/sigc++.h>

#include "include/core/SkRect.h"
#include "Vanilla/Base.h"
#include "Vanilla/Typetraits.h"
#include "Vanilla/KeySymbols.h"
VANILLA_NS_BEGIN

class Display;
class Context;
class Window : public std::enable_shared_from_this<Window>
{
public:
    explicit Window(WeakHandle<Display> display, SkColorType format);
    virtual ~Window() = default;

    static Handle<Window> Make(const Handle<Display>& display,
                                 vec::float2 size, vec::float2 pos, Handle<Window> parent = nullptr);

    virtual void setTitle(const std::string& title) = 0;
    virtual void setResizable(bool resizable) = 0;
    virtual void setIcon(std::istream& stream, std::streamsize size) = 0;
    virtual void show() = 0;
    virtual int32_t width() = 0;
    virtual int32_t height() = 0;
    virtual void update(const SkRect& rect) = 0;
    void update();
    void close();

    inline SkColorType format() const
    { return fFormat; }
    inline Handle<Display> getDisplay()
    { return fDisplay.lock(); }
    Handle<Context> getContext();

    virtual float scaleFactor()
    { return 1.0f; }

    VA_SIG_GETTER(Map)
    VA_SIG_GETTER(Unmap)
    VA_SIG_GETTER(Repaint)
    VA_SIG_GETTER(Close)
    VA_SIG_GETTER(Configure)
    VA_SIG_GETTER(ButtonPress)
    VA_SIG_GETTER(ButtonRelease)
    VA_SIG_GETTER(Motion)
    VA_SIG_GETTER(TouchBegin)
    VA_SIG_GETTER(TouchUpdate)
    VA_SIG_GETTER(TouchEnd)
    VA_SIG_GETTER(KeyPress)
    VA_SIG_GETTER(KeyRelease)

protected:
    virtual void onClose() = 0;

private:
    VA_SIG_FIELDS(VA_SIG_SIGNATURE(void(const Handle<Window>&), Map)
                  VA_SIG_SIGNATURE(void(const Handle<Window>&), Unmap)
                  VA_SIG_SIGNATURE(void(const Handle<Window>&, const SkRect&), Repaint)
                  VA_SIG_SIGNATURE(void(const Handle<Window>&), Close)
                  VA_SIG_SIGNATURE(void(const Handle<Window>&, const SkRect&), Configure)
                  VA_SIG_SIGNATURE(void(const Handle<Window>&, Button, vec::float2), ButtonPress)
                  VA_SIG_SIGNATURE(void(const Handle<Window>&, Button, vec::float2), ButtonRelease)
                  VA_SIG_SIGNATURE(void(const Handle<Window>&, vec::float2), Motion)
                  VA_SIG_SIGNATURE(void(const Handle<Window>&, vec::float2), TouchBegin)
                  VA_SIG_SIGNATURE(void(const Handle<Window>&, vec::float2), TouchUpdate)
                  VA_SIG_SIGNATURE(void(const Handle<Window>&, vec::float2), TouchEnd)
                  VA_SIG_SIGNATURE(void(const Handle<Window>&, KeySymbol,
                          Bitfield<KeyModifier>, Bitfield<KeyLed>), KeyPress)
                  VA_SIG_SIGNATURE(void(const Handle<Window>&, KeySymbol,
                          Bitfield<KeyModifier>, Bitfield<KeyLed>), KeyRelease))

    WeakHandle<Display>       fDisplay;
    SkColorType                 fFormat;
};

VANILLA_NS_END
#endif //COCOA_WINDOW_H
