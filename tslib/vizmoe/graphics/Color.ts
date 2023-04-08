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

import { CkColor4f } from 'glamor';
import * as Fmt from '../../core/formatter';

/**
 * 32-bit ARGB color value, unpremultiplied. Color components are always
 * in the order of ARGB (bits: A[31:24] R[23:16] G[15:8] B[7:0]).
 * For the alpha component, 255 is 100% opaque, zero is 100% transparent.
 */
export type Color32 = number;

function clamp(v: number, lower: number, upper: number): number {
    return Math.min(Math.max(v, lower), upper);
}

export enum ARGBChannel {
    kA,
    kR,
    kG,
    kB
}

/**
 * Float ARGB color value.
 */
export class Color4f implements Fmt.Formattable {
    public static FromColor32(from: Color32): Color4f {
        return new Color4f(((from >>> 16) & 0xff) / 255,
                           ((from >>> 8) & 0xff) / 255,
                           (from & 0xff) / 255,
                           ((from >> 24) & 0xff) / 255);
    }

    public static Make(R: number, G: number, B: number, A: number): Color4f {
        return new Color4f(R, G, B, A);
    }

    public static FromHSV(hsv: number[], alpha: number): Color4f {
        if (hsv.length != 3) {
            throw Error('Invalid HSV color triple');
        }
        const H = hsv[0], S = hsv[1], V = hsv[2];
        const C = V * S;
        const X = C * (1 - Math.abs((H / 60) % 2 - 1));
        const m = V - C;

        let R = 0, G = 0, B = 0;
        if (H >= 0 && H < 60) {
            R = C;
            G = X;
        } else if (H >= 60 && H < 120) {
            R = X;
            G = C;
        } else if (H >= 120 && H < 180) {
            G = C;
            B = X;
        } else if (H >= 180 && H < 240) {
            G = X;
            B = C;
        } else if (H >= 240 && H < 300) {
            R = X;
            B = C;
        } else if (H >= 300 && H < 360) {
            R = C;
            B = X;
        } else {
            throw RangeError('Invalid HSV color triple: H channel is out of range');
        }

        return new Color4f(R + m, G + m, B + m, alpha);
    }

    private constructor(_R: number, _G: number, _B: number, _A: number) {
        this.R = _R;
        this.G = _G;
        this.B = _B;
        this.A = _A;
    }

    public [Fmt.kObjectFormatter](ctx: Fmt.FormatterContext): Array<Fmt.TextBlock> {
        return [
            Fmt.TB(Fmt.TextBlockLayoutHint.kPrefix, [Fmt.TAG('Vizmoe.Color4f')]),
            Fmt.TB(Fmt.TextBlockLayoutHint.kCompoundStructureBegin, [
                Fmt.TAG('(')
            ]),
            ...Fmt.formatAnyValue(this.R, ctx),
            Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [
                Fmt.TAG(',')
            ]),
            ...Fmt.formatAnyValue(this.G, ctx),
            Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [
                Fmt.TAG(',')
            ]),
            ...Fmt.formatAnyValue(this.B, ctx),
            Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [
                Fmt.TAG(',')
            ]),
            ...Fmt.formatAnyValue(this.A, ctx),
            Fmt.TB(Fmt.TextBlockLayoutHint.kCompoundStructureEnd, [
                Fmt.TAG(')')
            ])
        ];
    }

    public fits(): boolean {
        return (this.R >= 0 && this.R <= 1 &&
                this.G >= 0 && this.G <= 1 &&
                this.B >= 0 && this.B <= 1 &&
                this.A >= 0 && this.A <= 1);
    }

    public clamp(): Color4f {
        return new Color4f(clamp(this.R, 0, 1),
                           clamp(this.G, 0, 1),
                           clamp(this.B, 0, 1),
                           clamp(this.A, 0, 1));
    }

    // Layout selector: [R, G, B, A]
    public swizzle(layout: Array<ARGBChannel>): Color4f {
        if (layout.length != 4) {
            throw Error('Invalid color channel layout');
        }
        const ch = [];
        ch[ARGBChannel.kA] = this.A;
        ch[ARGBChannel.kR] = this.R;
        ch[ARGBChannel.kG] = this.G;
        ch[ARGBChannel.kB] = this.B;
        return new Color4f(ch[layout[0]], ch[layout[1]], ch[layout[2]], ch[layout[3]]);
    }

    public toColor32(): Color32 {
        return ((this.A * 255) << 24) | ((this.R * 255) << 16) |
               ((this.G * 255) << 8) | (this.B * 255);
    }

    public equalTo(other: Color4f): boolean {
        return (this.A == other.A && this.R == other.R &&
                this.G == other.G && this.B == other.B);
    }

    public mul(S: number): Color4f {
        return new Color4f(S * this.R, S * this.G, S * this.B, S * this.A);
    }

    public mulc(S: Color4f): Color4f {
        return new Color4f(S.R * this.R, S.G * this.G, S.B * this.B, S.A * this.A);
    }

    public premul(): Color4f {
        return new Color4f(this.A * this.R, this.A * this.G,
                           this.A * this.B, this.A);
    }

    public unpremul(): Color4f {
        if (this.A == 0) {
            return new Color4f(0, 0, 0, 0);
        }
        const inv = 1 / this.A;
        return new Color4f(this.R * inv, this.G * inv, this.B * inv, this.A);
    }

    public makeOpaque(): Color4f {
        return new Color4f(this.R, this.G, this.B, 1);
    }

    public toHSV(): number[] {
        const Cmax = Math.max(this.R, this.G, this.B);
        const Cmin = Math.min(this.R, this.G, this.B);
        const d = Cmax - Cmin;

        let H = 0, S = 0, V = 0;

        if (Cmax == this.R) {
            H = 60 * (((this.G - this.B) / d) % 6);
        } else if (Cmax == this.G) {
            H = 60 * (((this.B - this.R) / d) + 2);
        } else if (Cmax == this.B) {
            H = 60 * (((this.R - this.G) / d) + 4);
        }

        if (H < 0) {
            H += 360;
        }

        if (Cmax != 0) {
            S = d / Cmax;
        }

        V = Cmax;
        return [H, S, V];
    }

    public toGLType(): CkColor4f {
        return [this.R, this.G, this.B, this.A];
    }

    public readonly R: number;
    public readonly G: number;
    public readonly B: number;
    public readonly A: number;
}

export function Color32ToHSV(color: Color32): number[] {
    return Color4f.FromColor32(color).toHSV();
}

export function HSVToColor32(hsv: number[], alphaNorm: number): Color32 {
    return Color4f.FromHSV(hsv, alphaNorm).toColor32();
}

export namespace Const {
    export const kAlphaTransparent  = 0x00;
    export const kAlphaOpaque       = 0xff;

    export const kColorTransparent  = 0x00000000;
    export const kColorWhite        = 0xFFFFFFFF;
    export const kColorBlack        = 0xFF000000;
    export const kColorDarkGrey     = 0xFF444444;
    export const kColorGrey         = 0xFF888888;
    export const kColorLightGrey    = 0xFFCCCCCC;
    export const kColorRed          = 0xFFFF0000;
    export const kColorGreen        = 0xFF00FF00;
    export const kColorBlue         = 0xFF0000FF;
    export const kColorYellow       = 0xFFFFFF00;
    export const kColorCyan         = 0xFF00FFFF;
    export const kColorMagenta      = 0xFFFF00FF;

    export const kColorTransparentF = Color4f.FromColor32(kColorTransparent);
    export const kColorWhiteF       = Color4f.FromColor32(kColorWhite);
    export const kColorBlackF       = Color4f.FromColor32(kColorBlack);
    export const kColorDarkGreyF    = Color4f.FromColor32(kColorDarkGrey);
    export const kColorGreyF        = Color4f.FromColor32(kColorGrey);
    export const kColorLightGreyF   = Color4f.FromColor32(kColorLightGrey);
    export const kColorRedF         = Color4f.FromColor32(kColorRed);
    export const kColorGreenF       = Color4f.FromColor32(kColorGreen);
    export const kColorBlueF        = Color4f.FromColor32(kColorBlue);
    export const kColorYellowF      = Color4f.FromColor32(kColorYellow);
    export const kColorCyanF        = Color4f.FromColor32(kColorCyan);
    export const kColorMagentaF     = Color4f.FromColor32(kColorMagenta);
}
