#ifndef COCOA_EVENTHANDLERMACROS_H
#define COCOA_EVENTHANDLERMACROS_H

#define VA_WIN_HANDLER_SIGNATURE(type) __priv_##type##_handler
#define VA_WIN_HANDLER_DECL(type) \
void VA_WIN_HANDLER_SIGNATURE(type) (const xcb_##type##_event_t *event);

#define VA_WIN_HANDLER_IMPL(klass, type) \
void klass::VA_WIN_HANDLER_SIGNATURE(type) (const xcb_##type##_event_t *event)

#endif //COCOA_EVENTHANDLERMACROS_H
