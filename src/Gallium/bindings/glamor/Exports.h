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
#include "Gallium/bindings/glamor/Types.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
#include "Gallium/binder/Class.h"
#include "Gallium/bindings/EventEmitter.h"
#include "Glamor/ForwardTypeDecls.h"
#include "Glamor/PresentRemoteHandle.h"
#include "Glamor/PresentRemoteCallReturn.h"
#include "Glamor/MaybeGpuObject.h"

#include "include/core/SkRRect.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/core/SkPicture.h"
#include "include/effects/SkRuntimeEffect.h"

GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

class DisplayWrap;
class SurfaceWrap;
class BlenderWrap;

class Scene;

class CkPictureWrap;
class CkImageWrap;
class CkBitmapWrap;

void GlamorSetInstanceProperties(v8::Local<v8::Object> instance);

enum class Capabilities : uint32_t
{
    kHWComposeEnabled,
    kProfilerEnabled,
    kProfilerMaxSamples,
    kMessageQueueProfilingEnabled,

    kLast = kMessageQueueProfilingEnabled
};

//! TSDecl: function queryCapabilities(cap: Enum<Capabilities>): boolean | number | string
v8::Local<v8::Value> QueryCapabilities(uint32_t cap);

//! TSDecl: function setApplicationInfo(name: string, major: number,
//!                                     minor: number, patch: number): void
void SetApplicationInfo(const std::string& name, int32_t major,
                        int32_t minor, int32_t patch);

//! TSDecl: function traceSkiaMemoryJSON(): string
v8::Local<v8::Value> TraceSkiaMemoryJSON();

//! TSDecl: class PresentThread
class PresentThreadWrap : public ExportableObjectBase
{
public:
    explicit PresentThreadWrap(gl::PresentThread *thread) : thread_(thread) {}

    //! TSDecl: function Start(): PresentThread
    static v8::Local<v8::Value> Start();

    //! TSDecl: function dispose(): void
    void dispose();

    //! TSDecl: function createDisplay(): Promise<Display>
    v8::Local<v8::Value> createDisplay();

    //! TSDecl: function traceResourcesJSON(): Promise<string>
    v8::Local<v8::Value> traceResourcesJSON();

    //! TSDecl: function collect(): void
    void collect();

private:
    void CheckDisposeOrThrow();

    gl::PresentThread *thread_;
};

//! TSDecl: class GProfiler
class GProfilerWrap : public ExportableObjectBase
{
public:
    explicit GProfilerWrap(gl::Shared<gl::GProfiler> profiler)
        : profiler_(std::move(profiler)) {}
    ~GProfilerWrap() = default;

    //! TSDecl: function purgeRecentHistorySamples(freeMemory: boolean): void
    void purgeRecentHistorySamples(bool free_memory);

    //! TSDecl: function generateCurrentReport(): Report
    v8::Local<v8::Value> generateCurrentReport();

private:
    gl::Shared<gl::GProfiler> profiler_;
};

//! TSDecl: class Display extends EventEmitterBase
class DisplayWrap : public ExportableObjectBase,
                    public EventEmitterBase
{
public:
    explicit DisplayWrap(gl::Shared<gl::PresentRemoteHandle> handle);
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
    v8::Local<v8::Object> OnGetObjectSelf(v8::Isolate *isolate) override;

    using MonitorsMap = std::map<std::shared_ptr<gl::Monitor>, v8::Global<v8::Object>>;

    std::shared_ptr<gl::PresentRemoteHandle>     handle_;
    MonitorsMap                                 monitor_objects_map_;
    v8::Global<v8::Object>                      default_cursor_theme_;
};

//! TSDecl: class Monitor extends EventEmitterBase
class MonitorWrap : public ExportableObjectBase,
                    public EventEmitterBase
{
public:
    explicit MonitorWrap(gl::Shared<gl::PresentRemoteHandle> handle);
    ~MonitorWrap() override = default;

    //! TSDecl: function requestPropertySet(): Promise<void>
    v8::Local<v8::Value> requestPropertySet();

private:
    v8::Local<v8::Object> OnGetObjectSelf(v8::Isolate *isolate) override;

    std::shared_ptr<gl::PresentRemoteHandle> handle_;
};

//! TSDecl: class CursorTheme
class CursorThemeWrap : public ExportableObjectBase
{
public:
    explicit CursorThemeWrap(std::shared_ptr<gl::CursorTheme> handle);
    ~CursorThemeWrap() = default;

    //! TSDecl: function dispose(): Promise<void>;
    v8::Local<v8::Value> dispose();

    //! TSDecl: function loadCursorFromName(name: string): Promise<Cursor>
    v8::Local<v8::Value> loadCursorFromName(const std::string& name);

private:
    std::shared_ptr<gl::CursorTheme> handle_;
};

//! TSDecl: class Cursor
class CursorWrap : public ExportableObjectBase
{
public:
    explicit CursorWrap(std::shared_ptr<gl::Cursor> handle);
    ~CursorWrap() = default;

    g_nodiscard g_inline std::shared_ptr<gl::Cursor> GetCursorHandle() const {
        return handle_;
    }

    //! TSDecl: function dispose(): Promise<void>;
    v8::Local<v8::Value> dispose();

    //! TSDecl: function getHotspotVector(): Promise<{x: number, y:number}>
    v8::Local<v8::Value> getHotspotVector();

private:
    std::shared_ptr<gl::Cursor> handle_;
};

//! TSDecl: class Surface extends EventEmitterBase
class SurfaceWrap : public ExportableObjectBase,
                    public EventEmitterBase
{
public:
    SurfaceWrap(gl::Shared<gl::PresentRemoteHandle> handle,
                v8::Local<v8::Object> display);
    ~SurfaceWrap() override;

    //! TSDecl: readonly width: number
    g_nodiscard int32_t getWidth();

    //! TSDecl: readonly height: number
    g_nodiscard int32_t getHeight();

    //! TSDecl: readonly display: Display
    g_nodiscard v8::Local<v8::Value> getDisplay();

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

private:
    v8::Local<v8::Object> OnGetObjectSelf(v8::Isolate *isolate) override;

    std::shared_ptr<gl::PresentRemoteHandle> handle_;
    v8::Global<v8::Object>                  display_wrapped_;
    uint32_t                                surface_closed_slot_;
};

//! TSDecl: class Blender extends EventEmitterBase
class BlenderWrap : public ExportableObjectBase
                  , public EventEmitterBase
{
public:
    explicit BlenderWrap(gl::Shared<gl::PresentRemoteHandle> handle);
    ~BlenderWrap() override;

    //! TSDecl: readonly profiler: null | GProfiler
    v8::Local<v8::Value> getProfiler();

    //! TSDecl: function dispose(): Promise<void>
    v8::Local<v8::Value> dispose();

    //! TSDecl: function update(scene: Scene): Promise<void>
    v8::Local<v8::Value> update(v8::Local<v8::Value> sceneObject);

    //! TSDecl: function captureNextFrameAsPicture(): Promise<number>
    v8::Local<v8::Value> captureNextFrameAsPicture();

    //! TSDecl: function purgeRasterCacheResources(): Promise<void>
    v8::Local<v8::Value> purgeRasterCacheResources();

    //! TSDecl: function importGpuSemaphoreFd(fd: GpuExportedFd): Promise<bigint>
    v8::Local<v8::Value> importGpuSemaphoreFd(v8::Local<v8::Value> fd);

    //! TSDecl: function deleteImportedSemaphore(id: bigint): Promise<void>
    v8::Local<v8::Value> deleteImportedGpuSemaphore(v8::Local<v8::Value> id);

private:
    v8::Local<v8::Object> OnGetObjectSelf(v8::Isolate *isolate) override;

    std::shared_ptr<gl::PresentRemoteHandle> handle_;
    v8::Global<v8::Object> wrapped_profiler_;
};

//! TSDecl: class CkImageFilter
class CkImageFilterWrap : public ExportableObjectBase,
                          public SkiaObjectWrapper<SkImageFilter>
{
public:
    using SkiaObjectWrapper::SkiaObjectWrapper;
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
    //   ARGUMENT       := '%' IDENT
    //   NULL           := '_'
    //
    // Examples:
    //   blur(3.0, 3.0, %tile, _)
    //   compose(blur(3.0, 3.0, %tile, _), image(%image, %sampling, _, _))

    //! TSDecl: function MakeFromDSL(dsl: string, kwargs: object): CkImageFilter
    static v8::Local<v8::Value> MakeFromDSL(v8::Local<v8::Value> dsl,
                                            v8::Local<v8::Value> kwargs);

    //! TSDecl: function Deserialize(buffer: TypedArray): CkImageFilter
    static v8::Local<v8::Value> Deserialize(v8::Local<v8::Value> buffer);

    //! TSDecl: function serialize(): ArrayBuffer
    v8::Local<v8::Value> serialize();
};

//! TSDecl: class CkColorFilter
class CkColorFilterWrap : public ExportableObjectBase,
                          public SkiaObjectWrapper<SkColorFilter>
{
public:
    using SkiaObjectWrapper::SkiaObjectWrapper;
    ~CkColorFilterWrap() = default;

    //! TSDecl: function MakeFromDSL(dsl: string, kwargs: object): CkColorFilter
    static v8::Local<v8::Value> MakeFromDSL(v8::Local<v8::Value> dsl,
                                            v8::Local<v8::Value> kwargs);

    //! TSDecl: function Deserialize(buffer: TypedArray): CkColorFilter
    static v8::Local<v8::Value> Deserialize(v8::Local<v8::Value> buffer);

    //! TSDecl: function serialize(): ArrayBuffer
    v8::Local<v8::Value> serialize();
};

//! TSDecl: class CkShader
class CkShaderWrap : public ExportableObjectBase,
                     public SkiaObjectWrapper<SkShader>
{
public:
    using SkiaObjectWrapper::SkiaObjectWrapper;

    //! TSDecl: function MakeFromDSL(dsl: string, kwargs: object): CkShader
    static v8::Local<v8::Value> MakeFromDSL(v8::Local<v8::Value> dsl,
                                            v8::Local<v8::Value> kwargs);

    //! TSDecl: function makeWithLocalMatrix(matrix: CkMatrix): CkShader
    v8::Local<v8::Value> makeWithLocalMatrix(v8::Local<v8::Value> matrix);

    //! TSDecl: function makeWithColorFilter(filter: CkColorFilter): CkShader
    v8::Local<v8::Value> makeWithColorFilter(v8::Local<v8::Value> filter);
};

//! TSDecl: class CkBlender
class CkBlenderWrap : public ExportableObjectBase,
                      public SkiaObjectWrapper<SkBlender>
{
public:
    using SkiaObjectWrapper::SkiaObjectWrapper;

    //! TSDecl: function Mode(mode: Enum<BlendMode>): CkBlender
    static v8::Local<v8::Value> Mode(int32_t mode);

    //! TSDecl: function Arithmetic(k1: number, k2: number, k3: number, k4: number,
    //!                             enforcePM: boolean): CkBlender
    static v8::Local<v8::Value> Arithmetic(float k1, float k2, float k3, float k4, bool enforcePM);
};

//! TSDecl: class CriticalPicture
class CriticalPictureWrap : public ExportableObjectBase
{
public:
    explicit CriticalPictureWrap(const gl::MaybeGpuObject<SkPicture>& picture)
        : picture_(picture) {}
    ~CriticalPictureWrap() = default;

    //! TSDecl: function sanitize(): Promise<CkPicture>
    v8::Local<v8::Value> sanitize();

    //! TSDecl: function serialize(): Promise<ArrayBuffer>
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
class CkPictureWrap : public ExportableObjectBase
{
public:
    explicit CkPictureWrap(sk_sp<SkPicture> picture);
    ~CkPictureWrap();

    //! TSDecl: function MakeFromData(buffer: TypedArray): CkPicture
    static v8::Local<v8::Value> MakeFromData(v8::Local<v8::Value> buffer);

    /**
     * Compared with `MakeFromData`, `MakeFromFile` is a fastpath if the caller
     * loads a serialized picture from a file.
     * It will map the corresponding file instead of doing unnecessary copies.
     */

    //! TSDecl: function MakeFromFile(path: string): CkPicture
    static v8::Local<v8::Value> MakeFromFile(const std::string& path);

    g_nodiscard const sk_sp<SkPicture>& getPicture() const;

    //! TSDecl: function serialize(): ArrayBuffer
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
class CkBitmapWrap : public ExportableObjectBase
{
public:
    CkBitmapWrap(std::shared_ptr<v8::BackingStore> backing_store,
                 size_t store_offset,
                 SkBitmap bitmap);
    ~CkBitmapWrap() = default;

    g_nodiscard const SkBitmap& getBitmap() const {
        return bitmap_;
    }

    //! TSDecl: function MakeFromTypedArray(array: Uint8Array,
    //!                                     width: number,
    //!                                     height: number,
    //!                                     stride: number,
    //!                                     colorType: number,
    //!                                     alphaType: number): CkBitmap
    g_nodiscard static v8::Local<v8::Value> MakeFromBuffer(v8::Local<v8::Value> array,
                                                           int32_t width,
                                                           int32_t height,
                                                           int32_t stride,
                                                           uint32_t color_type,
                                                           uint32_t alpha_type);

    //! TSDecl: function MakeFromEncodedFile(path: string): CkBitmap
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

    //! TSDecl: readonly shiftPerPixel: number
    g_nodiscard int32_t getShiftPerPixel();

    //! TSDecl: readonly rowBytes: number
    g_nodiscard size_t getRowBytes();

    //! TSDecl: readonly immutable: boolean
    bool getImmutable() {
        return bitmap_.isImmutable();
    }

    //! TSDecl: function setImmutable(): void
    void setImmutable() {
        bitmap_.setImmutable();
    }

    //! TSDecl: function computeByteSize(): number
    g_nodiscard size_t computeByteSize();

    //! TSDecl: function asImage(): Image
    g_nodiscard v8::Local<v8::Value> asImage();

    //! TSDecl: function makeShader(tmx: Enum<TileMode>, tmy: Enum<TileMode>,
    //!                             sampling: Enum<Sampling>,
    //!                             localMatrix: CkMatrix | null): CkShader
    g_nodiscard v8::Local<v8::Value> makeShader(int32_t tmx, int32_t tmy, int32_t sampling,
                                                v8::Local<v8::Value> local_matrix);

    //! TSDecl: function asTypedArray(): Uint8Array
    g_nodiscard v8::Local<v8::Value> asTypedArray();

private:
    std::shared_ptr<v8::BackingStore> backing_store_;
    size_t      store_offset_;
    SkBitmap    bitmap_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_EXPORTS_H
