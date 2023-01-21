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

import * as std from 'synthetic://core';
import * as GL from 'synthetic://glamor';

export interface AppInfo {
    name: string;
    major?: number;
    minor?: number;
    patch?: number;
}

export function Initialize(appInfo?: AppInfo): void {
    const initAppInfo: GL.ApplicationInfo = {
        name: 'Default Application',
        major: 0,
        minor: 0,
        patch: 0
    };
    if (appInfo) {
        initAppInfo.name = appInfo.name;
        initAppInfo.major = appInfo.major ? appInfo.major : 0;
        initAppInfo.minor = appInfo.minor ? appInfo.minor : 0;
        initAppInfo.patch = appInfo.patch ? appInfo.patch : 0;
    }

    GL.RenderHost.Initialize(initAppInfo);
}
