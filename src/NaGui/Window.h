#ifndef COCOA_WINDOW_H
#define COCOA_WINDOW_H

#include <memory>

#include "Ciallo/Drawable.h"
#include "Ciallo/Cairo2d/CrSurface.h"
#include "Ciallo/Cairo2d/CrCanvas.h"
#include "NaGui/Base.h"
#include "NaGui/Draw.h"
#include "NaGui/DrawContext.h"

NAGUI_NS_BEGIN

class Window
{
    friend class WindowListener;
public:
    using Ptr = std::unique_ptr<Window>;

    template<typename T, typename...ArgsT>
    static Ptr Make(ArgsT&&... args)
    {
        return std::make_unique<T>(std::forward<ArgsT>(args)...);
    }

    Window();
    virtual ~Window() = default;

    void paint();

    inline ciallo::Drawable *drawable()
    {
        return fDrawable;
    }

    inline ciallo::CrSurface::Ptr surface()
    {
        return fSurface;
    }

    void _privSetDrawable(ciallo::Drawable *pDrawable);
    void _privSetSurface(ciallo::CrSurface::Ptr surface);

protected:
    void paintEvent();
    virtual void onRepaint(Draw& draw) = 0;

private:
    ciallo::Drawable        *fDrawable;
    ciallo::CrSurface::Ptr   fSurface;

    std::unique_ptr<Draw>           fDraw;
};

void BindDrawableWindow(ciallo::Drawable *pDrawable,
                        Window::Ptr window);

NAGUI_NS_END
#endif //COCOA_WINDOW_H
