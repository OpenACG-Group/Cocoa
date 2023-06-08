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

import { CkImage, CkImageInfo } from 'synthetic://glamor';

type PNGEncoderFilterFlag = number;
type JPEGEncoderAlphaOption = number;
type JPEGEncoderDownsample = number;
type WebpEncoderCompression = number;

interface IConstants {
    PNG_ENCODER_FILTER_FLAG_ZERO: PNGEncoderFilterFlag;
    PNG_ENCODER_FILTER_FLAG_NONE: PNGEncoderFilterFlag;
    PNG_ENCODER_FILTER_FLAG_SUB: PNGEncoderFilterFlag;
    PNG_ENCODER_FILTER_FLAG_UP: PNGEncoderFilterFlag;
    PNG_ENCODER_FILTER_FLAG_AVG: PNGEncoderFilterFlag;
    PNG_ENCODER_FILTER_FLAG_PAETH: PNGEncoderFilterFlag;
    PNG_ENCODER_FILTER_FLAG_ALL: PNGEncoderFilterFlag;

    JPEG_ENCODER_ALPHA_OPTION_IGNORE: JPEGEncoderAlphaOption;
    JPEG_ENCODER_ALPHA_OPTION_BLEND_ON_BLACK: JPEGEncoderAlphaOption;

    JPEG_ENCODER_DOWNSAMPLE_K420: JPEGEncoderDownsample;
    JPEG_ENCODER_DOWNSAMPLE_K422: JPEGEncoderDownsample;
    JPEG_ENCODER_DOWNSAMPLE_K444: JPEGEncoderDownsample;

    WEBP_ENCODER_COMPRESSION_LOSSY: WebpEncoderCompression;
    WEBP_ENCODER_COMPRESSION_LOSSLESS: WebpEncoderCompression;
}

export const Constants: IConstants;

export interface PNGEncoderOptions {
    filterFlags?: PNGEncoderFilterFlag;
    zlibLevel?: number;
    comments?: Array<Uint8Array>;
}

export class PNGEncoder {
    public static EncodeImage(img: CkImage,
                              options: PNGEncoderOptions): ArrayBuffer | null;

    public static EncodeMemory(info: CkImageInfo,
                               pixels: Uint8Array,
                               rowBytes: number,
                               options: PNGEncoderOptions): ArrayBuffer | null;
}

export interface JPEGEncoderOptions {
    quality?: number;
    downsample?: JPEGEncoderDownsample;
    alphaOption?: JPEGEncoderAlphaOption;
    xmpMetadata?: Uint8Array;
}

export class JPEGEncoder {
    public static EncodeImage(img: CkImage,
                              options: JPEGEncoderOptions): ArrayBuffer | null;

    public static EncodeMemory(info: CkImageInfo,
                               pixels: Uint8Array,
                               rowBytes: number,
                               options: JPEGEncoderOptions): ArrayBuffer | null;
}

export interface WebpEncoderOptions {
    compression?: WebpEncoderCompression;
    quality?: number;
}

export interface WebpImageFrame {
    image: CkImage;
    duration: number;
}

export interface WebpMemoryFrame {
    info: CkImageInfo;
    pixels: Uint8Array;
    rowBytes: number;
    duration: number;
}

export class WebpEncoder {
    public static EncodeImage(img: CkImage,
                              options: WebpEncoderOptions): ArrayBuffer | null;

    public static EncodeMemory(info: CkImageInfo,
                               pixels: Uint8Array,
                               rowBytes: number,
                               options: WebpEncoderOptions): ArrayBuffer | null;

    public static EncodeAnimatedImage(frames: WebpImageFrame,
                                      options: WebpEncoderOptions): ArrayBuffer | null;

    public static EncodeAnimatedMemory(frames: WebpMemoryFrame,
                                       options: WebpEncoderOptions): ArrayBuffer | number;
}
