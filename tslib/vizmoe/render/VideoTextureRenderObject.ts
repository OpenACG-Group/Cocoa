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

import { LinkedList } from "../../core/linked_list";
import { FlattenContext, RenderObject, RenderObjectType } from "./RenderObject";
import { VideoBuffer } from "synthetic://utau";
import { Constants as GLConst } from "synthetic://glamor";
import { Rect } from '../graphics/Rect';

export class VideoTextureRenderObject extends RenderObject {
    private bounds_: Rect;
    private video_buffer_: VideoBuffer;

    constructor(parent: RenderObject, bounds: Rect) {
        super(parent, new LinkedList<RenderObject>(), RenderObjectType.kPainter);
        this.bounds_ = bounds;
        this.video_buffer_ = null;
    }

    public get bounds(): Rect {
        return this.bounds_;
    }

    /**
     * Submit a new video buffer to be presented. Caller should assume that the
     * ownership of `buffer` is transferred to this object from the caller,
     * and caller must NOT dispose the video buffer. Vizmoe will dispose the
     * buffer automatically.
     * Note that the video buffer will only be displayed in current frame,
     * which means caller always should update the video buffer every frame
     * to make sure that there is always a live buffer to be displayed.
     * 
     * @param buffer        A video buffer to be displayed.
     * @param bounds        An optional bounds rectangle; use the old bounds if not provided,
     *                      and the initial bounds is assigned by constructor.
     */
    public update(buffer: VideoBuffer, bounds?: Rect): void {
        if (bounds != null) {
            this.bounds_ = bounds;
        }
        this.video_buffer_ = buffer;
    }

    public flatten(context: FlattenContext): void {
        if (this.video_buffer_ == null) {
            return;
        }

        context.recorder.insertDrawVideoBuffer(
            this.video_buffer_, this.bounds_, GLConst.SAMPLING_FILTER_LINEAR);
        
        this.video_buffer_ = null;
    }
}
