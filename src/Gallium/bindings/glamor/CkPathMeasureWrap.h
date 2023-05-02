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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_CKPATHMEASUREWRAP_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_CKPATHMEASUREWRAP_H

#include "include/v8.h"
#include "include/core/SkPathMeasure.h"

#include "Gallium/bindings/glamor/Types.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

//! TSDecl: class CkPathMeasure
class CkPathMeasureWrap
{
public:
    CkPathMeasureWrap(const SkPath& path, bool forceClosed, SkScalar resScale);
    ~CkPathMeasureWrap() = default;

    //! TSDecl: function Make(path: CkPath, forceClosed: boolean, resScale: number): CkPathMeasure
    static v8::Local<v8::Value> Make(v8::Local<v8::Value> path,
                                     bool forceClosed, SkScalar resScale);

    //! TSDecl: function setPath(path: CkPath | null, forceClosed: boolean): void
    void setPath(v8::Local<v8::Value> path, bool forceClosed);

    //! TSDecl: function getLength(): number
    SkScalar getLength();

    //! TSDecl: interface PathContourPosTan {
    //!   position: CkPoint;
    //!   tangent: CkPoint;
    //! }

    //! TSDecl: function getPositionTangent(distance: number): PathContourPosTan | null
    v8::Local<v8::Value> getPositionTangent(SkScalar distance);

    //! TSDecl: function getMatrix(distance: number, flags: Bitfield<PathMeasureMatrixFlags>): CkMatrix | null
    v8::Local<v8::Value> getMatrix(SkScalar distance, uint32_t flags);

    //! TSDecl: function getSegment(startD: number, stopD: number, startWithMoveTo: boolean): CkPath | null
    v8::Local<v8::Value> getSegment(SkScalar startD, SkScalar stopD, bool startWithMoveTo);

    //! TSDecl: function isClosed(): boolean
    bool isClosed();

    //! TSDecl: function nextContour(): boolean
    bool nextContour();

private:
    SkPathMeasure measure_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_CKPATHMEASUREWRAP_H
