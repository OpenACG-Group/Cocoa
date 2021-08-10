export class VGRect {
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
    static MakeXYWH(x, y, w, h) {
        return new VGRect(x, y, w, h, y, x, y + h, x + w);
    }
    ;
    static MakeLTRB(left, top, right, bottom) {
        return new VGRect(left, top, right - left, bottom - top, top, left, bottom, right);
    }
    static MakeEmpty() {
        return new VGRect(NaN, NaN, NaN, NaN, NaN, NaN, NaN, NaN);
    }
    isEmpty() {
        return isNaN(this.x);
    }
    intersect(other) {
        if (this.isEmpty() || other.isEmpty())
            return VGRect.MakeEmpty();
        let a = this, b = other;
        let x = Math.max(a.left, b.left);
        let y = Math.max(a.top, b.top);
        let w = Math.min(a.right, b.right) - x;
        let h = Math.min(a.bottom, b.bottom) - y;
        if (w >= 0 && h >= 0) {
            return VGRect.MakeXYWH(x, y, w, h);
        }
        else {
            return VGRect.MakeEmpty();
        }
    }
    union(other) {
        if (this.isEmpty() || other.isEmpty())
            return VGRect.MakeEmpty();
        let a = this, b = other;
        let left = Math.min(a.left, b.left);
        let top = Math.min(a.top, b.top);
        let right = Math.max(a.right, b.right);
        let bottom = Math.max(a.bottom, b.bottom);
        return VGRect.MakeLTRB(left, top, right, bottom);
    }
    area() {
        return this.width * this.height;
    }
}
