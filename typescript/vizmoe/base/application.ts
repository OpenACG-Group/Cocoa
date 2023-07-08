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

import * as gl from 'glamor';

export interface VersionTriple {
    major: number;
    minor?: number;
    patch?: number;
}

export interface Options {
    hwCompose?: boolean;
}

export class Application {
    private readonly fQueueProfiling: boolean;
    private readonly fHasHWCompose: boolean;
    private readonly fOptions: Options;
    private fDisplay: gl.Display;
    private fMainWindow: gl.Surface;

    constructor(name: string, version: VersionTriple, options: Options = {}) {
        this.fOptions = options;

        this.fQueueProfiling = gl.queryCapabilities(
            gl.Constants.CAPABILITY_MESSAGE_QUEUE_PROFILING_ENABLED) as boolean;
        this.fHasHWCompose = gl.queryCapabilities(
            gl.Constants.CAPABILITY_HWCOMPOSE_ENABLED) as boolean;

        if (options.hwCompose && !this.fHasHWCompose) {
            throw Error('HWCompose is disabled, but required');
        }
        this.fHasHWCompose = options.hwCompose;

        if (!version.minor) {
            version.minor = 0;
        }
        if (!version.patch) {
            version.patch = 0;
        }

        gl.RenderHost.Initialize({
            name: name,
            major: version.major,
            minor: version.minor,
            patch: version.patch
        });

        this.fDisplay = null;
        this.fMainWindow = null;
    }

    public connectDisplay(): Promise<void> {
        if (this.fDisplay) {
            throw Error('Display has been already connected');
        }
        return gl.RenderHost.Connect().then((display) => {
            this.fDisplay = display;
        });
    }

    public dispose(): Promise<void> {
        if (this.fDisplay) {
            return this.fDisplay.close().then(() => {
                gl.RenderHost.Dispose();
            });
        } else {
            gl.RenderHost.Dispose();
            return Promise.resolve();
        }
    }

    public isQueueProfiling(): boolean {
        return this.fQueueProfiling;
    }

    public hasHWCompose(): boolean {
        return this.fHasHWCompose;
    }
}
