#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_EXPORTS_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_EXPORTS_H

#include "Gallium/bindings/Base.h"
#include "Gallium/binder/Class.h"
#include "Glamor/Glamor.h"
#include "Glamor/RenderClientObject.h"
#include "Glamor/Monitor.h"
#include "Glamor/Layers/Layer.h"
#include "Glamor/Layers/LayerTree.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/core/SkPicture.h"

#define GALLIUM_BINDINGS_GLAMOR_NS_BEGIN    namespace cocoa::gallium::bindings::glamor_wrap {
#define GALLIUM_BINDINGS_GLAMOR_NS_END      }

GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

class RenderClientObjectWrap;
class RenderHostWrap;
class DisplayWrap;
class SurfaceWrap;
class BlenderWrap;

class Scene;

class CkPictureWrap;
class CkImageWrap;
class CkBitmapWrap;
class MoeHeapObjectBinderWrap;
class MoeTranslationToolchainWrap;

struct SlotClosure;

void GlamorSetInstanceProperties(v8::Local<v8::Object> instance);

using InfoAcceptorResult = std::optional<std::vector<v8::Local<v8::Value>>>;
// using InfoAcceptor = InfoAcceptorResult(*)(v8::Isolate*, ::cocoa::glamor::RenderHostSlotCallbackInfo&);
using InfoAcceptor = std::function<InfoAcceptorResult(v8::Isolate*, glamor::RenderHostSlotCallbackInfo&)>;

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

    /* JSDecl: function MakeBlender(surface: Surface): Promise<Blender> */
    static v8::Local<v8::Value> MakeBlender(v8::Local<v8::Value> surface);
};

/* JSDecl: class RenderClientObject */
class RenderClientObjectWrap
{
public:
    explicit RenderClientObjectWrap(glamor::Shared<glamor::RenderClientObject> object);
    virtual ~RenderClientObjectWrap();

    g_nodiscard const glamor::Shared<glamor::RenderClientObject>& getObject() {
        return object_;
    }

    /* JSDecl: function connect(name: string, callback: Function): number */
    uint32_t connect(const std::string& name, v8::Local<v8::Function> callback);

    /* JSDecl: function disconnect(id: number): void */
    void disconnect(uint32_t slotId);

    // JSDecl:
    // interface RCOInspectSignal {
    //   name: string;
    //   code: number;
    //   connectedCallbacks: Function[];
    // }
    // interface RCOInspectResult {
    //   objectType: string;
    //   signals: RCOInspectSignal[];
    // }

    /* JSDecl: function inspectObject(): RCOInspectResult */
    v8::Local<v8::Value> inspectObject();

    void defineSignal(const char *name, int32_t code, InfoAcceptor acceptor);
    int32_t getSignalCodeByName(const std::string& name);

private:
    glamor::Shared<glamor::RenderClientObject> object_;
    std::map<std::string, int32_t>            signal_name_map_;
    std::map<uint32_t, InfoAcceptor>          acceptors_map_;
    std::map<uint32_t, std::unique_ptr<SlotClosure>> slot_closures_map_;
};

/* JSDecl: class Display extends RenderClientObject */
class DisplayWrap : public RenderClientObjectWrap
{
public:
    explicit DisplayWrap(glamor::Shared<glamor::RenderClientObject> object);
    ~DisplayWrap() override;

    /* JSDecl: function close(): Promise<void> */
    v8::Local<v8::Value> close();

    /* JSDecl: function createRasterSurface(w: number, h: number): Promise<Surface> */
    v8::Local<v8::Value> createRasterSurface(int32_t width, int32_t height);

    /* JSDecl: function createHWComposeSurface(w: number, h: number): Promise<Surface> */
    v8::Local<v8::Value> createHWComposeSurface(int32_t width, int32_t height);

    /* JSDecl: function requestMonitorList(): Promise<Monitor[]> */
    v8::Local<v8::Value> requestMonitorList();

private:
    std::map<glamor::Shared<glamor::Monitor>, v8::Global<v8::Object>> monitor_objects_map_;
};

/* JSDecl: class Monitor extends RenderClientObject */
class MonitorWrap : public RenderClientObjectWrap
{
public:
    explicit MonitorWrap(glamor::Shared<glamor::RenderClientObject> object);
    ~MonitorWrap() override = default;

    /* JSDecl: function requestPropertySet(): Promise<void> */
    v8::Local<v8::Value> requestPropertySet();
};

/* JSDecl: class Surface extends RenderClientObject */
class SurfaceWrap : public RenderClientObjectWrap
{
public:
    explicit SurfaceWrap(glamor::Shared<glamor::RenderClientObject> object);
    ~SurfaceWrap() override;

    /* JSDecl: readonly width: number */
    g_nodiscard int32_t getWidth();

    /* JSDecl: readonly height: number */
    g_nodiscard int32_t getHeight();

    /* JSDecl: function close(): Promise<void> */
    g_nodiscard v8::Local<v8::Value> close();

    /* JSDecl: function setTitle(): Promise<void> */
    g_nodiscard v8::Local<v8::Value> setTitle(const std::string& title);

    /* JSDecl: function resize(width: number, height: number): Promise<bool> */
    g_nodiscard v8::Local<v8::Value> resize(int32_t width, int32_t height);

    /* JSDecl: function getBuffersDescriptor(): Promise<string> */
    g_nodiscard v8::Local<v8::Value> getBuffersDescriptor();

    /* JSDecl: function requestNextFrame(): Promise<number> */
    g_nodiscard v8::Local<v8::Value> requestNextFrame();
};

/* JSDecl: class Blender extends RenderClientObject */
class BlenderWrap : public RenderClientObjectWrap
{
public:
    explicit BlenderWrap(glamor::Shared<glamor::RenderClientObject> object);
    ~BlenderWrap() override;

    /* JSDecl: function dispose(): Promise<void> */
    g_nodiscard v8::Local<v8::Value> dispose();

    /**
     * `updateScene` is the most important interface that connects Gallium with Glamor
     * graphics layer. Instead of being implemented by Glamor, it is not implemented by
     * Gallium directly.
     */

    /* JSDecl: function update(scene: Scene): Promise<void> */
    g_nodiscard v8::Local<v8::Value> update(v8::Local<v8::Value> sceneObject);
};


/**
 * JSDecl:
 * interface CkRect {
 *   left: number;
 *   top: number;
 *   bottom: number;
 *   right: number;
 * }
 */

SkRect CkRectToSkRectCast(v8::Isolate *isolate, v8::Local<v8::Value> object);
SkIRect CkRectToSkIRectCast(v8::Isolate *isolate, v8::Local<v8::Value> object);

/* JSDecl: class CkPicture */
class CkPictureWrap
{
public:
    explicit CkPictureWrap(sk_sp<SkPicture> picture);
    ~CkPictureWrap() = default;

    /* JSDecl: function MakeFromData(buffer: core.Buffer): CkPicture */
    static v8::Local<v8::Value> MakeFromData(v8::Local<v8::Value> buffer);


    /**
     * Compared with `MakeFromData`, `MakeFromFile` is a fastpath if the caller
     * loads a serialized picture from a file.
     * It will map the corresponding file instead of doing unnecessary copies.
     */

    /* JSDecl: function MakeFromFile(path: string): CkPicture */
    static v8::Local<v8::Value> MakeFromFile(const std::string& path);

    g_nodiscard const sk_sp<SkPicture>& getPicture() const;

    /* JSDecl: function serialize(): core.Buffer */
    g_nodiscard v8::Local<v8::Value> serialize();

    /* JSDecl: function approximateOpCount(nested: bool): number */
    g_nodiscard uint32_t approximateOpCount(bool nested);

    /* JSDecl: function approximateByteUsed(): number */
    g_nodiscard uint32_t approximateByteUsed();

    /* JSDecl: function uniqueId(): number */
    g_nodiscard uint32_t uniqueId();

private:
    sk_sp<SkPicture>    picture_;
};

/* JSDecl: class CkBitmap */
class CkBitmapWrap
{
public:
    g_nodiscard inline const std::shared_ptr<SkBitmap>& getBitmap() const {
        return bitmap_;
    }

private:
    std::shared_ptr<SkBitmap> bitmap_;
};

/* JSDecl: class CkImage */
class CkImageWrap
{
public:
    explicit CkImageWrap(sk_sp<SkImage> image);

    g_nodiscard inline const sk_sp<SkImage>& getImage() const {
        return image_;
    }

    /* JSDecl: function MakeFromEncodedData(buffer: core.Buffer): Promise<CkImage> */
    static v8::Local<v8::Value> MakeFromEncodedData(v8::Local<v8::Value> buffer);

    /* JSDecl: function MakeFromEncodedFile(path: string): Promise<CkImage> */
    static v8::Local<v8::Value> MakeFromEncodedFile(const std::string& path);

    /* JSDecl: function encodeToData(format: uint32_t, quality: number): Buffer */
    v8::Local<v8::Value> encodeToData(uint32_t format, int quality);

private:
    sk_sp<SkImage>      image_;
};

/* JSDecl: class MoeHeapObjectBinder */
class MoeHeapObjectBinderWrap
{
public:
    enum class Type
    {
        kString,
        kBitmap,
        kImage,
        kPicture
    };

    using TypedObjectPair = std::pair<Type, v8::Global<v8::Value>>;
    using ObjectMap = std::unordered_map<uint32_t, TypedObjectPair>;

    void bindString(uint32_t key, v8::Local<v8::Value> string);
    void bindBitmap(uint32_t key, v8::Local<v8::Value> bitmap);
    void bindImage(uint32_t key, v8::Local<v8::Value> image);
    void bindPicture(uint32_t key, v8::Local<v8::Value> picture);

    g_nodiscard inline auto& getBoundObjects() {
        return bound_objects_;
    }

private:
    ObjectMap   bound_objects_;
};

/* JSDecl: class MoeTranslationToolchain */
class MoeTranslationToolchainWrap
{
public:
    /**
     * JSDecl:
     * interface IHeapProfiling {
     *   heapSingleCellSize: number;
     *   heapTotalSize: number;
     *   heapAllocationsCount: number;
     *   heapExtractionsCount: number;
     *   heapLeakedCellsCount: number;
     * }
     *
     * interface ICompileResult {
     *   artifact?: GskPicture;
     *   heapProfiling?: IHeapProfiling;
     * }
     *
     * interface IMoeBreakpointCallbacks {
     * }
     */

    /**
     * JSDecl: function Interpreter(array: Array<core.Buffer>,
     *                              binder: IMoeHeapObjectBinder,
     *                              breakpointCallbacks: IMoeBreakpointCallbacks,
     *                              heapProfiling: boolean): ICompileResult
     */
    static v8::Local<v8::Value> Interpreter(v8::Local<v8::Value> array,
                                            v8::Local<v8::Value> binder,
                                            v8::Local<v8::Value> breakpointCallbacks,
                                            bool heapProfiling);

    static v8::Local<v8::Value> Compile();


    /* JSDecl: function Disassemble(array: Array<core.Buffer>): string */
    static v8::Local<v8::Value> Disassemble(v8::Local<v8::Value> array);
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_EXPORTS_H
