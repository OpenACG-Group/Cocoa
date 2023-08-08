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

import {
    CkImageFilter,
    CkPath,
    BlendMode,
    Constants as GLConst,
    CkPathMeasure
} from 'glamor';

import { Rect, RRect } from './rectangle';
import { Vector2f } from './vector';
import { Recorder } from './render-node-linearization';

enum ClippingType {
    kRect,
    kRRect,
    kPath,
    kNone
}

export class CompositeAttributes {
    private fImageFilter: CkImageFilter | null;
    private fBackdropFilter: CkImageFilter | null;
    private fBackdropFilterBlendMode: BlendMode;
    private fBackdropFilterBoundsClip: boolean;
    private fOpacity: number | null;
    private fClippingType: ClippingType;
    private fRectClip: Rect | null;
    private fRRectClip: RRect | null;
    private fClippingAA: boolean;
    private fPathClip: CkPath | null;
    private fOffset: Vector2f;
    private fRotate: { rad: number, pivot: Vector2f } | null;

    public constructor() {
        this.resetAll();
    }

    public hasTransform(): boolean {
        return this.hasOffset() || this.hasRotate();
    }

    public hasOffset(): boolean {
        return (this.fOffset.x != 0 || this.fOffset.y != 0);
    }

    public hasRotate(): boolean {
        return (this.fRotate != null && this.fRotate.rad != 0);
    }

    public hasClipping(): boolean {
        return (this.fClippingType != ClippingType.kNone);
    }

    public hasBackdropFilter(): boolean {
        return (this.fBackdropFilter != null);
    }

    public hasImageFilter(): boolean {
        return (this.fImageFilter != null);
    }

    public hasOpacity(): boolean {
        return (this.fOpacity != null);
    }

    public setImageFilter(filter: CkImageFilter): void {
        this.fImageFilter = filter;
    }

    public setBackdropFilter(filter: CkImageFilter, mode: BlendMode, clip: boolean): void {
        this.fBackdropFilter = filter;
        this.fBackdropFilterBlendMode = mode;
        this.fBackdropFilterBoundsClip = clip;
    }

    public setRectClip(clip: Rect): void {
        this.fClippingType = ClippingType.kRect;
        this.fRectClip = clip;
    }

    public setRRectClip(clip: RRect): void {
        this.fClippingType = ClippingType.kRRect;
        this.fRRectClip = clip;
    }

    public setPathClip(clip: CkPath): void {
        if (clip.isEmpty()) {
            return;
        }

        const measure = CkPathMeasure.Make(clip, false, 1);
        do {
            if (!measure.isClosed()) {
                throw Error('All the contours in the clip path must be closed');
            }
        } while (measure.nextContour());

        this.fClippingType = ClippingType.kPath;
        this.fPathClip = clip;
    }

    public setOffset(offset: Vector2f): void {
        this.fOffset = offset;
    }

    public setRotate(rad: number, pivot: Vector2f): void {
        this.fRotate = {rad: rad, pivot: pivot};
    }

    public setOpacity(opacity: number): void {
        if (opacity < 0) {
            throw RangeError('Invalid opacity value');
        }
        this.fOpacity = opacity;
    }

    public setClippingAntiAlias(aa: boolean): void {
        this.fClippingAA = aa;
    }

    public resetAll(): void {
        this.fImageFilter = null;
        this.fBackdropFilter = null;
        this.fBackdropFilterBlendMode = GLConst.BLEND_MODE_SRC_OVER;
        this.fBackdropFilterBoundsClip = true;
        this.fOpacity = null;
        this.fClippingType = ClippingType.kNone;
        this.fClippingAA = false;
        this.fRectClip = null;
        this.fRRectClip = null;
        this.fPathClip = null;
        this.fOffset = new Vector2f(0, 0);
        this.fRotate = null;
    }

    public resetImageFilter(): void {
        this.fImageFilter = null;
    }

    public resetBackdropFilter(): void {
        this.fBackdropFilter = null;
        this.fBackdropFilterBlendMode = GLConst.BLEND_MODE_SRC_OVER;
        this.fBackdropFilterBoundsClip = true;
    }

    public resetOpacity(): void {
        this.fOpacity = null;
    }

    public resetClip(): void {
        this.fClippingType = ClippingType.kNone;
        this.fRectClip = null;
        this.fRRectClip = null;
        this.fPathClip = null;
    }

    public resetOffset(): void {
        this.fOffset = new Vector2f(0, 0);
    }

    public resetRotate(): void {
        this.fRotate = null;
    }

    public makeScope(recorder: Recorder, callback: (recorder: Recorder) => void) {
        let pushCount = 0;
        if (this.hasOffset()) {
            recorder.pushOffset(this.fOffset);
            pushCount++;
        }

        if (this.hasRotate()) {
            recorder.pushRotate(this.fRotate.rad, this.fRotate.pivot);
            pushCount++;
        }

        switch (this.fClippingType) {
            case ClippingType.kRect:
                recorder.pushRectClip(this.fRectClip, this.fClippingAA);
                break;
            case ClippingType.kRRect:
                recorder.pushRRectClip(this.fRRectClip, this.fClippingAA);
                break;
            case ClippingType.kPath:
                recorder.pushPathClip(this.fPathClip, GLConst.CLIP_OP_INTERSECT, this.fClippingAA);
                break;
        }
        if (this.fClippingType != ClippingType.kNone) {
            pushCount++;
        }

        if (this.hasImageFilter()) {
            recorder.pushImageFilter(this.fImageFilter);
            pushCount++;
        }

        if (this.hasBackdropFilter()) {
            recorder.pushBackdropFilter(this.fBackdropFilter,
                                        this.fBackdropFilterBlendMode,
                                        this.fBackdropFilterBoundsClip);
            pushCount++;
        }

        if (this.hasOpacity()) {
            recorder.pushOpacity(this.fOpacity);
            pushCount++;
        }

        callback(recorder);

        while (pushCount > 0) {
            recorder.pop();
            pushCount--;
        }
    }
}
