#ifndef COCOA_GALLIUM_GALLIUM_H
#define COCOA_GALLIUM_GALLIUM_H

#include <memory>

#include "Core/Project.h"

#define GALLIUM_NS_BEGIN   namespace cocoa::gallium {
#define GALLIUM_NS_END     }

#define GALLIUM_BINDER_NS_BEGIN     namespace cocoa::gallium::binder {
#define GALLIUM_BINDER_NS_END       }

#define GALLIUM_BINDINGS_NS_BEGIN   namespace cocoa::gallium::bindings {
#define GALLIUM_BINDINGS_NS_END     }

#define GALLIUM_JS_TYPEOF_STRING    "string"
#define GALLIUM_JS_TYPEOF_OBJECT    "object"
#define GALLIUM_JS_TYPEOF_FUNCTION  "function"
#define GALLIUM_JS_TYPEOF_NUMBER    "number"
#define GALLIUM_JS_TYPEOF_BOOLEAN   "boolean"
#define GALLIUM_JS_TYPEOF_UNDEFINED "undefined"

#endif //COCOA_GALLIUM_GALLIUM_H
