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

import * as ui from 'glamor';
import { LinkedList } from '../../core/linked_list';

enum EnumMatchMode {
    kNoneOf,
    kAnyOf,
    kCallback
}

class ValueMatcher<T> {
    private readonly fMode: EnumMatchMode;
    private readonly fSelections: Array<T>;
    private readonly fCallback: (value: T) => boolean;

    constructor(mode: EnumMatchMode, selections?: Array<T>, callback?: (value: T) => boolean) {
        this.fMode = mode;
        if (mode == EnumMatchMode.kCallback) {
            if (!callback) {
                throw Error('Value matcher requires a callback function');
            }
            this.fSelections = null;
            this.fCallback = callback;
        } else {
            if (!selections) {
                throw Error('Value matcher requires selections');
            }
            this.fSelections = selections;
            this.fCallback = null;
        }
    }

    public match(value: T): boolean {
        if (this.fMode == EnumMatchMode.kAnyOf) {
            const test = this.fSelections.find(entry => entry == value);
            return (test != undefined);
        }

        if (this.fMode == EnumMatchMode.kNoneOf) {
            return this.fSelections.every(entry => entry != value);
        }

        return this.fCallback(value);
    }
}

type InFlowConnectorForwardCb = (connector: InFlowConnector) => void;
class InFlowConnector {
    private readonly fColorTypeMatcher: ValueMatcher<ui.ColorType>;
    private readonly fAlphaTypeMatcher: ValueMatcher<ui.AlphaType>;
    private readonly fForwardCallback: InFlowConnectorForwardCb;
    private readonly fQueue: LinkedList<ui.CkBitmap>;

    constructor(colorTypeMatcher: ValueMatcher<ui.ColorType>,
                alphaTypeMatcher: ValueMatcher<ui.AlphaType>,
                forwardCallback: InFlowConnectorForwardCb) {
        this.fColorTypeMatcher = colorTypeMatcher;
        this.fAlphaTypeMatcher = alphaTypeMatcher;
        this.fForwardCallback = forwardCallback;
    }

    public feed(bitmap: ui.CkBitmap): boolean {
        if (!this.fColorTypeMatcher.match(bitmap.colorType)) {
            return false;
        }
        if (!this.fAlphaTypeMatcher.match(bitmap.alphaType)) {
            return false;
        }
        this.fQueue.push(bitmap);
        this.fForwardCallback(this);
        return true;
    }

    public hasNextBitmap(): boolean {
        return !this.fQueue.isEmpty();
    }

    public consumeFirstBitmap(): ui.CkBitmap {
        if (this.fQueue.isEmpty()) {
            throw Error('No more bitmaps can be consumed');
        }
        const first = this.fQueue.first().value;
        this.fQueue.removeNode(this.fQueue.first());
        return first;
    }

    public isAcceptableColorType(colorType: ui.ColorType): boolean {
        return this.fColorTypeMatcher.match(colorType);
    }

    public isAcceptableAlphaType(alphaType: ui.AlphaType): boolean {
        return this.fColorTypeMatcher.match(alphaType);
    }

    // TODO(sora): `createAcceptableFormatConverter`
}

class OutFlowConnector {
}

class Linkage {
}
