#include "Core/Errors.h"
#include "Koi/binder/ThrowExcept.h"
#include "Koi/binder/Convert.h"

KOI_BINDER_NS_BEGIN

v8::Local<v8::Value> throw_(v8::Isolate* isolate, std::string_view str)
{
    return isolate->ThrowException(to_v8(isolate, str));
}

v8::Local<v8::Value> throw_(v8::Isolate* isolate, std::string_view str,
                            v8::Local<v8::Value> (*builder)(v8::Local<v8::String>))
{
    return isolate->ThrowException(builder(to_v8(isolate, str)));
}

void JSException::Throw(Category category, const std::string& what, v8::Isolate *isolate)
{
    if (isolate == nullptr)
        isolate = v8::Isolate::GetCurrent();
    throw JSException(isolate, what, category);
}

v8::Local<v8::Value> JSException::TakeOver(const JSException& except)
{
    return except.isolate_->ThrowException(except.asException());
}

v8::Local<v8::Value> JSException::asException() const
{
    v8::Local<v8::String> message = to_v8(isolate_, this->what());
    switch (category_)
    {
    case Category::kError:          return v8::Exception::Error(message);
    case Category::kRangeError:     return v8::Exception::RangeError(message);
    case Category::kTypeError:      return v8::Exception::TypeError(message);
    case Category::kReferenceError: return v8::Exception::ReferenceError(message);
    case Category::kSyntaxError:    return v8::Exception::SyntaxError(message);
    case Category::kWasmCompileError: return v8::Exception::WasmCompileError(message);
    case Category::kWasmLinkError:  return v8::Exception::WasmLinkError(message);
    case Category::kWasmRuntimeError: return v8::Exception::WasmRuntimeError(message);
    }
    MARK_UNREACHABLE("Unexpected enumeration value");
}

KOI_BINDER_NS_END
