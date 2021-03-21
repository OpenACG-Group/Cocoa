#ifndef COCOA_NAGUI_H
#define COCOA_NAGUI_H

#include <memory>
#include <list>
#include <queue>

#include "Core/UniquePersistent.h"
#include "Ciallo/Drawable.h"
#include "Ciallo/Cairo2d/CrSurface.h"
#include "Ciallo/Cairo2d/CrCanvas.h"

#define NAGUI_NS_BEGIN  namespace cocoa::NaGui {
#define NAGUI_NS_END    }

NAGUI_NS_BEGIN

enum class FontSlant
{
    kSlant_Normal,
    kSlant_Italic,
    kSlant_Oblique
};

enum class FontWeight
{
    kWeight_Normal,
    kWeight_Bold
};

class NaWindowCollector;
class NaSharedState;

/* NaContext is a container which holds the drawables */
class NaContext : public UniquePersistent<NaContext>
{
    friend class NaWindowCollector;
public:
    NaContext();
    ~NaContext();

    void addDrawable(ciallo::Drawable *pDrawable);
    void removeDrawable(ciallo::Drawable *pDrawable);
    void collectClosedWindowLater(ciallo::Drawable *drawable);

private:
    std::queue<ciallo::Drawable*>   fClosedwindows;
    std::list<ciallo::Drawable*>    fDrawables;
    NaWindowCollector              *fCollector;
};

class NaWindow
{
    friend class NaWindowListener;
public:
    using Ptr = std::unique_ptr<NaWindow>;

    template<typename T, typename...ArgsT>
    static Ptr Make(ArgsT&&... args)
    {
        return std::make_unique<T>(std::forward<ArgsT>(args)...);
    }

    virtual ~NaWindow() = default;

    void paint();

    inline ciallo::Drawable *drawable()
    {
        return fDrawable;
    }

    inline ciallo::CrSurface::Ptr surface()
    {
        return fSurface;
    }

    inline NaSharedState *state()
    {
        return fState;
    }

    void _privSetDrawable(ciallo::Drawable *pDrawable);
    void _privSetSurface(ciallo::CrSurface::Ptr surface);
    void _privSetSharedState(NaSharedState *state);

protected:
    void paintEvent();
    void applyStateUpdates();
    virtual void onRepaint() = 0;

    /* Immediate mode GUI apis */
    void SetFontFamily(const std::string& family);
    void SetFontStyle(FontSlant slant, FontWeight weight);
    void SetFontAntialias(bool antialias);
    void SetFontSize(double size);
    void SetWindowTitle(const std::string& title);
    void SetWindowResizable(bool resizable);

    void TextLabel(const std::string& text);
    bool Button(const std::string& label);

private:
    ciallo::Drawable        *fDrawable;
    ciallo::CrSurface::Ptr   fSurface;
    std::unique_ptr<ciallo::CrCanvas>
                             fCanvas;
    NaSharedState           *fState;
};

void BindDrawableWindow(ciallo::Drawable *pDrawable,
                        NaWindow::Ptr window);

NAGUI_NS_END
#endif // COCOA_NAGUI_H
