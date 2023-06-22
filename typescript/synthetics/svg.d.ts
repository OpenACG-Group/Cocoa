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

import { CkCanvas, CkRect, CkFontMgr } from 'synthetic://glamor';
import { CallbackScopedBuffer } from 'synthetic://core';
import { ResourceProvider } from 'synthetic://resources';

type SVGCanvasFlag = number;
type SVGLengthUnit = number;
type SVGLengthType = number;
type Bitfiled<T> = number;

interface IConstants {
    SVG_CANVAS_FLAG_CONVERT_TEXT_TO_PATHS: SVGCanvasFlag;
    SVG_CANVAS_FLAG_NO_PRETTY_XML: SVGCanvasFlag;
    SVG_CANVAS_FLAG_RELATIVE_PATH_ENCODING: SVGCanvasFlag;

    SVG_LENGTH_UNIT_UNKNOWN: SVGLengthUnit;
    SVG_LENGTH_UNIT_NUMBER: SVGLengthUnit;
    SVG_LENGTH_UNIT_PERCENTAGE: SVGLengthUnit;
    SVG_LENGTH_UNIT_EMS: SVGLengthUnit;
    SVG_LENGTH_UNIT_EXS: SVGLengthUnit;
    SVG_LENGTH_UNIT_PX: SVGLengthUnit;
    SVG_LENGTH_UNIT_CM: SVGLengthUnit;
    SVG_LENGTH_UNIT_MM: SVGLengthUnit;
    SVG_LENGTH_UNIT_IN: SVGLengthUnit;
    SVG_LENGTH_UNIT_PT: SVGLengthUnit;
    SVG_LENGTH_UNIT_PC: SVGLengthUnit;

    SVG_LENGTH_TYPE_VERTICAL: SVGLengthType;
    SVG_LENGTH_TYPE_HORIZONTAL: SVGLengthType;
    SVG_LENGTH_TYPE_OTHER: SVGLengthType;

    // Typically 90
    SVG_LENGTH_DEFAULT_DPI: number;
}

export const Constants: IConstants;

export class SVGCanvas extends CkCanvas {
    private constructor();

    public static Make(bounds: CkRect, writer: (buffer: CallbackScopedBuffer) => void,
                       flags: Bitfiled<SVGCanvasFlag>): SVGCanvas;

    public finish(): void;
}

export class SVGDOMLoader {
    public constructor();
    
    public setFontManager(mgr: CkFontMgr): SVGDOMLoader;
    public setResourceProvider(rp: ResourceProvider): SVGDOMLoader;

    public makeFromString(str: string): SVGDOM;
    public makeFromFile(path: string): SVGDOM;
    public makeFromData(data: Uint8Array): SVGDOM;
}

export interface ISVGLength {
    value: number;
    unit: SVGLengthUnit;
}

export interface ISize {
    width: number;
    height: number;
}

export class SVGDOM {
    public readonly width: ISVGLength;
    public readonly height: ISVGLength;

    private constructor();

    public setContainerSize(width: number, height: number): void;
    public render(canvas: CkCanvas): void;
    public intrinsicSize(context: SVGLengthContext): ISize;
}

export class SVGLengthContext {
    private constructor();

    public static Make(viewport: ISize, dpi: number): SVGLengthContext;

    public readonly viewport: ISize;
    public setViewport(vp: ISize): void;
    public resolve(length: ISVGLength, type: SVGLengthType): number;
    public resolveRect(x: ISVGLength, y: ISVGLength, w: ISVGLength, h: ISVGLength): CkRect;
}
