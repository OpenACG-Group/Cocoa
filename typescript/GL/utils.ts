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

export namespace RectOperators {
    export function Clone(from: GL.CkArrayXYWHRect): GL.CkArrayXYWHRect {
        return [from[0], from[1], from[2], from[3]];
    }

    export function ClearXY(from: GL.CkArrayXYWHRect): GL.CkArrayXYWHRect {
        return [0, 0, from[2], from[3]];
    }

    export function Union(a: GL.CkArrayXYWHRect, b: GL.CkArrayXYWHRect): GL.CkArrayXYWHRect {
        // Empty rects
        if (a[2] == 0 && a[3] == 0) {
            return Clone(b);
        }
        if (b[2] == 0 && b[3] == 0) {
            return Clone(a);
        }

        // Left, Top, Right, Bottom
        const L1 = a[0], T1 = a[1], R1 = a[0] + a[2], B1 = a[1] + a[3];
        const L2 = b[0], T2 = b[1], R2 = b[0] + b[2], B2 = b[1] + b[3];
        
        const LR = L1 < L2 ? L1 : L2,
              TR = T1 < T2 ? T1 : T2,
              RR = R1 > R2 ? R1 : R2,
              BR = B1 > B2 ? B1 : B2;

        // Convert to CkRect XYWH format
        return [LR, TR, RR - LR, BR - TR];
    }
}
