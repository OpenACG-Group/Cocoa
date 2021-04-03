#ifndef COCOA_CONTEXT_H
#define COCOA_CONTEXT_H

#include <queue>
#include <list>

#include "Core/UniquePersistent.h"
#include "Ciallo/Drawable.h"
#include "NaGui/Base.h"
NAGUI_NS_BEGIN

class WindowCollector;
class Context : public UniquePersistent<Context>
{
    friend class WindowCollector;
public:
    Context();
    ~Context();

    void addDrawable(ciallo::Drawable *pDrawable);
    void removeDrawable(ciallo::Drawable *pDrawable);
    void collectClosedWindowLater(ciallo::Drawable *drawable);

private:
    std::queue<ciallo::Drawable*>   fClosedwindows;
    std::list<ciallo::Drawable*>    fDrawables;
    WindowCollector                *fCollector;
};

NAGUI_NS_END
#endif //COCOA_CONTEXT_H
