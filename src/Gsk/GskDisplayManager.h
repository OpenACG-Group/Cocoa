#ifndef COCOA_GSKDISPLAYMANAGER_H
#define COCOA_GSKDISPLAYMANAGER_H

#include <sigc++/sigc++.h>

#include "Gsk/Gsk.h"
GSK_NAMESPACE_BEGIN

class GskDisplay;

class GskDisplayManager
{
public:
    explicit GskDisplayManager(GskGlobalScope *scope);
    ~GskDisplayManager() = default;

    Handle<GskDisplay> openDisplay(const std::string& displayName);

    g_nodiscard g_inline Handle<GskDisplay> getDefaultDisplay() const {
        return fDefaultDisplay;
    }

    g_signal_getter(DisplayOpen);

private:
    g_signal_fields(
        g_signal_signature(void(const Handle<GskDisplay>&), DisplayOpen)
    )

    Handle<GskDisplay>  fDefaultDisplay;
    bool                fWaylandAvailable : 1;
    bool                fXcbAvailable : 1;
};

GSK_NAMESPACE_END
#endif //COCOA_GSKDISPLAYMANAGER_H
