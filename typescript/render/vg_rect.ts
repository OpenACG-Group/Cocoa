export class VGRect {
    public readonly x: number;
    public readonly y: number;
    public readonly width: number;
    public readonly height: number;
    public readonly top: number;
    public readonly left: number;
    public readonly bottom: number;
    public readonly right: number;

    constructor(x, y, w, h, t, l, b, r) {
        this.x = x;
        this.y = y;
        this.width = w;
        this.height = h;
        this.top = t;
        this.left = l;
        this.bottom = b;
        this.right = r;
    }

    static MakeXYWH(x: number, y: number, w: number, h: number): VGRect {
        return new VGRect(x, y, w, h,
            y, x, y + h, x + w);
    };

    static MakeLTRB(left: number, top: number, right: number, bottom: number): VGRect {
        return new VGRect(left, top, right - left,
            bottom - top, top, left, bottom, right);
    }

    static MakeEmpty(): VGRect {
        return new VGRect(NaN, NaN, NaN, NaN,
                          NaN, NaN, NaN, NaN);
    }

    isEmpty(): boolean {
        return isNaN(this.x);
    }

    intersect(other: VGRect): VGRect {
        if (this.isEmpty() || other.isEmpty())
            return VGRect.MakeEmpty();

        let a = this, b = other;
        let x = Math.max(a.left, b.left);
        let y = Math.max(a.top, b.top);
        let w = Math.min(a.right, b.right) - x;
        let h = Math.min(a.bottom, b.bottom) - y;

        if (w >= 0 && h >= 0) {
            return VGRect.MakeXYWH(x, y, w, h);
        } else {
            return VGRect.MakeEmpty();
        }
    }

    union(other: VGRect): VGRect {
        if (this.isEmpty() || other.isEmpty())
            return VGRect.MakeEmpty();

        let a = this, b = other;
        let left = Math.min(a.left, b.left);
        let top = Math.min(a.top, b.top);
        let right = Math.max(a.right, b.right);
        let bottom = Math.max(a.bottom, b.bottom);
        return VGRect.MakeLTRB(left, top, right, bottom);
    }

    area(): number {
        return this.width * this.height;
    }
}
