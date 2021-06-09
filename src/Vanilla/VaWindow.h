#ifndef COCOA_VAWINDOW_H
#define COCOA_VAWINDOW_H

#include <cstring>
#include <sigc++/sigc++.h>

#include "include/core/SkRect.h"
#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class VaDisplay;
class Context;
class VaWindow : public std::enable_shared_from_this<VaWindow>
{
public:
    explicit VaWindow(WeakHandle<VaDisplay> display, VaColorFormat format);
    virtual ~VaWindow() = default;

    static Handle<VaWindow> Make(const Handle<VaDisplay>& display,
                                 VaVec2f size, VaVec2f pos, Handle<VaWindow> parent = nullptr);

    virtual void setTitle(const std::string& title) = 0;
    virtual void setResizable(bool resizable) = 0;
    virtual void setIconFile(const std::string& file) = 0;
    virtual void show() = 0;
    virtual int32_t width() = 0;
    virtual int32_t height() = 0;
    virtual void update(const SkRect& rect) = 0;
    void update();
    void close();

    inline VaColorFormat format() const
    { return fFormat; }
    inline Handle<VaDisplay> getDisplay()
    { return fDisplay.lock(); }
    Handle<Context> getContext();

    VA_SIG_GETTER(Map)
    VA_SIG_GETTER(Unmap)
    VA_SIG_GETTER(Repaint)
    VA_SIG_GETTER(Close)
    VA_SIG_GETTER(Configure)

protected:
    virtual void onClose() = 0;

private:
    VA_SIG_FIELDS(VA_SIG_SIGNATURE(void(const Handle<VaWindow>&), Map)
                  VA_SIG_SIGNATURE(void(const Handle<VaWindow>&), Unmap)
                  VA_SIG_SIGNATURE(void(const Handle<VaWindow>&, const SkRect&), Repaint)
                  VA_SIG_SIGNATURE(void(const Handle<VaWindow>&), Close)
                  VA_SIG_SIGNATURE(void(const Handle<VaWindow>&, const SkRect&), Configure))

    WeakHandle<VaDisplay>       fDisplay;
    VaColorFormat               fFormat;
    sigc::signal<void(void)>    fWindowMap;
};

VANILLA_NS_END
#endif //COCOA_VAWINDOW_H
