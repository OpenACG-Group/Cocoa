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

import { DrawContext } from "./draw-context";
import { CacheContext, Recorder } from './render-node-linearization';
import { RenderNode } from './render-node';
import { Event, EventEmitter } from "../base/event-dispatcher";
import { Vector2f } from "./vector";
import { Maybe } from '../base/error';

export class SubmittedEvent extends Event {
    public readonly serial: number;

    // This property is the same to the `closure` argument of method
    // `DrawContextSubmitter.submit()`. It can be used to carry
    // some userdata, and `undefined` is the default value.
    public readonly closure: any;

    constructor(serial: number, closure: any) {
        super();
        this.serial = serial;
        this.closure = closure;
    }
}

export class DrawContextSubmitter extends EventEmitter {
    private fDrawContext: DrawContext;
    private fSubmitSerial: number;
    private fPending: boolean;
    private fSubmitEventClosure: any;
    private fRNLCacheContext: CacheContext;

    constructor(drawContext: DrawContext) {
        super();
        this.fDrawContext = drawContext;
        this.fSubmitSerial = 0;
        this.fPending = false;
        this.fSubmitEventClosure = undefined;
        this.fRNLCacheContext = new CacheContext();
        super.registerEvent(SubmittedEvent);
    }

    private onSubmitted(): void {
        this.fPending = false;
        super.emitEvent(new SubmittedEvent(this.fSubmitSerial, this.fSubmitEventClosure));
        this.fSubmitEventClosure = undefined;
    }

    public submit(root: RenderNode, closure: any, captureThis: boolean): Maybe<number> {
        if (this.fPending) {
            return Maybe.None<number>();
        }
        const viewport = new Vector2f(this.fDrawContext.width, this.fDrawContext.height);
        const recorder = new Recorder();

        root.compose(recorder);

        // TODO: Support logger
        const scene = recorder.finish(viewport, this.fRNLCacheContext, null);
        this.fDrawContext.submitSceneOwnership(scene, captureThis)
            .then(this.onSubmitted.bind(this));

        this.fPending = true;
        this.fSubmitSerial++;
        this.fSubmitEventClosure = closure;

        return Maybe.Ok(this.fSubmitSerial);
    }
}
