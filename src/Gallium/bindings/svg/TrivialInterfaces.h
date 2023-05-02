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

#ifndef COCOA_GALLIUM_BINDINGS_SVG_TRIVIALINTERFACES_H
#define COCOA_GALLIUM_BINDINGS_SVG_TRIVIALINTERFACES_H

#include "modules/svg/include/SkSVGTypes.h"

#include "Gallium/bindings/svg/Exports.h"
GALLIUM_BINDINGS_SVG_NS_BEGIN

//! TSDecl: interface SVGLength {
//!   value: number;
//!   unit: Enum<SVGLengthUnit>;
//! }
class ISVGLength
{
public:
    static v8::Local<v8::Object> New(v8::Isolate *isolate, const SkSVGLength& from);
    static SkSVGLength Extract(v8::Isolate *isolate, v8::Local<v8::Value> from);
};

//! TSDecl: interface ISize {
//!   width: number;
//!   height: number;
//! }
class ISize
{
public:
    static v8::Local<v8::Object> New(v8::Isolate *isolate, const SkSize& from);
    static SkSize Extract(v8::Isolate *isolate, v8::Local<v8::Value> from);
};

GALLIUM_BINDINGS_SVG_NS_END
#endif //COCOA_GALLIUM_BINDINGS_SVG_TRIVIALINTERFACES_H
