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

import {CubicSamplers, FilterMode, MipmapMode, SamplingOptions} from 'renderer';

export class GskSampling {
    private readonly fOptions: SamplingOptions;

    public static MakeLinear(mipmap: MipmapMode = MipmapMode.None): GskSampling {
        return new GskSampling({ useCubic: false, filter: FilterMode.Linear, mipmap: mipmap });
    }

    public static MakeNearest(mipmap: MipmapMode = MipmapMode.None): GskSampling {
        return new GskSampling({ useCubic: false, filter: FilterMode.Nearest, mipmap: mipmap });
    }

    public static MakeCubicCatmullRom(): GskSampling {
        return new GskSampling(CubicSamplers.CatmullRom());
    }

    public static MakeCubicMitchell(): GskSampling {
        return new GskSampling(CubicSamplers.Mitchell());
    }

    public static MakeAniso(maxAniso: number): GskSampling {
        return new GskSampling({ useCubic: false, maxAniso: maxAniso });
    }

    private constructor(sampling: SamplingOptions) {
        this.fOptions = Object.freeze(sampling);
    }

    public asSamplingOptions(): SamplingOptions {
        return this.fOptions;
    }
}
