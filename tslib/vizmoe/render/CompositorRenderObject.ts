/**
 * This file is part of Vizmoe.
 *
 * Vizmoe is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Vizmoe is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vizmoe. If not, see <https://www.gnu.org/licenses/>.
 */

import {
    CkImageFilter,
    BlendMode,
    Constants as GConst,
    CkRect,
    CkRRect
} from 'synthetic://glamor';

import { LinkedList } from '../../core/linked_list';
import { FlattenContext, RenderObject, RenderObjectType } from './RenderObject';

enum EffectorRole {
    kBackground,
    kForeground
}

export class CompositionFilter {
    private readonly image_filter_: CkImageFilter;
    private readonly blend_mode_: BlendMode;
    private readonly auto_child_clipping_: boolean;

    public static MakeFromImageFilter(filter: CkImageFilter,
                                      auto_child_clipping: boolean,
                                      blend_mode: BlendMode = GConst.BLEND_MODE_SRC_OVER)
                                      : CompositionFilter
    {
        return new CompositionFilter(filter, blend_mode, auto_child_clipping);
    }

    private constructor(image_filter: CkImageFilter,
                        blend_mode: BlendMode,
                        auto_child_clipping: boolean)
    {
        this.image_filter_ = image_filter;
        this.blend_mode_ = blend_mode;
        this.auto_child_clipping_ = auto_child_clipping;
    }

    public apply(ctx: FlattenContext, role: EffectorRole): void {
        if (role == EffectorRole.kForeground) {
            ctx.recorder.insertPushImageFilter(this.image_filter_);
        } else {
            ctx.recorder.insertPushBackdropFilter(this.image_filter_,
                                                     this.blend_mode_,
                                                     this.auto_child_clipping_);
        }
    }
}

export class CompositionClip {
    private readonly shape_rect_: CkRect | null;
    private readonly shape_rrect_: CkRRect | null;
    private readonly AA_: boolean;

    public static MakeFromRect(rect: CkRect, AA: boolean): CompositionClip {
        return new CompositionClip(rect, null, AA);
    }

    public static MakeFromRRect(rrect: CkRRect, AA: boolean): CompositionClip {
        return new CompositionClip(null, rrect, AA);
    }

    private constructor(shape_rect: CkRect | null, shape_rrect: CkRRect | null, AA: boolean) {
        this.shape_rect_ = shape_rect;
        this.shape_rrect_ = shape_rrect;
        this.AA_ = AA;
    }

    public apply(ctx: FlattenContext): void {
        if (this.shape_rect_ != null) {
            ctx.recorder.insertPushRectClip(this.shape_rect_, this.AA_);
        } else if (this.shape_rrect_ != null) {
            ctx.recorder.insertPushRRectClip(this.shape_rrect_, this.AA_);
        }
    }
}

export class CompositorRenderObject extends RenderObject {
    private readonly foreground_filter_: CompositionFilter | null;
    private readonly background_filter_: CompositionFilter | null;
    private readonly clip_: CompositionClip | null;

    constructor(parent: RenderObject,
                foreground_filter: CompositionFilter | null,
                background_filter: CompositionFilter | null,
                clip: CompositionClip | null)
    {
        super(parent, new LinkedList<RenderObject>(), RenderObjectType.kCompositor);
        this.foreground_filter_ = foreground_filter;
        this.background_filter_ = background_filter;
        this.clip_ = clip;
    }

    public get hasForegroundEffector(): boolean {
        return (this.foreground_filter_ != null);
    }

    public get hasBackgroundEffector(): boolean {
        return (this.background_filter_ != null);
    }

    public get hasClipping(): boolean {
        return (this.clip_ != null);
    }

    public get isTrivialComposition(): boolean {
        return this.foreground_filter_ == null &&
               this.background_filter_ == null &&
               this.clip_ == null;
    }

    public flatten(context: FlattenContext): void {
        let extra_layers_count = 0;

        if (this.clip_ != null) {
            this.clip_.apply(context);
            extra_layers_count++;
        }

        if (this.background_filter_ != null) {
            this.background_filter_.apply(context, EffectorRole.kBackground);
            extra_layers_count++;
        }

        if (this.foreground_filter_ != null) {
            this.foreground_filter_.apply(context, EffectorRole.kBackground);
            extra_layers_count++;
        }

        for (const child of this.children_) {
            child.flatten(context);
        }

        while (extra_layers_count > 0) {
            context.recorder.insertPop();
            extra_layers_count--;
        }
    }
}
