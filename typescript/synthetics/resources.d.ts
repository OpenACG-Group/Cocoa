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

import * as GL from 'synthetic://glamor';

export type CRPKGSourceType = number;

interface IConstants {
    CRPKG_SOURCE_TYPE_UINT8ARRAY: CRPKGSourceType,
    CRPKG_SOURCE_TYPE_CRPKG_STORAGE: CRPKGSourceType,
    CRPKG_SOURCE_TYPE_FILEPATH: CRPKGSourceType
}

export const Constants: IConstants;

export interface IExternalTrackAssetImpl {
    seek(t: number): void;
}

export class ExternalTrackAsset {
    private constructor();

    public static MakeImpl(impl: IExternalTrackAssetImpl): ExternalTrackAsset;
}

export interface ImageAssetFrameData {
    image: GL.CkImage;
    sampling: GL.SamplingOption;
    matrix: GL.CkMatrix;
    scaling: GL.MatrixScaleToFit;
}

export interface IImageAssetImpl {
    isMultiFrame(): boolean;
    getFrameData(t: number): ImageAssetFrameData;
}

export class ImageAsset {
    private constructor();

    public static MakeMultiFrame(data: Uint8Array, predecode: boolean): ImageAsset;
    public static MakeImpl(impl: IImageAssetImpl): ImageAsset;
}

export interface IResourceProvider {
    load(path: string, name: string): Uint8Array;
    loadImageAsset(path: string, name: string, id: string): ImageAsset;
    loadAudioAsset(path: string, name: string, id: string): ExternalTrackAsset;
    loadTypeface(name: string, url: string): GL.CkTypeface;
}

export interface IResourceProviderProxy {
    load?(path: string, name: string): Uint8Array;
    loadImageAsset?(path: string, name: string, id: string): ImageAsset;
    loadAudioAsset?(path: string, name: string, id: string): ExternalTrackAsset;
    loadTypeface?(name: string, url: string): GL.CkTypeface;
}

export class ResourceProvider {
    private constructor();

    public static MakeFile(baseDir: string, predecode: boolean): ResourceProvider;
    public static MakeImpl(impl: IResourceProvider): ResourceProvider;
    public static MakeCachingProxy(rp: ResourceProvider): ResourceProvider;
    public static MakeDataURIProxy(rp: ResourceProvider, predecode: boolean): ResourceProvider;
    public static MakeProxyImpl(impl: IResourceProviderProxy): ResourceProvider;
}

export class CRPKGStorage {
    public readonly byteLength: number;

    private constructor();

    public read(srcOffset: number, dst: Uint8Array, dstOffset: number, size: number): Promise<number>;
    public readSync(srcOffset: number, dst: Uint8Array, dstOffset: number, size: number): number;
    public unref(): void;
}

export interface CRPKGSource {
    type: CRPKGSourceType;
    source: Uint8Array | CRPKGStorage | string;
}

export class CRPKGVirtualDisk {
    private constructor();

    public static MakeLayers(layers: Array<CRPKGSource>): CRPKGVirtualDisk;

    public resolve(path: string): CRPKGStorage | null;
    public unref(): void;
}
