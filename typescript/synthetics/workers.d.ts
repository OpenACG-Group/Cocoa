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

import { EventEmitterBase } from 'private/base';

export class Worker {
    private constructor();
    public static MakeFromURL(url: string): Worker;

    readonly port: MessagePort;
}

export class MessagePort extends EventEmitterBase {
    private constructor();
    public static MakeConnectedPair(): [MessagePort, MessagePort];

    public close(): void;
    public postMessage(message: any, transferList?: Array<any>): void;
}
