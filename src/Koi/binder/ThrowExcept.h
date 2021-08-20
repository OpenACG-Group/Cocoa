#ifndef COCOA_THROWEXCEPT_H
#define COCOA_THROWEXCEPT_H

#include <string>

#include "include/v8.h"
#include "Koi/binder/Utility.h"
KOI_BINDER_NS_BEGIN

v8::Local<v8::Value> throw_(v8::Isolate* isolate,
                            std::string_view str);

v8::Local<v8::Value> throw_(v8::Isolate* isolate,
                            std::string_view str,
                            v8::Local<v8::Value>(*builder)(v8::Local<v8::String>));

KOI_BINDER_NS_END
#endif //COCOA_THROWEXCEPT_H
