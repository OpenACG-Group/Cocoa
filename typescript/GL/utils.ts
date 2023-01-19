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

import * as std from 'synthetic://core';
import * as GL from 'synthetic://glamor';
import * as CanvasKit from 'internal://canvaskit';

const canvaskit = CanvasKit.canvaskit;

export interface AppInfo {
    name: string;
    major?: number;
    minor?: number;
    patch?: number;
}

export function Initialize(appInfo?: AppInfo): void {
    const initAppInfo: GL.ApplicationInfo = {
        name: 'Default Application',
        major: 0,
        minor: 0,
        patch: 0
    };
    if (appInfo) {
        initAppInfo.name = appInfo.name;
        initAppInfo.major = appInfo.major ? appInfo.major : 0;
        initAppInfo.minor = appInfo.minor ? appInfo.minor : 0;
        initAppInfo.patch = appInfo.patch ? appInfo.patch : 0;
    }

    GL.RenderHost.Initialize(initAppInfo);
    canvaskit.bindWithInitializedRenderHost(GL.RenderHost);
}

export type DrawFunction = (canvas: CanvasKit.Canvas) => void;
export function DrawToCkPicture(onDraw: DrawFunction, bounds: CanvasKit.InputRect): GL.CkPicture {
    const recorder = new canvaskit.PictureRecorder();
    const canvas = recorder.beginRecording(bounds);

    try {
        onDraw(canvas);
    } catch (except) {
        recorder.delete();
        throw except;
    }

    const pictureData = recorder.finishRecordingAsPicture()?.serializeForTransfer();
    recorder.delete();
    if (!pictureData) {
        throw Error('Failed to finish canvas recording');
    }

    return GL.CkPicture.MakeFromData(std.Buffer.MakeFromAdoptBuffer(pictureData),
                                     GL.CkPicture.USAGE_TRANSFER);
}

export function DrawToSkPicture(onDraw: DrawFunction, bounds: CanvasKit.InputRect): CanvasKit.SkPicture {
    const recorder = new canvaskit.PictureRecorder();
    const canvas = recorder.beginRecording(bounds);
    try {
        onDraw(canvas);
    } catch (except) {
        recorder.delete();
        throw except;
    }
    const picture = recorder.finishRecordingAsPicture();
    recorder.delete();
    return picture;
}

export function LoadTypefaceFromBuffer(data: std.Buffer): CanvasKit.Typeface {
    const typeface = canvaskit.Typeface.MakeFreeTypeFaceFromData(data.byteArray.buffer);
    if (!typeface) {
        throw Error('Failed to load typeface from specified data');
    }
    canvaskit.registerRenderableTypeface(typeface);
    return typeface;
}

export async function LoadTypefaceFromFile(file: string): Promise<CanvasKit.Typeface> {
    return LoadTypefaceFromBuffer(await std.Buffer.MakeFromFile(file));
}

export function LoadTypefaceFromFileSync(file: string): CanvasKit.Typeface {
    return LoadTypefaceFromBuffer(std.File.ReadFileSync(file));
}

const kCkSkColorTypeMap = [
    [ canvaskit.ColorType.BGRA_8888, GL.Constants.COLOR_TYPE_BGRA8888 ],
    [ canvaskit.ColorType.RGBA_8888, GL.Constants.COLOR_TYPE_RGBA8888 ],
    [ canvaskit.ColorType.RGBA_1010102, GL.Constants.COLOR_TYPE_RGBA1010102 ],
    [ canvaskit.ColorType.RGBA_F16, GL.Constants.COLOR_TYPE_RGBA_F16 ],
    [ canvaskit.ColorType.RGBA_F32, GL.Constants.COLOR_TYPE_RGBA_F32 ],
    [ canvaskit.ColorType.RGB_101010x, GL.Constants.COLOR_TYPE_RGB101010x ],
    [ canvaskit.ColorType.RGB_565, GL.Constants.COLOR_TYPE_RGB565 ],
    [ canvaskit.ColorType.Alpha_8, GL.Constants.COLOR_TYPE_ALPHA8 ],
    [ canvaskit.ColorType.Gray_8, GL.Constants.COLOR_TYPE_GRAY8 ]
];

const kCkSkAlphaTypeMap = [
    [ canvaskit.AlphaType.Opaque, GL.Constants.ALPHA_TYPE_OPAQUE ],
    [ canvaskit.AlphaType.Premul, GL.Constants.ALPHA_TYPE_PREMULTIPLIED ],
    [ canvaskit.AlphaType.Unpremul, GL.Constants.ALPHA_TYPE_UNPREMULTIPLIED ]
];

export function CkToSkColorType(ct: GL.ColorType): CanvasKit.ColorType {
    for (let entry of kCkSkColorTypeMap) {
        if (entry[1] == ct) {
            return entry[0] as CanvasKit.ColorType;
        }
    }
    throw Error('Unsupported color type by CanvasKit');
}

export function SkToCkColorType(ct: CanvasKit.ColorType): GL.ColorType {
    for (let entry of kCkSkColorTypeMap) {
        if (entry[0] == ct) {
            return entry[1] as GL.ColorType;
        }
    }
    throw Error('Unsupported color type by Glamor');
}

export function CkToSkAlphaType(at: GL.AlphaType): CanvasKit.AlphaType {
    for (let entry of kCkSkAlphaTypeMap) {
        if (entry[1] == at) {
            return entry[0] as CanvasKit.AlphaType;
        }
    }
    throw Error('Unsupported alpha type by CanvasKit');
}

export function SkToCkAlphaType(at: CanvasKit.AlphaType): GL.AlphaType {
    for (let entry of kCkSkAlphaTypeMap) {
        if (entry[0] == at) {
            return entry[1] as GL.AlphaType;
        }
    }
    throw Error('Unsupported alpha type by Glamor');
}

export function CopyCkImageToSkImage(image: GL.CkImage): CanvasKit.Image {
    const pixmap = image.makeSharedPixelsBuffer();
    return canvaskit.MakeImage({
        width: pixmap.width,
        height: pixmap.height,
        colorType: CkToSkColorType(pixmap.colorType),
        alphaType: CkToSkAlphaType(pixmap.alphaType),
        colorSpace: canvaskit.ColorSpace.SRGB
    }, pixmap.buffer.byteArray, pixmap.rowBytes);
}
