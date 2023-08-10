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

import * as GL from 'glamor';
import { DrawContext } from "./draw-context";
import { Event, EventEmitter } from "../base/event-dispatcher";
import { Buffer } from 'core';
import { Rect } from "./rectangle";

export class TextureDeletionEvent extends Event {
    constructor() {
        super();
    }
}

export class StaticTexture extends EventEmitter {
}