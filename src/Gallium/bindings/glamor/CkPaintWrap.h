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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_CKPAINTWRAP_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_CKPAINTWRAP_H

#include "include/v8.h"
#include "include/core/SkPaint.h"

#include "Gallium/bindings/glamor/Types.h"
#include "Gallium/bindings/ExportableObjectBase.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

class CkPaint : public ExportableObjectBase
{
public:
    explicit CkPaint(SkPaint paint) : paint_(std::move(paint)) {}
    ~CkPaint() = default;

    g_nodiscard g_inline SkPaint& GetPaint() {
        return paint_;
    }

    //! TSDecl: constructor()
    CkPaint() = default;

    //! TSDecl: function reset(): void
    void reset();

    //! TSDecl: function setAntiAlias(AA: boolean): void
    void setAntiAlias(bool AA);

    //! TSDecl: function setDither(dither: boolean): void
    void setDither(bool dither);

    //! TSDecl: function setStyle(style: Enum<PaintStyle>): void
    void setStyle(int32_t style);

    //! TSDecl: function setColor(color: number): void
    void setColor(uint32_t color);

    //! TSDecl: function setColor4f(color: Array<number>): void
    void setColor4f(v8::Local<v8::Value> color);

    //! TSDecl: function setAlpha(alpha: number): void
    void setAlpha(uint32_t alpha);

    //! TSDecl: function setAlphaf(alpha: number): void
    void setAlphaf(float alpha);

    //! TSDecl: function setStrokeWidth(width: number): void
    void setStrokeWidth(SkScalar width);

    //! TSDecl: function setStrokeMiter(miter: number): void
    void setStrokeMiter(SkScalar miter);

    //! TSDecl: function setStrokeCap(cap: Enum<PaintCap>): void
    void setStrokeCap(int32_t cap);

    //! TSDecl: function setStrokeJoin(join: Enum<PaintJoin>): void
    void setStrokeJoin(int32_t join);

    //! TSDecl: function setShader(shader: CkShader): void
    void setShader(v8::Local<v8::Value> shader);

    //! TSDecl: function setColorFilter(filter: CkColorFilter): void
    void setColorFilter(v8::Local<v8::Value> filter);

    //! TSDecl: function setBlendMode(mode: Enum<BlendMode>): void
    void setBlendMode(int32_t mode);

    //! TSDecl: function setBlender(blender: CkBlender): void
    void setBlender(v8::Local<v8::Value> blender);

    //! TSDecl: function setPathEffect(effect: CkPathEffect): void
    void setPathEffect(v8::Local<v8::Value> effect);

    //! TSDecl: function setImageFilter(filter: CkImageFilter): void
    void setImageFilter(v8::Local<v8::Value> filter);

private:
    SkPaint paint_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_CKPAINTWRAP_H
