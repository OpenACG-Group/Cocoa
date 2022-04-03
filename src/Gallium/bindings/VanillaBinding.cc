#include <iostream>

#include "Core/Project.h"
#include "Core/Journal.h"
#include "Core/EventLoop.h"
#include "Vanilla/Context.h"
#include "Vanilla/Display.h"
#include "Vanilla/Window.h"
#include "Vanilla/DrawContext.h"
#include "Vanilla/RenderKit/InputEventListener.h"
#include "Gallium/lang/VanillaBinding.h"
#include "Gallium/binder/Factory.h"
#include "Gallium/binder/Class.h"
#include "Gallium/binder/Module.h"
#include "Gallium/binder/CallV8.h"

namespace vg = cocoa::vanilla;

GALLIUM_BINDINGS_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.binding)


template<typename T>
T *extract_from_js(v8::Isolate *isolate, v8::Local<v8::Value> object)
{
    CHECK_AND_JS_THROW_WITH_RET(!object->IsObject(), "Not an object", nullptr);
    T *ptr;
    try {
        ptr = binder::Class<T>::unwrap_object(isolate, object);
    } catch (const std::runtime_error& e) {
        CHECK_AND_JS_THROW_WITH_RET(true, "Corrupted object type", nullptr);
    }
    return ptr;
}

/////////////////////////////////////////////////////////
// jni_nvg_Context (Cocoa.nvg.Context)
//

class jni_nvg_Context
{
public:
    explicit jni_nvg_Context(const std::string& displayName);
    ~jni_nvg_Context();

    static binder::Class<jni_nvg_Context> GetClass() {
        return binder::Class<jni_nvg_Context>(v8::Isolate::GetCurrent())
                .constructor<const std::string&>()
                .set("width", binder::Property(&jni_nvg_Context::getWidth))
                .set("height", binder::Property(&jni_nvg_Context::getHeight))
                .set("dispose", &jni_nvg_Context::dispose);
    }

    int32_t getWidth();
    int32_t getHeight();

    void dispose();

    koi_nodiscard inline vg::Handle<vg::Display> getDisplay() {
        return fDisplay;
    }

private:
    vg::Handle<vg::Context>     fContext;
    vg::Handle<vg::Display>     fDisplay;
};

jni_nvg_Context::jni_nvg_Context(const std::string& displayName)
{
    fContext = vg::Context::Make(EventLoop::Instance(), vg::Context::Backend::kXcb);
    fContext->connectTo(displayName.empty() ? nullptr : displayName.c_str(), vg::Context::kDefault);
    if (!fContext->hasDisplay(vg::Context::kDefault))
    {
        binder::throw_(v8::Isolate::GetCurrent(), "Failed to connect to display");
        return;
    }
    fDisplay = fContext->display(vg::Context::kDefault);
}

jni_nvg_Context::~jni_nvg_Context()
{
    if (fDisplay)
        fDisplay->dispose();
}

void jni_nvg_Context::dispose()
{
    if (fDisplay)
    {
        fDisplay->dispose();
        fDisplay = nullptr;
    }
    // Don't destroy Context here because the Display may be referenced by
    // other objects, otherwise it will cause an assertion failure.
    // fContext = nullptr;
}

int32_t jni_nvg_Context::getWidth()
{
    CHECK_AND_JS_THROW_WITH_RET(!fDisplay, "InternalError: null pointer of Display", 0);
    return fDisplay->width();
}

int32_t jni_nvg_Context::getHeight()
{
    CHECK_AND_JS_THROW_WITH_RET(!fDisplay, "InternalError: null pointer of Display", 0);
    return fDisplay->height();
}

/////////////////////////////////////////////////////////
// jni_nvg_Window (Cocoa.nvg.Window)
//

class jni_nvg_Window : public vg::InputEventListener
{
public:
    jni_nvg_Window(v8::Local<v8::Value> jsContext,
                   v8::Local<v8::Value> jsParent,
                   int32_t x, int32_t y, int32_t w, int32_t h);
    ~jni_nvg_Window() override = default;

    static binder::Class<jni_nvg_Window> GetClass() {
        return binder::Class<jni_nvg_Window>(v8::Isolate::GetCurrent())
                .constructor<v8::Local<v8::Object>, v8::Local<v8::Object>, int32_t, int32_t, int32_t, int32_t>()
                .set("width", binder::Property(&jni_nvg_Window::jni_width))
                .set("height", binder::Property(&jni_nvg_Window::jni_height))
                .set("setTitle", &jni_nvg_Window::jni_setTitle)
                .set("setResizable", &jni_nvg_Window::jni_setResizable)
                .set("show", &jni_nvg_Window::jni_show)
                .set("update", &jni_nvg_Window::jni_update)
                .set("close", &jni_nvg_Window::jni_close)
                .set("setEventListener", &jni_nvg_Window::jni_setEventListener)
                .set("unsetEventListener", &jni_nvg_Window::jni_unsetEventListener);
    }

    // TODO: Implement jni_setIcon() method
    void jni_setTitle(const std::string& title);
    void jni_setResizable(bool resizable);
    void jni_show();

    int32_t jni_width();
    int32_t jni_height();

    void jni_update();
    void jni_close();

    void jni_setEventListener(const std::string& event, v8::Local<v8::Value> cbValue);
    void jni_unsetEventListener(const std::string& event);

    static constexpr const size_t WINDOW_EVENT_COUNT = 13;

private:
    v8::Local<v8::Function> getEventListenerCallback(const char *name);

    void onMap() override;
    void onUnmap() override;
    void onResizeOrDrag(const SkRect& rect) override;
    void onRepaint(const SkRect& boundary) override;
    void onClose() override;
    void onButtonPress(vg::Button button, vg::vec::float2 pos) override;
    void onButtonRelease(vg::Button button, vg::vec::float2 pos) override;
    void onMotion(vg::vec::float2 pos) override;
    void onTouchBegin(vg::vec::float2 pos) override;
    void onTouchUpdate(vg::vec::float2 pos) override;
    void onTouchEnd(vg::vec::float2 pos) override;
    void onKeyPress(vg::KeySymbol key, vg::Bitfield<vg::KeyModifier> modifiers) override;
    void onKeyRelease(vg::KeySymbol key, vg::Bitfield<vg::KeyModifier> modifiers) override;

    struct EventCallback
    {
        ~EventCallback() {
            callback.Reset();
        }
        uint64_t id = 0;
        v8::Global<v8::Function> callback;
    };
    
    vg::Handle<vg::Window>      fWindow;
    std::array<EventCallback, WINDOW_EVENT_COUNT>
                                fEventCallbacks;
};

namespace {

const char *gWindowEventNames[jni_nvg_Window::WINDOW_EVENT_COUNT] = {
        "map",
        "unmap",
        "resize-drag",
        "repaint",
        "close",
        "button-press",
        "button-release",
        "motion",
        "touch-begin",
        "touch-update",
        "touch-end",
        "key-press",
        "key-release"
};

} // namespace anonymous

jni_nvg_Window::jni_nvg_Window(v8::Local<v8::Value> jsContext,
                               v8::Local<v8::Value> jsParent,
                               int32_t x, int32_t y, int32_t w, int32_t h)
    : vg::InputEventListener(nullptr)
{
    std::hash<std::string_view> hashFunc;
    for (size_t i = 0; i < WINDOW_EVENT_COUNT; i++)
    {
        fEventCallbacks[i].id = hashFunc(std::string_view(gWindowEventNames[i]));
    }

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_AND_JS_THROW(!jsContext->IsObject(), "Invalid context object");
    auto *context = extract_from_js<jni_nvg_Context>(isolate, jsContext);
    if (!context)
    {
        return;
    }
    CHECK_AND_JS_THROW(!context->getDisplay(), "InternalError: null pointer of Display");

    jni_nvg_Window *parent = nullptr;
    if (jsParent->IsObject())
    {
        parent = extract_from_js<jni_nvg_Window>(isolate, jsParent);
        if (!parent)
        {
            return;
        }
    }

    vg::Handle<vg::Window> parentWindow = parent ? parent->fWindow : nullptr;
    fWindow = context->getDisplay()->createWindow({static_cast<float>(w), static_cast<float>(h)},
                                                  {static_cast<float>(x), static_cast<float>(y)},
                                                  parentWindow);
    CHECK_AND_JS_THROW(!fWindow, "Failed to create a window");
    vg::InputEventListener::connectListenerSlots(fWindow);
}

void jni_nvg_Window::jni_setTitle(const std::string& title)
{
    CHECK_AND_JS_THROW(!fWindow, "Null pointer of Window");
    fWindow->setTitle(title);
}

void jni_nvg_Window::jni_setResizable(bool resizable)
{
    CHECK_AND_JS_THROW(!fWindow, "Null pointer of Window");
    fWindow->setResizable(resizable);
}

void jni_nvg_Window::jni_show()
{
    CHECK_AND_JS_THROW(!fWindow, "Null pointer of Window");
    fWindow->show();
}

void jni_nvg_Window::jni_close()
{
    CHECK_AND_JS_THROW(!fWindow, "Null pointer of Window");
    fWindow->close();
}

int32_t jni_nvg_Window::jni_width()
{
    CHECK_AND_JS_THROW(!fWindow, "Null pointer of Window");
    return fWindow->width();
}

int32_t jni_nvg_Window::jni_height()
{
    CHECK_AND_JS_THROW(!fWindow, "Null pointer of Window");
    return fWindow->height();
}

void jni_nvg_Window::jni_update()
{
    CHECK_AND_JS_THROW(!fWindow, "Null pointer of Window");
    fWindow->update();
}

void jni_nvg_Window::jni_setEventListener(const std::string& event, v8::Local<v8::Value> cbValue)
{
    CHECK_AND_JS_THROW(!cbValue->IsFunction(), "Callback is not a function");

    v8::Local<v8::Function> func = v8::Local<v8::Function>::Cast(cbValue);
    CHECK_AND_JS_THROW(func.IsEmpty(), "Failed to cast callback to a function");

    uint64_t hash = std::hash<std::string>()(event);
    bool found = false;
    for (auto& pair : fEventCallbacks)
    {
        if (pair.id == hash)
        {
            pair.callback.Reset(v8::Isolate::GetCurrent(), func);
            found = true;
            break;
        }
    }
    CHECK_AND_JS_THROW(!found, "Invalid event name");
}

void jni_nvg_Window::jni_unsetEventListener(const std::string& event)
{
    uint64_t hash = std::hash<std::string>()(event);
    bool found = false;
    for (auto& pair : fEventCallbacks)
    {
        if (pair.id == hash)
        {
            pair.callback.Reset();
            found = true;
            break;
        }
    }
    CHECK_AND_JS_THROW(!found, "Invalid event name");
}

v8::Local<v8::Function> jni_nvg_Window::getEventListenerCallback(const char *name)
{
    uint64_t hash = std::hash<std::string_view>()(std::string_view(name));
    for (auto& pair : fEventCallbacks)
    {
        if (pair.id == hash)
        {
            if (pair.callback.IsEmpty())
            {
                return {};
            }
            return pair.callback.Get(v8::Isolate::GetCurrent());
        }
    }
    return {};
}

#define TRY_GET_CALLBACK(name) \
    v8::Isolate *isolate = v8::Isolate::GetCurrent(); \
    v8::Local<v8::Context> context = isolate->GetCurrentContext(); \
    v8::Local<v8::Function> callback; \
    do {                        \
        callback = getEventListenerCallback(name); \
        if (callback.IsEmpty()) \
        {                       \
            return;             \
        }                       \
    } while (false)

void jni_nvg_Window::onMap()
{
    TRY_GET_CALLBACK("map");
    binder::call_v8(isolate, callback, context->Global());
}

void jni_nvg_Window::onUnmap()
{
    TRY_GET_CALLBACK("unmap");
    binder::call_v8(isolate, callback, context->Global());
}

void jni_nvg_Window::onResizeOrDrag(const SkRect& rect)
{
    TRY_GET_CALLBACK("resize-drag");
    binder::call_v8(isolate, callback, context->Global(),
                    static_cast<int32_t>(rect.x()),
                    static_cast<int32_t>(rect.y()),
                    static_cast<int32_t>(rect.width()),
                    static_cast<int32_t>(rect.height()));
}

void jni_nvg_Window::onRepaint(const SkRect& boundary)
{
    TRY_GET_CALLBACK("repaint");
    binder::call_v8(isolate, callback, context->Global(),
                    static_cast<int32_t>(boundary.x()),
                    static_cast<int32_t>(boundary.y()),
                    static_cast<int32_t>(boundary.width()),
                    static_cast<int32_t>(boundary.height()));
}

void jni_nvg_Window::onClose()
{
    TRY_GET_CALLBACK("close");
    binder::call_v8(isolate, callback, context->Global());
}

void jni_nvg_Window::onButtonPress(vg::Button button, vg::vec::float2 pos)
{
    // TODO: this
}

void jni_nvg_Window::onButtonRelease(vg::Button button, vg::vec::float2 pos)
{
    // TODO: this
}

void jni_nvg_Window::onMotion(vg::vec::float2 pos)
{
    TRY_GET_CALLBACK("motion");
    float x = pos[0], y = pos[1];
    binder::call_v8(isolate, callback, context->Global(), x, y);
}

void jni_nvg_Window::onTouchBegin(vg::vec::float2 pos)
{
    TRY_GET_CALLBACK("touch-begin");
    float x = pos[0], y = pos[1];
    binder::call_v8(isolate, callback, context->Global(), x, y);
}

void jni_nvg_Window::onTouchUpdate(vg::vec::float2 pos)
{
    TRY_GET_CALLBACK("touch-update");
    float x = pos[0], y = pos[1];
    binder::call_v8(isolate, callback, context->Global(), x, y);
}

void jni_nvg_Window::onTouchEnd(vg::vec::float2 pos)
{
    TRY_GET_CALLBACK("touch-end");
    float x = pos[0], y = pos[1];
    binder::call_v8(isolate, callback, context->Global(), x, y);
}

void jni_nvg_Window::onKeyPress(vg::KeySymbol key, vg::Bitfield<vg::KeyModifier> modifiers)
{
    // TODO: this
}

void jni_nvg_Window::onKeyRelease(vg::KeySymbol key, vg::Bitfield<vg::KeyModifier> modifiers)
{
    // TODO: this
}

/////////////////////////////////////////////////////////
// Language binding framework
//

VanillaBindingModule::VanillaBindingModule()
    : BindingBase("nvg",
                  "Vanilla engine for Cocoa JavaScript")
{
}

#define LVALUE_OF(value, idx)     auto _lvalue_##idx = value
#define LVALUE_GET(idx)           _lvalue_##idx

void VanillaBindingModule::getModule(binder::Module& mod)
{
    LVALUE_OF(jni_nvg_Context::GetClass(), 1);
    LVALUE_OF(jni_nvg_Window::GetClass(), 2);

    mod.set("Context", LVALUE_GET(1))
       .set("Window", LVALUE_GET(2));
}

GALLIUM_BINDINGS_NS_END
