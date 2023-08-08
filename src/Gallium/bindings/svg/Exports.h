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

#ifndef COCOA_GALLIUM_BINDINGS_SVG_EXPORTS_H
#define COCOA_GALLIUM_BINDINGS_SVG_EXPORTS_H

#include <memory>

#include "include/v8.h"
#include "include/core/SkStream.h"
#include "modules/svg/include/SkSVGNode.h"
#include "modules/svg/include/SkSVGDOM.h"
#include "modules/svg/include/SkSVGRenderContext.h"

#include "Gallium/bindings/glamor/CkCanvasWrap.h"

#define GALLIUM_BINDINGS_SVG_NS_BEGIN namespace cocoa::gallium::bindings::svg_wrap {
#define GALLIUM_BINDINGS_SVG_NS_END   }

GALLIUM_BINDINGS_SVG_NS_BEGIN

void SetInstanceProperties(v8::Local<v8::Object> instance);

class SVGJSWStreamImpl : public SkWStream
{
public:
    SVGJSWStreamImpl(v8::Isolate *isolate, v8::Local<v8::Function> func)
        : isolate_(isolate), func_(isolate, func), bytes_written_(0) {}

    bool write(const void *buffer, size_t size) override;
    void flush() override;
    g_nodiscard size_t bytesWritten() const override;

private:
    v8::Isolate *isolate_;
    v8::Global<v8::Function> func_;
    size_t bytes_written_;
};

//! TSDecl: class SVGCanvas
class SVGCanvasWrap : public glamor_wrap::CkCanvas
{
public:
    SVGCanvasWrap(std::unique_ptr<SkCanvas> canvas,
                  std::unique_ptr<SVGJSWStreamImpl> stream)
        : glamor_wrap::CkCanvas(canvas.get())
        , canvas_(std::move(canvas)), stream_(std::move(stream)) {}

    //! TSDecl: function Make(bounds: Glamor.CkRect,
    //!                       writer: (buffer: core.CallbackScopedBuffer) => void,
    //!                       flags: Bitfield<SVGCanvasFlags>): Glamor.CkCanvas
    static v8::Local<v8::Value> Make(v8::Local<v8::Value> bounds,
                                     v8::Local<v8::Value> writer,
                                     uint32_t flags);

    //! TSDecl: function finish(): void
    void finish();

private:
    std::unique_ptr<SkCanvas> canvas_;
    std::unique_ptr<SVGJSWStreamImpl> stream_;
};

//! TSDecl: class SVGDOMLoader
class SVGDOMLoaderWrap : public ExportableObjectBase
{
public:
    SVGDOMLoaderWrap() = default;
    ~SVGDOMLoaderWrap() = default;

    //! TSDecl: function setFontManager(mgr: glamor.CkFontMgr): SVGDOMLoader
    v8::Local<v8::Value> setFontManager(v8::Local<v8::Value> mgr);

    //! TSDecl: function setResourceProvider(rp: resources.ResourceProvider): SVGDOMLoader
    v8::Local<v8::Value> setResourceProvider(v8::Local<v8::Value> rp);

    //! TSDecl: function makeFromFile(path: string): SVGDOM
    v8::Local<v8::Value> makeFromFile(const std::string& path);

    //! TSDecl: function makeFromData(data: Uint8Array): SVGDOM
    v8::Local<v8::Value> makeFromData(v8::Local<v8::Value> data);

    //! TSDecl: function makeFromString(str: string): SVGDOM
    v8::Local<v8::Value> makeFromString(v8::Local<v8::Value> str);

private:
    v8::Local<v8::Object> ReturnThis();

    SkSVGDOM::Builder builder_;
};

//! TSDecl: class SVGDOM
class SVGDOMWrap : public ExportableObjectBase
{
public:
    explicit SVGDOMWrap(sk_sp<SkSVGDOM> dom) : dom_(std::move(dom)) {}
    ~SVGDOMWrap() = default;

    //! TSDecl: function setContainerSize(width: number, height: number): void
    void setContainerSize(SkScalar width, SkScalar height);

    //! TSDecl: function render(canvas: glamor.CkCanvas): void
    void render(v8::Local<v8::Value> canvas);

    //! TSDecl: function intrinsicSize(ctx: SVGLengthContext): ISize
    v8::Local<v8::Value> intrinsicSize(v8::Local<v8::Value> ctx);

    //! TSDecl: readonly width: ISVGLength
    v8::Local<v8::Value> width();

    //! TSDecl: readonly height: ISVGLength
    v8::Local<v8::Value> height();

private:
    sk_sp<SkSVGDOM> dom_;
};

//! TSDecl: class SVGLengthContext
class SVGLengthContextWrap : public ExportableObjectBase
{
public:
    SVGLengthContextWrap(const SkSize& viewport, SkScalar dpi)
        : ctx_(viewport, dpi) {}
    ~SVGLengthContextWrap() = default;

    //! TSDecl: function Make(vp: ISize, dpi: number): SVGLengthContext
    static v8::Local<v8::Value> Make(v8::Local<v8::Value> vp, SkScalar dpi);

    g_nodiscard g_inline const SkSVGLengthContext& GetContext() const {
        return ctx_;
    }

    //! TSDecl: readonly viewport: ISize
    v8::Local<v8::Value> viewport();

    //! TSDecl: function setViewport(vp: ISize): void
    void setViewport(v8::Local<v8::Value> vp);

    //! TSDecl: function resolve(length: ISVGLength, type: Enum<SVGLengthType>): number
    SkScalar resolve(v8::Local<v8::Value> length, int32_t type);

    //! TSDecl: function resolveRect(x: ISVGLength, y: ISVGLength,
    //!                              w: ISVGLength, h: ISVGLength): glamor.CkRect
    v8::Local<v8::Value> resolveRect(v8::Local<v8::Value> x, v8::Local<v8::Value> y,
                                     v8::Local<v8::Value> w, v8::Local<v8::Value> h);

private:
    SkSVGLengthContext  ctx_;
};

GALLIUM_BINDINGS_SVG_NS_END
#endif //COCOA_GALLIUM_BINDINGS_SVG_EXPORTS_H
