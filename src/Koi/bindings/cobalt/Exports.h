#ifndef COCOA_EXPORTS_H
#define COCOA_EXPORTS_H

#include "Koi/bindings/Base.h"
#include "Koi/binder/Class.h"
#include "Cobalt/Cobalt.h"
#include "Cobalt/RenderClientObject.h"
KOI_BINDINGS_NS_BEGIN

class RenderClientObjectWrap;
class DisplayWrap;

class CobaltBinding : public BindingBase
{
public:
    CobaltBinding();
    ~CobaltBinding() override;

    const char *onGetUniqueId() override;
    void onGetModule(binder::Module& self) override;
    const char **onGetExports() override;

    void onSetInstanceProperties(v8::Local<v8::Object> instance) override;
    void onRegisterClasses(v8::Isolate *isolate) override;

    binder::Class<RenderClientObjectWrap> *render_client_object_wrap_class_;
    binder::Class<DisplayWrap> *display_wrap_class_;
};

/* JSDecl: function renderHostInitialize(application: string): void */
void RenderHostInitialize(const std::string& application);

/* JSDecl: function renderHostDispose(): void */
void RenderHostDispose();

/* JSDecL: function connect(name?: string): Promise<Display> */
v8::Local<v8::Value> Connect(const v8::FunctionCallbackInfo<v8::Value>& info);

/* JSDecl: class RenderClientObject */
class RenderClientObjectWrap
{
public:
    explicit RenderClientObjectWrap(cobalt::co_sp<cobalt::RenderClientObject> object);
    virtual ~RenderClientObjectWrap();

    koi_nodiscard const cobalt::co_sp<cobalt::RenderClientObject>& getObject() {
        return object_;
    }

    /* JSDecl: function connect(name: string, callback: Function): number */
    uint32_t connect(const std::string& name, v8::Local<v8::Function> callback);

    /* JSDecl: function disconnect(id: number): void */
    void disconnect(uint32_t slotId);

    void setSignalName(const char *name, int32_t code);
    int32_t getSignalCodeByName(const std::string& name);

private:
    struct SlotClosure;

    cobalt::co_sp<cobalt::RenderClientObject> object_;
    std::map<std::string, int32_t>            signal_name_map_;
    std::map<uint32_t, SlotClosure*>          slot_closures_map_;
};

/* JSDecl: class Display */
class DisplayWrap : public RenderClientObjectWrap
{
public:
    explicit DisplayWrap(cobalt::co_sp<cobalt::RenderClientObject> object);
    ~DisplayWrap() override;

    /* JSDecl: function close(): Promise<void> */
    v8::Local<v8::Value> close();
};

KOI_BINDINGS_NS_END
#endif //COCOA_EXPORTS_H
