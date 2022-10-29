/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_EXPORTS_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_EXPORTS_H

#include <map>

#include "Gallium/bindings/Base.h"
#include "Gallium/binder/Class.h"
#include "Glamor/ForwardTypeDecls.h"
#include "Glamor/RenderClientObject.h"
#include "Glamor/RenderHostCallbackInfo.h"
#include "Glamor/MaybeGpuObject.h"

#include "include/core/SkRRect.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/core/SkPicture.h"
#include "include/effects/SkRuntimeEffect.h"

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
class CanvasKitTransferContext;

struct SlotClosure;

void GlamorSetInstanceProperties(v8::Local<v8::Object> instance);

using InfoAcceptorResult = std::optional<std::vector<v8::Local<v8::Value>>>;
// using InfoAcceptor = InfoAcceptorResult(*)(v8::Isolate*, ::cocoa::gl::RenderHostSlotCallbackInfo&);
using InfoAcceptor = std::function<InfoAcceptorResult(v8::Isolate*, gl::RenderHostSlotCallbackInfo&)>;

enum class Sampling : uint32_t
{
    kNearest = 0,
    kLinear,
    kCubicMitchell,
    kCubicCatmullRom,

    kLast = kCubicCatmullRom
};

SkSamplingOptions SamplingToSamplingOptions(int32_t v);

//! TSDecl: class RenderHost
class RenderHostWrap
{
public:
    //! TSDecl:
    //! interface ApplicationInfo {
    //!   name: string;
    //!   major: number;
    //!   minor: number;
    //!   patch: number;
    //! }

    //! TSDecl: function Initialize(info: ApplicationInfo): void
    static void Initialize(v8::Local<v8::Object> info);

    //! TSDecl: function Dispose(): void
    static void Dispose();

    //! TSDecl: function Connect(name?: string): Promise<Display>
    static v8::Local<v8::Value> Connect(const v8::FunctionCallbackInfo<v8::Value>& info);

    //! TSDecl:
    //! interface TypefaceSignature {
    //!   family: string;
    //!   weight: number;
    //!   width: number;
    //!   slant: string;
    //! }

    //! TSDecl: function SetTypefaceTransferCallback(func: (signature: TypefaceSignature) => Uint8Array): void
    static void SetTypefaceTransferCallback(v8::Local<v8::Value> func);

    //! TSDecl: function WaitForSyncBarrier(timeoutInMs: number): void;
    static void WaitForSyncBarrier(int64_t timeout);

    //! TSDecl: function SleepRendererFor(timeoutInMs: number): Promise<void>;
    static v8::Local<v8::Value> SleepRendererFor(int64_t timeout);

    //! TSDecl: function TraceGraphicsResources(): Promise<string>
    static v8::Local<v8::Value> TraceGraphicsResources();

    //! TSDecl: function CollectCriticalSharedResources(): void
    static void CollectCriticalSharedResources();
};

//! TSDecl: class RenderClientObject
class RenderClientObjectWrap
{
public:
    explicit RenderClientObjectWrap(gl::Shared<gl::RenderClientObject> object);
    virtual ~RenderClientObjectWrap();

    g_nodiscard const gl::Shared<gl::RenderClientObject>& getObject() {
        return object_;
    }

    //! TSDecl: function connect(name: string, callback: Function): number
    uint32_t connect(const std::string& name, v8::Local<v8::Function> callback);

    //! TSDecl: function disconnect(id: number): void
    void disconnect(uint32_t slotId);

    //! TSDecl:
    //! interface RCOInspectSignal {
    //!   name: string;
    //!   code: number;
    //!   connectedCallbacks: Function[];
    //! }
    //! interface RCOInspectResult {
    //!   objectType: string;
    //!   signals: RCOInspectSignal[];
    //! }

    //! TSDecl: function inspectObject(): RCOInspectResult
    v8::Local<v8::Value> inspectObject();

    void defineSignal(const char *name, int32_t code, InfoAcceptor acceptor);
    int32_t getSignalCodeByName(const std::string& name);

private:
    gl::Shared<gl::RenderClientObject> object_;
    std::map<std::string, int32_t>            signal_name_map_;
    std::map<uint32_t, InfoAcceptor>          acceptors_map_;
    std::map<uint32_t, std::unique_ptr<SlotClosure>> slot_closures_map_;
};

//! TSDecl: class Display extends RenderClientObject
class DisplayWrap : public RenderClientObjectWrap
{
public:
    explicit DisplayWrap(gl::Shared<gl::RenderClientObject> object);
    ~DisplayWrap() override;

    //! TSDecl: function close(): Promise<void>
    v8::Local<v8::Value> close();

    //! TSDecl: function createRasterSurface(w: number, h: number): Promise<Surface>
    v8::Local<v8::Value> createRasterSurface(int32_t width, int32_t height);

    //! TSDecl: function createHWComposeSurface(w: number, h: number): Promise<Surface>
    v8::Local<v8::Value> createHWComposeSurface(int32_t width, int32_t height);

    //! TSDecl: function requestMonitorList(): Promise<Monitor[]>
    v8::Local<v8::Value> requestMonitorList();

    //! TSDecl: readonly defaultCursorTheme: CursorTheme
    v8::Local<v8::Value> getDefaultCursorTheme();

    //! TSDecl: function loadCursorTheme(name: string, size: number): Promise<CursorTheme>
    v8::Local<v8::Value> loadCursorTheme(const std::string& name, int size);

    //! TSDecl: function createCursor(image: CkBitmap, hotspotX: number, hotspotY: number): Promise<Cursor>
    v8::Local<v8::Value> createCursor(v8::Local<v8::Value> bitmap, int hotspotX, int hotspotY);

private:
    std::map<gl::Shared<gl::Monitor>, v8::Global<v8::Object>> monitor_objects_map_;
    v8::Global<v8::Object> default_cursor_theme_;
};

//! TSDecl: class Monitor extends RenderClientObject
class MonitorWrap : public RenderClientObjectWrap
{
public:
    explicit MonitorWrap(gl::Shared<gl::RenderClientObject> object);
    ~MonitorWrap() override = default;

    //! TSDecl: function requestPropertySet(): Promise<void>
    v8::Local<v8::Value> requestPropertySet();
};

//! TSDecl: class CursorTheme extends RenderClientObject
class CursorThemeWrap : public RenderClientObjectWrap
{
public:
    explicit CursorThemeWrap(const std::shared_ptr<gl::CursorTheme>& theme);
    ~CursorThemeWrap() override = default;

    //! TSDecl: function dispose(): Promise<void>;
    v8::Local<v8::Value> dispose();

    //! TSDecl: function loadCursorFromName(name: string): Promise<Cursor>
    v8::Local<v8::Value> loadCursorFromName(const std::string& name);
};

//! TSDecl: class Cursor extends RenderClientObject
class CursorWrap : public RenderClientObjectWrap
{
public:
    explicit CursorWrap(const std::shared_ptr<gl::Cursor>& cursor);
    ~CursorWrap() override = default;

    //! TSDecl: function dispose(): Promise<void>;
    v8::Local<v8::Value> dispose();

    //! TSDecl: function getHotspotVector(): Promise<{x: number, y:number}>
    v8::Local<v8::Value> getHotspotVector();
};

//! TSDecl: class Surface extends RenderClientObject
class SurfaceWrap : public RenderClientObjectWrap
{
public:
    explicit SurfaceWrap(gl::Shared<gl::RenderClientObject> object);
    ~SurfaceWrap() override;

    //! TSDecl: readonly width: number
    g_nodiscard int32_t getWidth();

    //! TSDecl: readonly height: number
    g_nodiscard int32_t getHeight();

    //! TSDecl: function createBlender(): Promise<Blender>
    g_nodiscard v8::Local<v8::Value> createBlender();

    //! TSDecl: function close(): Promise<void>
    g_nodiscard v8::Local<v8::Value> close();

    //! TSDecl: function setTitle(): Promise<void>
    g_nodiscard v8::Local<v8::Value> setTitle(const std::string& title);

    //! TSDecl: function resize(width: number, height: number): Promise<bool>
    g_nodiscard v8::Local<v8::Value> resize(int32_t width, int32_t height);

    //! TSDecl: function getBuffersDescriptor(): Promise<string>
    g_nodiscard v8::Local<v8::Value> getBuffersDescriptor();

    //! TSDecl: function requestNextFrame(): Promise<number>
    g_nodiscard v8::Local<v8::Value> requestNextFrame();

    //! TSDecl: function setMaxSize(width: number, height: number): Promise<void>
    g_nodiscard v8::Local<v8::Value> setMaxSize(int32_t width, int32_t height);

    //! TSDecl: function setMinSize(width: number, height: number): Promise<void>
    g_nodiscard v8::Local<v8::Value> setMinSize(int32_t width, int32_t height);

    //! TSDecl: function setMaximized(value: boolean): Promise<void>
    g_nodiscard v8::Local<v8::Value> setMaximized(bool value);

    //! TSDecl: function setMinimized(value: boolean): Promise<void>
    g_nodiscard v8::Local<v8::Value> setMinimized(bool value);

    //! TSDecl: function setFullscreen(value: boolean, monitor: Monitor): Promise<void>
    g_nodiscard v8::Local<v8::Value> setFullscreen(bool value, v8::Local<v8::Value> monitor);

    //! TSDecl: function setAttachedCursor(cursor: Cursor): Promise<void>
    g_nodiscard v8::Local<v8::Value> setAttachedCursor(v8::Local<v8::Value> cursor);
};

//! TSDecl: class Blender extends RenderClientObject
class BlenderWrap : public RenderClientObjectWrap
{
public:
    explicit BlenderWrap(gl::Shared<gl::RenderClientObject> object);
    ~BlenderWrap() override;

    //! TSDecl: function dispose(): Promise<void>
    v8::Local<v8::Value> dispose();

    //! TSDecl: function update(scene: Scene): Promise<void>
    v8::Local<v8::Value> update(v8::Local<v8::Value> sceneObject);

    //! TSDecl: function createTextureFromImage(image: CkImage, annotation: string): Promise<number>
    v8::Local<v8::Value> createTextureFromImage(v8::Local<v8::Value> image,
                                                const std::string& annotation);

    //! TSDecl: function createTextureFromEncodedData(data: core.Buffer,
    //!                                               alphaType: number | null,
    //!                                               annotation: string): Promise<number>
    v8::Local<v8::Value> createTextureFromEncodedData(v8::Local<v8::Value> buffer,
                                                      v8::Local<v8::Value> alphaType,
                                                      const std::string& annotation);

    //! TSDecl: function createTextureFromPixmap(pixels: core.Buffer,
    //!                                          width: number,
    //!                                          height: number,
    //!                                          colorType: number,
    //!                                          alphaType: number,
    //!                                          annotation: string): Promise<number>
    v8::Local<v8::Value> createTextureFromPixmap(v8::Local<v8::Value> buffer,
                                                 int32_t width,
                                                 int32_t height,
                                                 int32_t colorType,
                                                 int32_t alphaType,
                                                 const std::string& annotation);

    //! TSDecl: function deleteTexture(id: number): Promise<void>
    v8::Local<v8::Value> deleteTexture(int64_t id);

    //! TSDecl: function newTextureDeletionSubscriptionSignal(id: number, sigName: string): Promise<void>
    v8::Local<v8::Value> newTextureDeletionSubscriptionSignal(int64_t id, const std::string& sigName);

    //! TSDecl: function captureNextFrameAsPicture(): Promise<number>
    v8::Local<v8::Value> captureNextFrameAsPicture();

    //! TSDecl: function purgeRasterCacheResources(): Promise<void>
    v8::Local<v8::Value> purgeRasterCacheResources();
};


//! TSDecl:
//! interface CkLTRBRect {
//!   left: number;
//!   top: number;
//!   bottom: number;
//!   right: number;
//! }
//! interface CkXYWHRect {
//!   x: number;
//!   y: number;
//!   width: number;
//!   height: number;
//! }
//! type CkRect = CkLTRBRect | CkXYWHRect | Array<number> | Float32Array;
//!
//! interface CkUniformRadiusRRect {
//!   rect: CkRect;
//!   radius: number;
//! }
//! interface CkXYUniformRadiusRRect {
//!   rect: CkRect;
//!   radiusX: number;
//!   radiusY: number;
//! }
//! interface CkUniformRadiiRRect {
//!   rect: CkRect;
//!   radiusUpperLeft: number;
//!   radiusUpperRight: number;
//!   radiusLowerRight: number;
//!   radiusLowerLeft: number;
//! }
//! interface CkXYRadiiRRect {
//!   rect: CkRect;
//!   radiusUpperLeftX: number;
//!   radiusUpperLeftY: number;
//!   radiusUpperRightX: number;
//!   radiusUpperRightY: number;
//!   radiusLowerRightX: number;
//!   radiusLowerRightY: number;
//!   radiusLowerLeftX: number;
//!   radiusLowerLeftY: number;
//! }
//! type CkRRect = CkUniformRadiusRRect |
//!                CkXYUniformRadiusRRect |
//!                CkUniformRadiiRRect |
//!                CkXYRadiiRRect;

SkRect ExtractCkRect(v8::Isolate *isolate, v8::Local<v8::Value> object);
SkRRect ExtractCkRRect(v8::Isolate *isolate, v8::Local<v8::Value> object);

template<typename T>
class SkiaObjectWrapper
{
public:
    using ValueType = sk_sp<T>;
    explicit SkiaObjectWrapper(ValueType value)
            : wrapped_value_(std::move(value)) {}
    ~SkiaObjectWrapper() = default;

    g_nodiscard g_inline const ValueType& getSkiaObject() const {
        return wrapped_value_;
    }

private:
    ValueType   wrapped_value_;
};

//! TSDecl: class CkImageFilter
class CkImageFilterWrap : public SkiaObjectWrapper<SkImageFilter>
{
public:
    explicit CkImageFilterWrap(sk_sp<SkImageFilter> filter)
        : SkiaObjectWrapper(std::move(filter)) {}
    ~CkImageFilterWrap() = default;

    // DSL syntax:
    //   filter_expr    := IDENT '(' param_list ')'
    //   param_list     := expr
    //                   | param_list ',' expr
    //
    //   array_expr     := '[' array_list ']'
    //   array_list     := expr
    //                   | array_list ',' expr
    //
    //   expr           := INTEGER
    //                   | FLOAT
    //                   | NULL
    //                   | ARGUMENT
    //                   | filter_expr
    //                   | array_expr
    //   INTEGER        := <decimal integer>
    //   FLOAT          := <decimal float>
    //   ARGUMENT       := %IDENT
    //   NULL           := '_'
    //
    // Examples:
    //   blur(3.0, 3.0, %tile, _)
    //   compose(blur(3.0, 3.0, %tile, _), image(%image, %sampling, _, _))

    //! TSDecl: function MakeFromDSL(dsl: string, kwargs: object): CkImageFilter
    static v8::Local<v8::Value> MakeFromDSL(v8::Local<v8::Value> dsl,
                                            v8::Local<v8::Value> kwargs);

    //! TSDecl: function Deserialize(buffer: core.Buffer): CkImageFilter
    static v8::Local<v8::Value> Deserialize(v8::Local<v8::Value> buffer);

    //! TSDecl: function serialize(): core.Buffer
    v8::Local<v8::Value> serialize();
};

//! TSDecl: class CkColorFilter
class CkColorFilterWrap : public SkiaObjectWrapper<SkColorFilter>
{
public:
    explicit CkColorFilterWrap(sk_sp<SkColorFilter> color_filter)
            : SkiaObjectWrapper(std::move(color_filter)) {}
    ~CkColorFilterWrap() = default;

    //! TSDecl: function MakeFromDSL(dsl: string, kwargs: object): CkColorFilter
    static v8::Local<v8::Value> MakeFromDSL(v8::Local<v8::Value> dsl,
                                            v8::Local<v8::Value> kwargs);

    //! TSDecl: function Deserialize(buffer: core.Buffer): CkColorFilter
    static v8::Local<v8::Value> Deserialize(v8::Local<v8::Value> buffer);

    //! TSDecl: function serialize(): core.Buffer
    v8::Local<v8::Value> serialize();
};

//! TSDecl: class CriticalPictureWrap
class CriticalPictureWrap
{
public:
    explicit CriticalPictureWrap(const gl::MaybeGpuObject<SkPicture>& picture)
        : picture_(picture) {}
    ~CriticalPictureWrap() = default;

    //! TSDecl: function sanitize(): Promise<CkPicture>
    v8::Local<v8::Value> sanitize();

    //! TSDecl: function serialize(): Promise<core.Buffer>
    v8::Local<v8::Value> serialize();

    //! TSDecl: function discardOwnership(): void
    void discardOwnership();

    //! TSDecl: function setCollectionCallback(F: () => void): void
    void setCollectionCallback(v8::Local<v8::Value> F);

private:
    gl::MaybeGpuObject<SkPicture> picture_;
    v8::Global<v8::Function> callback_;
};

//! TSDecl: class CkPicture
class CkPictureWrap
{
public:
    explicit CkPictureWrap(sk_sp<SkPicture> picture);
    ~CkPictureWrap();

    /**
     * Skia caches glyphs that have been used to draw. If the picture
     * is a BigPicture containing serialized typeface and image datas,
     * Skia will cache the typeface objects for each provided picture,
     * respectively, which causes a serious memory pressure (memory usage
     * increases almost linearly).
     *
     * Specifying `kTransfer_Usage` makes the picture decoder identify
     * typefaces contained in the picture and cache them by itself.
     * That avoids the memory pressure problem described above.
     */
    enum PictureUsage
    {
        kTransfer_Usage,
        kGeneric_Usage
    };

    //! TSDecl: function MakeFromData(buffer: core.Buffer, usage: number): CkPicture
    static v8::Local<v8::Value> MakeFromData(v8::Local<v8::Value> buffer,
                                             PictureUsage usage);

    /**
     * Compared with `MakeFromData`, `MakeFromFile` is a fastpath if the caller
     * loads a serialized picture from a file.
     * It will map the corresponding file instead of doing unnecessary copies.
     */

    //! TSDecl: function MakeFromFile(path: string, usage: number): CkPicture
    static v8::Local<v8::Value> MakeFromFile(const std::string& path,
                                             PictureUsage usage);

    g_nodiscard const sk_sp<SkPicture>& getPicture() const;

    //! TSDecl: function serialize(): core.Buffer
    g_nodiscard v8::Local<v8::Value> serialize();

    //! TSDecl: function approximateOpCount(nested: bool): number
    g_nodiscard uint32_t approximateOpCount(bool nested);

    //! TSDecl: function approximateByteUsed(): number
    g_nodiscard uint32_t approximateByteUsed();

    //! TSDecl: function uniqueId(): number
    g_nodiscard uint32_t uniqueId();

private:
    sk_sp<SkPicture>    picture_;
    size_t              picture_size_hint_;
};

//! TSDecl: class CkBitmap
class CkBitmapWrap
{
public:
    CkBitmapWrap(v8::Isolate *isolate,
                 v8::Local<v8::Object> buffer_object,
                 std::shared_ptr<SkBitmap> bitmap);
    ~CkBitmapWrap() = default;

    g_nodiscard inline const std::shared_ptr<SkBitmap>& getBitmap() const {
        return bitmap_;
    }

    //! TSDecl: function MakeFromBuffer(buffer: core.Buffer,
    //!                                 width: number, height: number,
    //!                                 colorType: number, alphaType: number): Bitmap
    g_nodiscard static v8::Local<v8::Value> MakeFromBuffer(v8::Local<v8::Value> buffer,
                                                           int32_t width,
                                                           int32_t height,
                                                           uint32_t colorType,
                                                           uint32_t alphaType);

    //! TSDecl: function MakeFromEncodedFile(path: string): Bitmap
    g_nodiscard static v8::Local<v8::Value> MakeFromEncodedFile(const std::string& path);

    //! TSDecl: readonly width: number
    g_nodiscard int32_t getWidth();

    //! TSDecl: readonly height: number
    g_nodiscard int32_t getHeight();

    //! TSDecl: readonly alphaType: number
    g_nodiscard uint32_t getAlphaType();

    //! TSDecl: readonly colorType: number
    g_nodiscard uint32_t getColorType();

    //! TSDecl: readonly bytesPerPixel: number
    g_nodiscard int32_t getBytesPerPixel();

    //! TSDecl: readonly rowBytesAsPixels: number
    g_nodiscard int32_t getRowBytesAsPixels();

    //! TSDecl: readonly getShiftPerPixel: number
    g_nodiscard int32_t getShiftPerPixel();

    //! TSDecl: readonly rowBytes: number
    g_nodiscard size_t getRowBytes();

    //! TSDecl: function computeByteSize(): number
    g_nodiscard size_t computeByteSize();

    //! TSDecl: function toImage(): Image;
    g_nodiscard v8::Local<v8::Value> toImage();

    //! TSDecl: function getPixelBuffer(): Buffer;
    g_nodiscard v8::Local<v8::Value> getPixelBuffer();

private:
    v8::Global<v8::Object>      buffer_object_;
    std::shared_ptr<SkBitmap>   bitmap_;
};

//! TSDecl: class CkImage
class CkImageWrap
{
public:
    explicit CkImageWrap(sk_sp<SkImage> image);

    g_nodiscard inline const sk_sp<SkImage>& getImage() const {
        return image_;
    }

    //! TSDecl: function MakeFromEncodedData(buffer: core.Buffer): Promise<CkImage>
    static v8::Local<v8::Value> MakeFromEncodedData(v8::Local<v8::Value> buffer);

    //! TSDecl: function MakeFromEncodedFile(path: string): Promise<CkImage>
    static v8::Local<v8::Value> MakeFromEncodedFile(const std::string& path);

    //! TSDecl: readonly width: number
    g_nodiscard int32_t getWidth();

    //! TSDecl: readonly height: number
    g_nodiscard int32_t getHeight();

    //! TSDecl: readonly alphaType: number
    g_nodiscard uint32_t getAlphaType();

    //! TSDecl: readonly colorType: number
    g_nodiscard uint32_t getColorType();

    //! TSDecl: function uniqueId(): number
    g_nodiscard uint32_t uniqueId();

    //! TSDecl: function encodeToData(format: uint32_t, quality: number): Buffer
    v8::Local<v8::Value> encodeToData(uint32_t format, int quality);

private:
    sk_sp<SkImage>      image_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_EXPORTS_H
