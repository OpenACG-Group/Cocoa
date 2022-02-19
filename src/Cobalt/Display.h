#ifndef COCOA_COBALT_DISPLAY_H
#define COCOA_COBALT_DISPLAY_H

#include "uv.h"
#include "Cobalt/Cobalt.h"
#include "Cobalt/RenderClientObject.h"
COBALT_NAMESPACE_BEGIN

#define CROP_DISPLAY_CLOSE      1
#define CRSI_DISPLAY_CLOSED     1

class Display : public RenderClientObject
{
public:
    static co_sp<Display> Connect(uv_loop_t *loop, const std::string& name);

    explicit Display(uv_loop_t *eventLoop);
    ~Display() override;

    g_nodiscard g_inline uv_loop_t *GetEventLoop() const {
        return event_loop_;
    }

    void Close();

protected:
    virtual void OnDispose() = 0;

    uv_loop_t           *event_loop_;
    bool                 has_disposed_;
};

COBALT_NAMESPACE_END
#endif //COCOA_COBALT_DISPLAY_H
