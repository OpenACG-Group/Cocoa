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

import { ResourceProvider } from 'synthetic://resources';
import { CkFontMgr, CkCanvas, CkRect } from 'synthetic://glamor';

type Enum<T> = number;
type AnimationBuilderFlags = number;
type AnimationRenderFlag = number;
type LoggerLevel = number;

type Bitfield<T> = number;

interface IConstants {
    ANIMATION_BUILDER_FLAGS_DEFER_IMAGE_LOADING: AnimationBuilderFlags;
    ANIMATION_BUILDER_FLAGS_PREFER_EMBEDDED_FONTS: AnimationBuilderFlags;

    ANIMATION_RENDER_FLAG_SKIP_TOP_LEVEL_ISOLATION: AnimationRenderFlag;
    ANIMATION_RENDER_FLAG_DISABLE_TOP_LEVEL_CLIPPING: AnimationRenderFlag;

    LOGGER_LEVEL_WARNING: LoggerLevel;
    LOGGER_LEVEL_ERROR: LoggerLevel;
}

export const Constants: IConstants;

type Logger = (level: Enum<LoggerLevel>, message: string, json: string | null) => void;
type MarkerObserver = (name: string, t0: number, t1: number) => void;
type ExternalLayerRenderFunc = (canvas: CkCanvas, t: number) => void;
type PrecompInterceptorFunc = (id: string, name: string,
                               width: number, height: number) => ExternalLayerRenderFunc;

export interface IExpressionManager {
    createNumberExpressionEvaluator(expr: string): (t: number) => number;
    createStringExpressionEvaluator(expr: string): (t: number) => string;
    createArrayExpressionEvaluator(expr: string): (t: number) => Array<number>;
}

export class AnimationBuilder {
    public constructor(flags: Bitfield<AnimationBuilderFlags>);

    public setResourceProvider(rp: ResourceProvider): AnimationBuilder;
    public setFontManager(manager: CkFontMgr): AnimationBuilder;
    public setLogger(func: Logger): AnimationBuilder;
    public setMarkerObserver(func: MarkerObserver): AnimationBuilder;
    public setPrecompInterceptor(func: PrecompInterceptorFunc): AnimationBuilder;
    public setExpressionManager(manager: IExpressionManager): AnimationBuilder;
    public make(json: string): Animation;
    public makeFromFile(path: string): Animation;
}

export class Animation {
    public readonly duration: number;
    public readonly fps: number;
    public readonly inPoint: number;
    public readonly outPoint: number;

    private constructor();

    public render(canvas: CkCanvas, dst: CkRect | null, flags: Bitfield<AnimationRenderFlag>): void;
    public seekFrame(t: number): void;
    public seekFrameTime(t: number): void;
}
