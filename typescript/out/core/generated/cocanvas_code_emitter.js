export var Opcode;
(function (Opcode) {
    // Description: .SwitchNextBuffer
    Opcode.kSwitchNextBuffer = 0x1;
    // Description: .CommandPoolEnd
    Opcode.kCommandPoolEnd = 0x2;
    // Description: .DrawBounds                 f32 $width, f32 $height
    Opcode.kDrawBounds = 0x3;
    // Description: HeapClone                   any %from, any %key
    Opcode.kHeapClone = 0x4;
    // Description: HeapFree                    any %key
    Opcode.kHeapFree = 0x5;
    // Description: HeapCreateU32Array          any %key, u32 $size
    Opcode.kHeapCreateU32Array = 0x6;
    // Description: HeapCreateF32Array          any %key, u32 $size
    Opcode.kHeapCreateF32Array = 0x7;
    // Description: HeapU32ArrayStore           u32array %array, u32 $idx, u32 $value
    Opcode.kHeapU32ArrayStore = 0x8;
    // Description: HeapF32ArrayStore           f32array %array, u32 $idx, f32 $value
    Opcode.kHeapF32ArrayStore = 0x9;
    // Description: HeapCreateM44               any %key
    Opcode.kHeapCreateM44 = 0xa;
    // Description: HeapCreatePaint             any %key
    Opcode.kHeapCreatePaint = 0xb;
    // Description: HeapCreatePath              any %key
    Opcode.kHeapCreatePath = 0xc;
    // Description: HeapCreateVector2           any %key, f32 $x, f32 $y
    Opcode.kHeapCreateVector2 = 0xd;
    // Description: HeapCreateVector3           any %key, f32 $x, f32 $y, f32 $z
    Opcode.kHeapCreateVector3 = 0xe;
    // Description: HeapCreateVector4           any %key, f32 $x, f32 $y, f32 $z, f32 $w
    Opcode.kHeapCreateVector4 = 0xf;
    // Description: HeapCreateRect              any %key, f32 $x, f32 $y, f32 $w, f32 $h
    Opcode.kHeapCreateRect = 0x10;
    // Description: HeapCreateEmptyShader       any %key
    Opcode.kHeapCreateEmptyShader = 0x11;
    // Description: HeapCreateColorShader       any %key, u32 $color
    Opcode.kHeapCreateColorShader = 0x12;
    // Description: HeapCreateBlendShader       any %key, blender %blender, shader %dst, shader %src
    Opcode.kHeapCreateBlendShader = 0x13;
    // Description: HeapCreateLinearGradientShader any %key, vec2 %start, vec2 %end, u32array %colors, tilemode $mode
    Opcode.kHeapCreateLinearGradientShader = 0x14;
    // Description: HeapCreateLinearGradientShader2 any %key, vec2 %start, vec2 %end, u32array %colors, f32array %pos, tilemode $mode
    Opcode.kHeapCreateLinearGradientShader2 = 0x15;
    // Description: HeapCreateRadialGradientShader any %key, vec2 %center, f32 $radius, u32array %colors, tilemode $mode
    Opcode.kHeapCreateRadialGradientShader = 0x16;
    // Description: HeapCreateRadialGradientShader2 any %key, vec2 %center, f32 $radius, u32array %colors, f32array %pos, tilemode $mode
    Opcode.kHeapCreateRadialGradientShader2 = 0x17;
    // Description: HeapCreateTwoPointConicalGradientShader any %key, vec2 %start, f32 $startRadius, vec2 %end, f32 $endRadius, u32array %colors, f32array %pos, tilemode $mode
    Opcode.kHeapCreateTwoPointConicalGradientShader = 0x18;
    // Description: HeapCreateSweepGradientShader any %key, f32 $cx, f32 $cy, u32array %colors
    Opcode.kHeapCreateSweepGradientShader = 0x19;
    // Description: HeapCreateSweepGradientShader2 any %key, f32 $cx, f32 $cy, u32array %colors, f32array %pos
    Opcode.kHeapCreateSweepGradientShader2 = 0x1a;
    // Description: HeapCreatePerlinNoiseFractalNoiseShader any %key, f32 $baseFreqX, f32 $baseFreqY, u32 $numOctaves, f32 $seed
    Opcode.kHeapCreatePerlinNoiseFractalNoiseShader = 0x1b;
    // Description: HeapCreatePerlinNoiseTurbulenceShader any %key, f32 $baseFreqX, f32 $baseFreqY, u32 $numOctaves, f32 $seed
    Opcode.kHeapCreatePerlinNoiseTurbulenceShader = 0x1c;
    // Description: HeapCreateModeBlender       any %key, blendmode $mode
    Opcode.kHeapCreateModeBlender = 0x1d;
    // Description: HeapCreateArithmeticBlender any %key, f32 $k1, f32 $k2, f32 $k3, f32 $k4, u8 $enforcePremul
    Opcode.kHeapCreateArithmeticBlender = 0x1e;
    // Description: M44SetRows                  mat4x4 %m, vec4 %r0, vec4 %r1, vec4 %r2, vec4 %r3
    Opcode.kM44SetRows = 0x1f;
    // Description: M44SetCols                  mat4x4 %m, vec4 %c0, vec4 %c1, vec4 %c2, vec4 %c3
    Opcode.kM44SetCols = 0x20;
    // Description: M44SetTranslate             mat4x4 %m, f32 $x, f32 $y, f32 $z
    Opcode.kM44SetTranslate = 0x21;
    // Description: M44SetScale                 mat4x4 %m, f32 $x, f32 $y, f32 $z
    Opcode.kM44SetScale = 0x22;
    // Description: M44SetRectToRect            mat4x4 %m, rect %src, rect %dst
    Opcode.kM44SetRectToRect = 0x23;
    // Description: M44SetLookAt                mat4x4 %m, vec3 %eye, vec3 %center, vec3 %up
    Opcode.kM44SetLookAt = 0x24;
    // Description: M44SetPerspective           mat4x4 %m, f32 $near, f32 $far, f32 $angle
    Opcode.kM44SetPerspective = 0x25;
    // Description: M44SetIdentity              mat4x4 %m
    Opcode.kM44SetIdentity = 0x26;
    // Description: M44Concat                   mat4x4 %m, mat4x4 %other
    Opcode.kM44Concat = 0x27;
    // Description: PaintReset                  paint %p
    Opcode.kPaintReset = 0x28;
    // Description: PaintSetAntialias           paint %p, u8 $aa
    Opcode.kPaintSetAntialias = 0x29;
    // Description: PaintSetDither              paint %p, u8 $dither
    Opcode.kPaintSetDither = 0x2a;
    // Description: PaintSetStyleStroke         paint %p, u8 $stroke
    Opcode.kPaintSetStyleStroke = 0x2b;
    // Description: PaintSetColor               paint %p, u32 $color
    Opcode.kPaintSetColor = 0x2c;
    // Description: PaintSetAlphaf              paint %p, f32 $alpha
    Opcode.kPaintSetAlphaf = 0x2d;
    // Description: PaintSetColorARGB           paint %p, u8 $a, u8 $r, u8 $g, u8 $b
    Opcode.kPaintSetColorARGB = 0x2e;
    // Description: PaintSetStrokeWidth         paint %p, f32 $width
    Opcode.kPaintSetStrokeWidth = 0x2f;
    // Description: PaintSetStrokeMiter         paint %p, f32 $miter
    Opcode.kPaintSetStrokeMiter = 0x30;
    // Description: PaintSetStrokeCap           paint %p, paintcap $cap
    Opcode.kPaintSetStrokeCap = 0x31;
    // Description: PaintSetStrokeJoin          paint %p, paintjoin $join
    Opcode.kPaintSetStrokeJoin = 0x32;
    // Description: PaintSetShader              paint %p, shader %shader
    Opcode.kPaintSetShader = 0x33;
})(Opcode || (Opcode = {})); // namespace Opcode
export var Constants;
(function (Constants) {
    Constants.PAINTCAP_BUTT = 0x0;
    Constants.PAINTCAP_ROUND = 0x1;
    Constants.PAINTCAP_SQUARE = 0x2;
    Constants.PAINTJOIN_MITER = 0x0;
    Constants.PAINTJOIN_ROUND = 0x1;
    Constants.PAINTJOIN_BEVEL = 0x2;
    Constants.BLENDMODE_CLEAR = 0x0;
    Constants.BLENDMODE_SRC = 0x1;
    Constants.BLENDMODE_DST = 0x2;
    Constants.BLENDMODE_SRC_OVER = 0x3;
    Constants.BLENDMODE_DST_OVER = 0x4;
    Constants.BLENDMODE_SRC_IN = 0x5;
    Constants.BLENDMODE_DST_IN = 0x6;
    Constants.BLENDMODE_SRC_OUT = 0x7;
    Constants.BLENDMODE_DST_OUT = 0x8;
    Constants.BLENDMODE_SRC_ATOP = 0x9;
    Constants.BLENDMODE_DST_ATOP = 0xa;
    Constants.BLENDMODE_XOR = 0xb;
    Constants.BLENDMODE_PLUS = 0xc;
    Constants.BLENDMODE_MODULATE = 0xd;
    Constants.BLENDMODE_SCREEN = 0xe;
    Constants.BLENDMODE_OVERLAY = 0xf;
    Constants.BLENDMODE_DARKEN = 0x10;
    Constants.BLENDMODE_LIGHTEN = 0x11;
    Constants.BLENDMODE_COLOR_DODGE = 0x12;
    Constants.BLENDMODE_COLOR_BURN = 0x13;
    Constants.BLENDMODE_HARD_LIGHT = 0x14;
    Constants.BLENDMODE_SOFT_LIGHT = 0x15;
    Constants.BLENDMODE_DIFFERENCE = 0x16;
    Constants.BLENDMODE_EXCLUSION = 0x17;
    Constants.BLENDMODE_MULTIPLY = 0x18;
    Constants.BLENDMODE_HUE = 0x19;
    Constants.BLENDMODE_SATURATION = 0x1a;
    Constants.BLENDMODE_COLOR = 0x1b;
    Constants.BLENDMODE_LUMINOSITY = 0x1c;
    Constants.TILEMODE_CLAMP = 0x0;
    Constants.TILEMODE_REPEAT = 0x1;
    Constants.TILEMODE_MIRROR = 0x2;
    Constants.TILEMODE_DECAL = 0x3;
})(Constants || (Constants = {})); // namespace Constants
export class ProtoCodeEmitter {
    constructor(writer) {
        this.writer = writer;
    }
    emitSwitchNextBuffer() {
        this.writer.performPossibleBufferSwitching(2);
        this.writer.writeUint16Unsafe(Opcode.kSwitchNextBuffer | 0x0);
    }
    emitCommandPoolEnd() {
        this.writer.performPossibleBufferSwitching(2);
        this.writer.writeUint16Unsafe(Opcode.kCommandPoolEnd | 0x0);
    }
    emitDrawBounds(width, height) {
        this.writer.performPossibleBufferSwitching(10);
        this.writer.writeUint16Unsafe(Opcode.kDrawBounds | 0x200);
        this.writer.writeFloat32Unsafe(width);
        this.writer.writeFloat32Unsafe(height);
    }
    emitHeapClone(from, key) {
        this.writer.performPossibleBufferSwitching(10);
        this.writer.writeUint16Unsafe(Opcode.kHeapClone | 0x200);
        this.writer.writeUint32Unsafe(from);
        this.writer.writeUint32Unsafe(key);
    }
    emitHeapFree(key) {
        this.writer.performPossibleBufferSwitching(6);
        this.writer.writeUint16Unsafe(Opcode.kHeapFree | 0x100);
        this.writer.writeUint32Unsafe(key);
    }
    emitHeapCreateU32Array(key, size) {
        this.writer.performPossibleBufferSwitching(10);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateU32Array | 0x200);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint32Unsafe(size);
    }
    emitHeapCreateF32Array(key, size) {
        this.writer.performPossibleBufferSwitching(10);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateF32Array | 0x200);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint32Unsafe(size);
    }
    emitHeapU32ArrayStore(array, idx, value) {
        this.writer.performPossibleBufferSwitching(14);
        this.writer.writeUint16Unsafe(Opcode.kHeapU32ArrayStore | 0x300);
        this.writer.writeUint32Unsafe(array);
        this.writer.writeUint32Unsafe(idx);
        this.writer.writeUint32Unsafe(value);
    }
    emitHeapF32ArrayStore(array, idx, value) {
        this.writer.performPossibleBufferSwitching(14);
        this.writer.writeUint16Unsafe(Opcode.kHeapF32ArrayStore | 0x300);
        this.writer.writeUint32Unsafe(array);
        this.writer.writeUint32Unsafe(idx);
        this.writer.writeFloat32Unsafe(value);
    }
    emitHeapCreateM44(key) {
        this.writer.performPossibleBufferSwitching(6);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateM44 | 0x100);
        this.writer.writeUint32Unsafe(key);
    }
    emitHeapCreatePaint(key) {
        this.writer.performPossibleBufferSwitching(6);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreatePaint | 0x100);
        this.writer.writeUint32Unsafe(key);
    }
    emitHeapCreatePath(key) {
        this.writer.performPossibleBufferSwitching(6);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreatePath | 0x100);
        this.writer.writeUint32Unsafe(key);
    }
    emitHeapCreateVector2(key, x, y) {
        this.writer.performPossibleBufferSwitching(14);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateVector2 | 0x300);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeFloat32Unsafe(x);
        this.writer.writeFloat32Unsafe(y);
    }
    emitHeapCreateVector3(key, x, y, z) {
        this.writer.performPossibleBufferSwitching(18);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateVector3 | 0x400);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeFloat32Unsafe(x);
        this.writer.writeFloat32Unsafe(y);
        this.writer.writeFloat32Unsafe(z);
    }
    emitHeapCreateVector4(key, x, y, z, w) {
        this.writer.performPossibleBufferSwitching(22);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateVector4 | 0x500);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeFloat32Unsafe(x);
        this.writer.writeFloat32Unsafe(y);
        this.writer.writeFloat32Unsafe(z);
        this.writer.writeFloat32Unsafe(w);
    }
    emitHeapCreateRect(key, x, y, w, h) {
        this.writer.performPossibleBufferSwitching(22);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateRect | 0x500);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeFloat32Unsafe(x);
        this.writer.writeFloat32Unsafe(y);
        this.writer.writeFloat32Unsafe(w);
        this.writer.writeFloat32Unsafe(h);
    }
    emitHeapCreateEmptyShader(key) {
        this.writer.performPossibleBufferSwitching(6);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateEmptyShader | 0x100);
        this.writer.writeUint32Unsafe(key);
    }
    emitHeapCreateColorShader(key, color) {
        this.writer.performPossibleBufferSwitching(10);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateColorShader | 0x200);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint32Unsafe(color);
    }
    emitHeapCreateBlendShader(key, blender, dst, src) {
        this.writer.performPossibleBufferSwitching(18);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateBlendShader | 0x400);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint32Unsafe(blender);
        this.writer.writeUint32Unsafe(dst);
        this.writer.writeUint32Unsafe(src);
    }
    emitHeapCreateLinearGradientShader(key, start, end, colors, mode) {
        this.writer.performPossibleBufferSwitching(19);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateLinearGradientShader | 0x500);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint32Unsafe(start);
        this.writer.writeUint32Unsafe(end);
        this.writer.writeUint32Unsafe(colors);
        this.writer.writeUint8Unsafe(mode);
    }
    emitHeapCreateLinearGradientShader2(key, start, end, colors, pos, mode) {
        this.writer.performPossibleBufferSwitching(23);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateLinearGradientShader2 | 0x600);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint32Unsafe(start);
        this.writer.writeUint32Unsafe(end);
        this.writer.writeUint32Unsafe(colors);
        this.writer.writeUint32Unsafe(pos);
        this.writer.writeUint8Unsafe(mode);
    }
    emitHeapCreateRadialGradientShader(key, center, radius, colors, mode) {
        this.writer.performPossibleBufferSwitching(19);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateRadialGradientShader | 0x500);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint32Unsafe(center);
        this.writer.writeFloat32Unsafe(radius);
        this.writer.writeUint32Unsafe(colors);
        this.writer.writeUint8Unsafe(mode);
    }
    emitHeapCreateRadialGradientShader2(key, center, radius, colors, pos, mode) {
        this.writer.performPossibleBufferSwitching(23);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateRadialGradientShader2 | 0x600);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint32Unsafe(center);
        this.writer.writeFloat32Unsafe(radius);
        this.writer.writeUint32Unsafe(colors);
        this.writer.writeUint32Unsafe(pos);
        this.writer.writeUint8Unsafe(mode);
    }
    emitHeapCreateTwoPointConicalGradientShader(key, start, startRadius, end, endRadius, colors, pos, mode) {
        this.writer.performPossibleBufferSwitching(31);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateTwoPointConicalGradientShader | 0x800);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint32Unsafe(start);
        this.writer.writeFloat32Unsafe(startRadius);
        this.writer.writeUint32Unsafe(end);
        this.writer.writeFloat32Unsafe(endRadius);
        this.writer.writeUint32Unsafe(colors);
        this.writer.writeUint32Unsafe(pos);
        this.writer.writeUint8Unsafe(mode);
    }
    emitHeapCreateSweepGradientShader(key, cx, cy, colors) {
        this.writer.performPossibleBufferSwitching(18);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateSweepGradientShader | 0x400);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeFloat32Unsafe(cx);
        this.writer.writeFloat32Unsafe(cy);
        this.writer.writeUint32Unsafe(colors);
    }
    emitHeapCreateSweepGradientShader2(key, cx, cy, colors, pos) {
        this.writer.performPossibleBufferSwitching(22);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateSweepGradientShader2 | 0x500);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeFloat32Unsafe(cx);
        this.writer.writeFloat32Unsafe(cy);
        this.writer.writeUint32Unsafe(colors);
        this.writer.writeUint32Unsafe(pos);
    }
    emitHeapCreatePerlinNoiseFractalNoiseShader(key, baseFreqX, baseFreqY, numOctaves, seed) {
        this.writer.performPossibleBufferSwitching(22);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreatePerlinNoiseFractalNoiseShader | 0x500);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeFloat32Unsafe(baseFreqX);
        this.writer.writeFloat32Unsafe(baseFreqY);
        this.writer.writeUint32Unsafe(numOctaves);
        this.writer.writeFloat32Unsafe(seed);
    }
    emitHeapCreatePerlinNoiseTurbulenceShader(key, baseFreqX, baseFreqY, numOctaves, seed) {
        this.writer.performPossibleBufferSwitching(22);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreatePerlinNoiseTurbulenceShader | 0x500);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeFloat32Unsafe(baseFreqX);
        this.writer.writeFloat32Unsafe(baseFreqY);
        this.writer.writeUint32Unsafe(numOctaves);
        this.writer.writeFloat32Unsafe(seed);
    }
    emitHeapCreateModeBlender(key, mode) {
        this.writer.performPossibleBufferSwitching(7);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateModeBlender | 0x200);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint8Unsafe(mode);
    }
    emitHeapCreateArithmeticBlender(key, k1, k2, k3, k4, enforcePremul) {
        this.writer.performPossibleBufferSwitching(23);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateArithmeticBlender | 0x600);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeFloat32Unsafe(k1);
        this.writer.writeFloat32Unsafe(k2);
        this.writer.writeFloat32Unsafe(k3);
        this.writer.writeFloat32Unsafe(k4);
        this.writer.writeUint8Unsafe(enforcePremul);
    }
    emitM44SetRows(m, r0, r1, r2, r3) {
        this.writer.performPossibleBufferSwitching(22);
        this.writer.writeUint16Unsafe(Opcode.kM44SetRows | 0x500);
        this.writer.writeUint32Unsafe(m);
        this.writer.writeUint32Unsafe(r0);
        this.writer.writeUint32Unsafe(r1);
        this.writer.writeUint32Unsafe(r2);
        this.writer.writeUint32Unsafe(r3);
    }
    emitM44SetCols(m, c0, c1, c2, c3) {
        this.writer.performPossibleBufferSwitching(22);
        this.writer.writeUint16Unsafe(Opcode.kM44SetCols | 0x500);
        this.writer.writeUint32Unsafe(m);
        this.writer.writeUint32Unsafe(c0);
        this.writer.writeUint32Unsafe(c1);
        this.writer.writeUint32Unsafe(c2);
        this.writer.writeUint32Unsafe(c3);
    }
    emitM44SetTranslate(m, x, y, z) {
        this.writer.performPossibleBufferSwitching(18);
        this.writer.writeUint16Unsafe(Opcode.kM44SetTranslate | 0x400);
        this.writer.writeUint32Unsafe(m);
        this.writer.writeFloat32Unsafe(x);
        this.writer.writeFloat32Unsafe(y);
        this.writer.writeFloat32Unsafe(z);
    }
    emitM44SetScale(m, x, y, z) {
        this.writer.performPossibleBufferSwitching(18);
        this.writer.writeUint16Unsafe(Opcode.kM44SetScale | 0x400);
        this.writer.writeUint32Unsafe(m);
        this.writer.writeFloat32Unsafe(x);
        this.writer.writeFloat32Unsafe(y);
        this.writer.writeFloat32Unsafe(z);
    }
    emitM44SetRectToRect(m, src, dst) {
        this.writer.performPossibleBufferSwitching(14);
        this.writer.writeUint16Unsafe(Opcode.kM44SetRectToRect | 0x300);
        this.writer.writeUint32Unsafe(m);
        this.writer.writeUint32Unsafe(src);
        this.writer.writeUint32Unsafe(dst);
    }
    emitM44SetLookAt(m, eye, center, up) {
        this.writer.performPossibleBufferSwitching(18);
        this.writer.writeUint16Unsafe(Opcode.kM44SetLookAt | 0x400);
        this.writer.writeUint32Unsafe(m);
        this.writer.writeUint32Unsafe(eye);
        this.writer.writeUint32Unsafe(center);
        this.writer.writeUint32Unsafe(up);
    }
    emitM44SetPerspective(m, near, far, angle) {
        this.writer.performPossibleBufferSwitching(18);
        this.writer.writeUint16Unsafe(Opcode.kM44SetPerspective | 0x400);
        this.writer.writeUint32Unsafe(m);
        this.writer.writeFloat32Unsafe(near);
        this.writer.writeFloat32Unsafe(far);
        this.writer.writeFloat32Unsafe(angle);
    }
    emitM44SetIdentity(m) {
        this.writer.performPossibleBufferSwitching(6);
        this.writer.writeUint16Unsafe(Opcode.kM44SetIdentity | 0x100);
        this.writer.writeUint32Unsafe(m);
    }
    emitM44Concat(m, other) {
        this.writer.performPossibleBufferSwitching(10);
        this.writer.writeUint16Unsafe(Opcode.kM44Concat | 0x200);
        this.writer.writeUint32Unsafe(m);
        this.writer.writeUint32Unsafe(other);
    }
    emitPaintReset(p) {
        this.writer.performPossibleBufferSwitching(6);
        this.writer.writeUint16Unsafe(Opcode.kPaintReset | 0x100);
        this.writer.writeUint32Unsafe(p);
    }
    emitPaintSetAntialias(p, aa) {
        this.writer.performPossibleBufferSwitching(7);
        this.writer.writeUint16Unsafe(Opcode.kPaintSetAntialias | 0x200);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint8Unsafe(aa);
    }
    emitPaintSetDither(p, dither) {
        this.writer.performPossibleBufferSwitching(7);
        this.writer.writeUint16Unsafe(Opcode.kPaintSetDither | 0x200);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint8Unsafe(dither);
    }
    emitPaintSetStyleStroke(p, stroke) {
        this.writer.performPossibleBufferSwitching(7);
        this.writer.writeUint16Unsafe(Opcode.kPaintSetStyleStroke | 0x200);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint8Unsafe(stroke);
    }
    emitPaintSetColor(p, color) {
        this.writer.performPossibleBufferSwitching(10);
        this.writer.writeUint16Unsafe(Opcode.kPaintSetColor | 0x200);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint32Unsafe(color);
    }
    emitPaintSetAlphaf(p, alpha) {
        this.writer.performPossibleBufferSwitching(10);
        this.writer.writeUint16Unsafe(Opcode.kPaintSetAlphaf | 0x200);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeFloat32Unsafe(alpha);
    }
    emitPaintSetColorARGB(p, a, r, g, b) {
        this.writer.performPossibleBufferSwitching(10);
        this.writer.writeUint16Unsafe(Opcode.kPaintSetColorARGB | 0x500);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint8Unsafe(a);
        this.writer.writeUint8Unsafe(r);
        this.writer.writeUint8Unsafe(g);
        this.writer.writeUint8Unsafe(b);
    }
    emitPaintSetStrokeWidth(p, width) {
        this.writer.performPossibleBufferSwitching(10);
        this.writer.writeUint16Unsafe(Opcode.kPaintSetStrokeWidth | 0x200);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeFloat32Unsafe(width);
    }
    emitPaintSetStrokeMiter(p, miter) {
        this.writer.performPossibleBufferSwitching(10);
        this.writer.writeUint16Unsafe(Opcode.kPaintSetStrokeMiter | 0x200);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeFloat32Unsafe(miter);
    }
    emitPaintSetStrokeCap(p, cap) {
        this.writer.performPossibleBufferSwitching(7);
        this.writer.writeUint16Unsafe(Opcode.kPaintSetStrokeCap | 0x200);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint8Unsafe(cap);
    }
    emitPaintSetStrokeJoin(p, join) {
        this.writer.performPossibleBufferSwitching(7);
        this.writer.writeUint16Unsafe(Opcode.kPaintSetStrokeJoin | 0x200);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint8Unsafe(join);
    }
    emitPaintSetShader(p, shader) {
        this.writer.performPossibleBufferSwitching(10);
        this.writer.writeUint16Unsafe(Opcode.kPaintSetShader | 0x200);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint32Unsafe(shader);
    }
}
