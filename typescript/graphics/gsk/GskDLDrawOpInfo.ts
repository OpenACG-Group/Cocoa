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

import { Color4f } from '../base/Color';
import { GskSampling } from './GskSampling';
import * as Render from 'renderer';
import * as Fmt from '../../core/formatter';

export enum GskDLDrawOpVerb {
    // <obj@Color4f> color, <i32@BlendMode> mode
    kDrawColor,

    // <obj@Color4f> color
    kClear,

    // <obj@Paint> paint
    kDrawPaint,

    // <i32@PointMode> mode, <obj@Float32Array> pts, <obj@Paint> paint
    kDrawPoints,

    // <f32> x, y, <obj@Paint> paint
    kDrawPoint,

    // <f32> x0, y0, x1, y1, <obj@Paint> paint
    kDrawLine,

    // <obj@Rect> rect, <obj@Paint> paint
    kDrawRect,

    // <obj@Rect> oval, <obj@Paint> paint
    kDrawOval,

    // <obj@RRect> rrect, <obj@Paint> paint
    kDrawRRect,

    // <obj@RRect> outer, inner, <obj@Paint> paint
    kDrawDRRect,

    // <f32> cx, cy, radius, <obj@Paint> paint
    kDrawCircle,

    // <obj@Rect> oval, <f32> startAngleDeg, sweepAngleDeg, <bool> useCenter, <obj@Paint> paint
    kDrawArc,

    // <obj@Rect> rect, <f32> rx, ry, <obj@Paint> paint
    kDrawRoundRect,

    // <obj@Path> path, <obj@Paint> paint
    kDrawPath,

    // <obj@Image> image, <f32> left, top, <obj@GskSampling> sampling, <obj@Paint>? paint
    kDrawImage,

    // <obj@Image> image, <obj@Rect> dst, <obj@GskSampling> sampling, <obj@Paint>? paint
    kDrawImageRect,

    // <obj@Image> image, <obj@Rect> src, dst, <obj@GskSampling> sampling, <obj@Paint>? paint, <bool> strictConstraint
    kDrawImageRectToRect,

    // <obj@Picture> picture, <obj@Mat3x3>? matrix, <obj@Paint>? paint
    kDrawPicture,

    // <obj@Vertices> vertices, <i32@BlendMode> mode, <obj@Paint> paint
    kDrawVertices,

    // <string> text, <f32> x, y, <obj@Font> font, <obj@Paint> paint
    kDrawString,

    // <obj@Uint16Array> glyphs, <obj@RSXformArray> xforms, <f32> originX, originY, <obj@Font> font, <obj@Paint> paint
    kDrawGlyphs,

    // TODO(sora): other ops

    // Internal DrawOps:

    // <i32> index
    kDrawSlice,

    // <i32> index, <obj@DLLayerInfo> info
    kDrawSliceLayer
}

export enum GskDrawOpArgType {
    // Numeric
    kI32 = 'i32',
    kU32 = 'u32',
    kF32 = 'f32',
    // Other primitive types
    kBool = 'bool',
    kString = 'string',
    
    // Enums (implemented as i32)
    kEnum_BlendMode = 'BlendMode',
    kEnum_PointMode = 'PointMode',

    // Objects
    kObj_GskSampling = 'GskSampling',
    kObj_Paint = 'Paint',
    kObj_Rect = 'Rect',
    kObj_RRect = 'RRect',
    kObj_Path = 'Path',
    kObj_Color4f = 'Color4f',
    kObj_Image = 'Image',
    kObj_RSXformArray = 'RSXformArray',
    kObj_Font = 'Font',
    kObj_Picture = 'Picture',
    kObj_Mat3x3 = 'Mat3x3',
    kObj_Vertices = 'Vertices',
    kObj_Float32Array = 'Float32Array',
    kObj_Uint16Array = 'Uint16Array',

    // Internal objects
    kInternal_Obj_DLLayerInfo = '#DLLayerInfo',
    kInternal_I32_ChildIndex = '#ChildIndex[i32]'
}

interface DLDrawOpInfoEntry {
    args: Array<{ name: string, type: GskDrawOpArgType, nullable: boolean }>;
    canvasPrototypeFunc?: Function;
}

const kDLDrawOpInfoMap = new Map<GskDLDrawOpVerb, DLDrawOpInfoEntry>([
    [
        GskDLDrawOpVerb.kDrawColor,
        {
            args: [
                { name: 'color', type: GskDrawOpArgType.kObj_Color4f, nullable: false },
                { name: 'mode', type: GskDrawOpArgType.kEnum_BlendMode, nullable: false }
            ],
            canvasPrototypeFunc: Render.Canvas.prototype.drawColor
        }
    ],
    [
        GskDLDrawOpVerb.kClear,
        {
            args: [{ name: 'color', type: GskDrawOpArgType.kObj_Color4f, nullable: false }],
            canvasPrototypeFunc: Render.Canvas.prototype.clear
        }
    ],
    [
        GskDLDrawOpVerb.kDrawPaint,
        {
            args: [{ name: 'paint', type: GskDrawOpArgType.kObj_Paint, nullable: false }],
            canvasPrototypeFunc: Render.Canvas.prototype.drawPaint
        }
    ],
    [
        GskDLDrawOpVerb.kDrawPoints,
        {
            args: [
                { name: 'mode', type: GskDrawOpArgType.kEnum_PointMode, nullable: false },
                { name: 'pts', type: GskDrawOpArgType.kObj_Float32Array, nullable: false },
                { name: 'paint', type: GskDrawOpArgType.kObj_Paint, nullable: false }
            ],
            canvasPrototypeFunc: Render.Canvas.prototype.drawPoints
        }
    ],
    [
        GskDLDrawOpVerb.kDrawPoint,
        {
            args: [
                { name: 'x', type: GskDrawOpArgType.kF32, nullable: false },
                { name: 'y', type: GskDrawOpArgType.kF32, nullable: false },
                { name: 'paint', type: GskDrawOpArgType.kObj_Paint, nullable: false }
            ],
            canvasPrototypeFunc: Render.Canvas.prototype.drawPoint
        }
    ],
    [
        GskDLDrawOpVerb.kDrawLine,
        {
            args: [
                { name: 'x0', type: GskDrawOpArgType.kF32, nullable: false },
                { name: 'y0', type: GskDrawOpArgType.kF32, nullable: false },
                { name: 'x1', type: GskDrawOpArgType.kF32, nullable: false },
                { name: 'y1', type: GskDrawOpArgType.kF32, nullable: false },
                { name: 'paint', type: GskDrawOpArgType.kObj_Paint, nullable: false }
            ],
            canvasPrototypeFunc: Render.Canvas.prototype.drawLine
        }
    ],
    [
        GskDLDrawOpVerb.kDrawRect,
        {
            args: [
                { name: 'rect', type: GskDrawOpArgType.kObj_Rect, nullable: false },
                { name: 'paint', type: GskDrawOpArgType.kObj_Paint, nullable: false }
            ],
            canvasPrototypeFunc: Render.Canvas.prototype.drawRect
        }
    ],
    [
        GskDLDrawOpVerb.kDrawOval,
        {
            args: [
                { name: 'oval', type: GskDrawOpArgType.kObj_Rect, nullable: false },
                { name: 'paint', type: GskDrawOpArgType.kObj_Paint, nullable: false }
            ],
            canvasPrototypeFunc: Render.Canvas.prototype.drawOval
        }
    ],
    [
        GskDLDrawOpVerb.kDrawRRect,
        {
            args: [
                { name: 'rrect', type: GskDrawOpArgType.kObj_RRect, nullable: false },
                { name: 'paint', type: GskDrawOpArgType.kObj_Paint, nullable: false }
            ],
            canvasPrototypeFunc: Render.Canvas.prototype.drawRRect
        }
    ],
    [
        GskDLDrawOpVerb.kDrawDRRect,
        {
            args: [
                { name: 'outer', type: GskDrawOpArgType.kObj_RRect, nullable: false },
                { name: 'inner', type: GskDrawOpArgType.kObj_RRect, nullable: false },
                { name: 'paint', type: GskDrawOpArgType.kObj_Paint, nullable: false }
            ],
            canvasPrototypeFunc: Render.Canvas.prototype.drawDRRect
        }
    ],
    [
        GskDLDrawOpVerb.kDrawCircle,
        {
            args: [
                { name: 'cx', type: GskDrawOpArgType.kF32, nullable: false },
                { name: 'cy', type: GskDrawOpArgType.kF32, nullable: false },
                { name: 'paint', type: GskDrawOpArgType.kObj_Paint, nullable: false }
            ],
            canvasPrototypeFunc: Render.Canvas.prototype.drawCircle
        }
    ],
    [
        GskDLDrawOpVerb.kDrawArc,
        {
            args: [
                { name: 'oval', type: GskDrawOpArgType.kObj_RRect, nullable: false },
                { name: 'startAngleDeg', type: GskDrawOpArgType.kF32, nullable: false },
                { name: 'sweepAngleDeg', type: GskDrawOpArgType.kF32, nullable: false },
                { name: 'useCenter', type: GskDrawOpArgType.kBool, nullable: false },
                { name: 'paint', type: GskDrawOpArgType.kObj_Paint, nullable: false }
            ],
            canvasPrototypeFunc: Render.Canvas.prototype.drawArc
        }
    ],
    [
        GskDLDrawOpVerb.kDrawRoundRect,
        {
            args: [
                { name: 'rect', type: GskDrawOpArgType.kObj_Rect, nullable: false },
                { name: 'rx', type: GskDrawOpArgType.kF32, nullable: false },
                { name: 'ry', type: GskDrawOpArgType.kF32, nullable: false },
                { name: 'paint', type: GskDrawOpArgType.kObj_Paint, nullable: false }
            ],
            canvasPrototypeFunc: Render.Canvas.prototype.drawRoundRect
        }
    ],
    [
        GskDLDrawOpVerb.kDrawPath,
        {
            args: [
                { name: 'path', type: GskDrawOpArgType.kObj_Path, nullable: false },
                { name: 'paint', type: GskDrawOpArgType.kObj_Paint, nullable: false }
            ],
            canvasPrototypeFunc: Render.Canvas.prototype.drawPath
        }
    ],
    [
        GskDLDrawOpVerb.kDrawImage,
        {
            args: [
                { name: 'image', type: GskDrawOpArgType.kObj_Image, nullable: false },
                { name: 'left', type: GskDrawOpArgType.kF32, nullable: false },
                { name: 'top', type: GskDrawOpArgType.kF32, nullable: false },
                { name: 'sampling', type: GskDrawOpArgType.kObj_GskSampling, nullable: false },
                { name: 'paint', type: GskDrawOpArgType.kObj_Paint, nullable: true }
            ],
            canvasPrototypeFunc: Render.Canvas.prototype.drawImage
        }
    ],
    [
        GskDLDrawOpVerb.kDrawImageRect,
        {
            args: [
                { name: 'image', type: GskDrawOpArgType.kObj_Image, nullable: false },
                { name: 'dst', type: GskDrawOpArgType.kObj_Rect, nullable: false },
                { name: 'sampling', type: GskDrawOpArgType.kObj_GskSampling, nullable: false },
                { name: 'paint', type: GskDrawOpArgType.kObj_Paint, nullable: true }
            ],
            canvasPrototypeFunc: Render.Canvas.prototype.drawImageRect
        }
    ],
    [
        GskDLDrawOpVerb.kDrawImageRectToRect,
        {
            args: [
                { name: 'image', type: GskDrawOpArgType.kObj_Image, nullable: false },
                { name: 'src', type: GskDrawOpArgType.kObj_Rect, nullable: false },
                { name: 'dst', type: GskDrawOpArgType.kObj_Rect, nullable: false },
                { name: 'sampling', type: GskDrawOpArgType.kObj_GskSampling, nullable: false },
                { name: 'paint', type: GskDrawOpArgType.kObj_Paint, nullable: true },
                { name: 'strictConstraint', type: GskDrawOpArgType.kBool, nullable: false }
            ],
            canvasPrototypeFunc: Render.Canvas.prototype.drawImageRectToRect
        }
    ],
    [
        GskDLDrawOpVerb.kDrawPicture,
        {
            args: [
                { name: 'picture', type: GskDrawOpArgType.kObj_Picture, nullable: false },
                { name: 'matrix', type: GskDrawOpArgType.kObj_Mat3x3, nullable: true },
                { name: 'paint', type: GskDrawOpArgType.kObj_Paint, nullable: true }
            ],
            canvasPrototypeFunc: Render.Canvas.prototype.drawPicture
        }
    ],
    [
        GskDLDrawOpVerb.kDrawVertices,
        {
            args: [
                { name: 'vertices', type: GskDrawOpArgType.kObj_Vertices, nullable: false },
                { name: 'mode', type: GskDrawOpArgType.kEnum_BlendMode, nullable: false },
                { name: 'paint', type: GskDrawOpArgType.kObj_Paint, nullable: false }
            ],
            canvasPrototypeFunc: Render.Canvas.prototype.drawVertices
        }
    ],
    [
        GskDLDrawOpVerb.kDrawString,
        {
            args: [
                { name: 'text', type: GskDrawOpArgType.kString, nullable: false },
                { name: 'x', type: GskDrawOpArgType.kF32, nullable: false },
                { name: 'y', type: GskDrawOpArgType.kF32, nullable: false },
                { name: 'font', type: GskDrawOpArgType.kObj_Font, nullable: false },
                { name: 'paint', type: GskDrawOpArgType.kObj_Paint, nullable: false }
            ],
            canvasPrototypeFunc: Render.Canvas.prototype.drawString
        }
    ],
    [
        GskDLDrawOpVerb.kDrawGlyphs,
        {
            args: [
                { name: 'glyphs', type: GskDrawOpArgType.kObj_Uint16Array, nullable: false },
                { name: 'xforms', type: GskDrawOpArgType.kObj_RSXformArray, nullable: false },
                { name: 'originX', type: GskDrawOpArgType.kF32, nullable: false },
                { name: 'originY', type: GskDrawOpArgType.kF32, nullable: false },
                { name: 'font', type: GskDrawOpArgType.kObj_Font, nullable: false },
                { name: 'paint', type: GskDrawOpArgType.kObj_Paint, nullable: false }
            ],
            canvasPrototypeFunc: Render.Canvas.prototype.drawGlyphs
        }
    ],
    [
        GskDLDrawOpVerb.kDrawSlice,
        {
            args: [{ name: 'index', type: GskDrawOpArgType.kInternal_I32_ChildIndex, nullable: false }]
        }
    ],
    [
        GskDLDrawOpVerb.kDrawSliceLayer,
        {
            args: [
                { name: 'index', type: GskDrawOpArgType.kInternal_I32_ChildIndex, nullable: false },
                { name: 'info', type: GskDrawOpArgType.kInternal_Obj_DLLayerInfo, nullable: false }
            ]
        }
    ]
]);

const kBlendModeReverseMap = new Map<Render.BlendMode, string>();
for (const entryName of Object.getOwnPropertyNames(Render.BlendMode)) {
    kBlendModeReverseMap.set(Render.BlendMode[entryName], entryName);
}


export namespace GskDLDrawOpInspect {

    export interface DrawOp {
        startIndex: number;
        verb: GskDLDrawOpVerb;
        args: Array<unknown>;
        reflection: DLDrawOpInfoEntry;
    }

    export function GetName(op: GskDLDrawOpVerb): string {
        // To strip the leading character 'k'
        return GskDLDrawOpVerb[op].slice(1);
    }

    export function* Iterate(buffer: Array<unknown>): Generator<DrawOp> {
        let currentDrawOp: DrawOp = null;
        let i = 0;
        while (i < buffer.length) {
            if (currentDrawOp == null) {
                const verb = buffer[i] as GskDLDrawOpVerb;
                const reflection = kDLDrawOpInfoMap.get(verb);
                if (reflection == null) {
                    throw Error(`unexpected DrawOp verb: ${verb}`);
                }
                currentDrawOp = {
                    startIndex: i,
                    verb: verb,
                    args: [],
                    reflection: reflection
                };
                // Lookahead
                i++;
                continue;
            }

            if (currentDrawOp.reflection.args.length > 0) {
                currentDrawOp.args.push(buffer[i]);
            }
            if (currentDrawOp.reflection.args.length == currentDrawOp.args.length) {
                // A complete DrawOp has been read
                yield currentDrawOp;
                currentDrawOp = null;
            }
            i++;
        }

        if (currentDrawOp != null) {
            throw Error('incomplete DrawOp at the end buffer');
        }
    }

    export function FormatMat3x3(mat: Render.Mat3x3, ctx: Fmt.FormatterContext): Array<Fmt.TextBlock> {
        const arr = [
            [ mat.at(0, 0), mat.at(0, 1), mat.at(0, 2) ],
            [ mat.at(1, 0), mat.at(1, 1), mat.at(1, 2) ],
            [ mat.at(2, 0), mat.at(2, 1), mat.at(2, 2) ],
        ];
        return [
            Fmt.TB(Fmt.TextBlockLayoutHint.kPrefix, [Fmt.TAG('Mat3x3', Fmt.TextColor.kGreen)]),
            ...Fmt.formatAnyValue(arr, ctx)
        ];
    }

    export function StringifyBlendMode(mode: Render.BlendMode): string {
        const value = kBlendModeReverseMap.get(mode);
        if (value == null) {
            throw TypeError('invalid value for enumeration `BlendMode`');
        }
        return value;
    }
}
