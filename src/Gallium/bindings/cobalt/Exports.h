#ifndef COCOA_GALLIUM_BINDINGS_COBALT_EXPORTS_H
#define COCOA_GALLIUM_BINDINGS_COBALT_EXPORTS_H

#include "Gallium/bindings/Base.h"
#include "Gallium/binder/Class.h"
#include "Cobalt/Cobalt.h"
#include "Cobalt/RenderClientObject.h"

#define GALLIUM_BINDINGS_COBALT_NS_BEGIN    namespace cocoa::gallium::bindings::cobalt_wrap {
#define GALLIUM_BINDINGS_COBALT_NS_END      }

GALLIUM_BINDINGS_COBALT_NS_BEGIN

class RenderClientObjectWrap;
class RenderHostWrap;
class DisplayWrap;
class SurfaceWrap;

struct SlotClosure;

class CobaltBinding : public BindingBase
{
    GALLIUM_BINDING_OBJECT
public:
    CobaltBinding();
    ~CobaltBinding() override;

    void onSetInstanceProperties(v8::Local<v8::Object> instance) override;
    void onRegisterClasses(v8::Isolate *isolate) override;

    ClassExport<RenderHostWrap>         render_host_wrap_;
    ClassExport<RenderClientObjectWrap> render_client_object_wrap_class_;
    ClassExport<DisplayWrap>            display_wrap_class_;
    ClassExport<SurfaceWrap>            surface_wrap_class_;
};

using InfoAcceptorResult = std::optional<std::vector<v8::Local<v8::Value>>>;
using InfoAcceptor = InfoAcceptorResult(*)(v8::Isolate*, ::cocoa::cobalt::RenderHostSlotCallbackInfo&);

/* JSDecl: class RenderHost */
class RenderHostWrap
{
public:
    /**
     * JSDecl:
     * interface ApplicationInfo {
     *   name: string;
     *   major: number;
     *   minor: number;
     *   patch: number;
     * }
     */

    /* JSDecl: function Initialize(info: ApplicationInfo): void */
    static void Initialize(v8::Local<v8::Object> info);

    /* JSDecl: function Dispose(): void */
    static void Dispose();

    /* JSDecL: function Connect(name?: string): Promise<Display> */
    static v8::Local<v8::Value> Connect(const v8::FunctionCallbackInfo<v8::Value>& info);
};

/* JSDecl: class RenderClientObject */
class RenderClientObjectWrap
{
public:
    explicit RenderClientObjectWrap(cobalt::co_sp<cobalt::RenderClientObject> object);
    virtual ~RenderClientObjectWrap();

    gal_nodiscard const cobalt::co_sp<cobalt::RenderClientObject>& getObject() {
        return object_;
    }

    /* JSDecl: function connect(name: string, callback: Function): number */
    uint32_t connect(const std::string& name, v8::Local<v8::Function> callback);

    /* JSDecl: function disconnect(id: number): void */
    void disconnect(uint32_t slotId);

    void defineSignal(const char *name, int32_t code, InfoAcceptor acceptor);
    int32_t getSignalCodeByName(const std::string& name);

private:
    cobalt::co_sp<cobalt::RenderClientObject> object_;
    std::map<std::string, int32_t>            signal_name_map_;
    std::map<uint32_t, InfoAcceptor>          acceptors_map_;
    std::map<uint32_t, std::unique_ptr<SlotClosure>> slot_closures_map_;
};

/* JSDecl: class Display extends RenderClientObject */
class DisplayWrap : public RenderClientObjectWrap
{
public:
    explicit DisplayWrap(cobalt::co_sp<cobalt::RenderClientObject> object);
    ~DisplayWrap() override;

    /* JSDecl: function close(): Promise<void> */
    v8::Local<v8::Value> close();

    /* JSDecl: function createRasterSurface(w: number, h: number): Promise<Surface> */
    v8::Local<v8::Value> createRasterSurface(int32_t width, int32_t height);

    /* JSDecl: function createHWComposeSurface(w: number, h: number): Promise<Surface> */
    v8::Local<v8::Value> createHWComposeSurface(int32_t width, int32_t height);
};

/* JSDecl: class Surface extends RenderClientObject */
class SurfaceWrap : public RenderClientObjectWrap
{
public:
    explicit SurfaceWrap(cobalt::co_sp<cobalt::RenderClientObject> object);
    ~SurfaceWrap() override;

    /* JSDecl: readonly width: number */
    gal_nodiscard int32_t getWidth();

    /* JSDecl: readonly height: number */
    gal_nodiscard int32_t getHeight();

    /* JSDecl: function close(): Promise<void> */
    gal_nodiscard v8::Local<v8::Value> close();

    /* JSDecl: function setTitle(): Promise<void> */
    gal_nodiscard v8::Local<v8::Value> setTitle(const std::string& title);

    /* JSDecl: function resize(width: number, height: number): Promise<bool> */
    gal_nodiscard v8::Local<v8::Value> resize(int32_t width, int32_t height);

    /* JSDecl: function getBuffersDescriptor(): Promise<string> */
    gal_nodiscard v8::Local<v8::Value> getBuffersDescriptor();
};

GALLIUM_BINDINGS_COBALT_NS_END
#endif //COCOA_GALLIUM_BINDINGS_COBALT_EXPORTS_H
