import { IProtoBufferWriter } from '../cocanvas_iface';
export namespace Opcode {
// Description: .SwitchNextBuffer
export const kSwitchNextBuffer = 0x1;

// Description: .CommandPoolEnd
export const kCommandPoolEnd = 0x2;

// Description: .DrawBounds                 f32 $width, f32 $height
export const kDrawBounds = 0x3;

// Description: HeapClone                   any %from, any %key
export const kHeapClone = 0x4;

// Description: HeapFree                    any %key
export const kHeapFree = 0x5;

// Description: HeapCreateU32Array          any %key, u32 $size
export const kHeapCreateU32Array = 0x6;

// Description: HeapCreateF32Array          any %key, u32 $size
export const kHeapCreateF32Array = 0x7;

// Description: HeapU32ArrayStore           u32array %array, u32 $idx, u32 $value
export const kHeapU32ArrayStore = 0x8;

// Description: HeapF32ArrayStore           f32array %array, u32 $idx, f32 $value
export const kHeapF32ArrayStore = 0x9;

// Description: HeapCreateM44               any %key
export const kHeapCreateM44 = 0xa;

// Description: HeapCreatePaint             any %key
export const kHeapCreatePaint = 0xb;

// Description: HeapCreatePath              any %key
export const kHeapCreatePath = 0xc;

// Description: HeapCreateVector2           any %key, f32 $x, f32 $y
export const kHeapCreateVector2 = 0xd;

// Description: HeapCreateVector3           any %key, f32 $x, f32 $y, f32 $z
export const kHeapCreateVector3 = 0xe;

// Description: HeapCreateVector4           any %key, f32 $x, f32 $y, f32 $z, f32 $w
export const kHeapCreateVector4 = 0xf;

// Description: HeapCreateRect              any %key, f32 $x, f32 $y, f32 $w, f32 $h
export const kHeapCreateRect = 0x10;

// Description: HeapCreateEmptyShader       any %key
export const kHeapCreateEmptyShader = 0x11;

// Description: HeapCreateColorShader       any %key, u32 $color
export const kHeapCreateColorShader = 0x12;

// Description: HeapCreateBlendShader       any %key, blender %blender, shader %dst, shader %src
export const kHeapCreateBlendShader = 0x13;

// Description: HeapCreateLinearGradientShader any %key, vec2 %start, vec2 %end, u32array %colors, tilemode $mode
export const kHeapCreateLinearGradientShader = 0x14;

// Description: HeapCreateLinearGradientShader2 any %key, vec2 %start, vec2 %end, u32array %colors, f32array %pos, tilemode $mode
export const kHeapCreateLinearGradientShader2 = 0x15;

// Description: HeapCreateRadialGradientShader any %key, vec2 %center, f32 $radius, u32array %colors, tilemode $mode
export const kHeapCreateRadialGradientShader = 0x16;

// Description: HeapCreateRadialGradientShader2 any %key, vec2 %center, f32 $radius, u32array %colors, f32array %pos, tilemode $mode
export const kHeapCreateRadialGradientShader2 = 0x17;

// Description: HeapCreateTwoPointConicalGradientShader any %key, vec2 %start, f32 $startRadius, vec2 %end, f32 $endRadius, u32array %colors, f32array %pos, tilemode $mode
export const kHeapCreateTwoPointConicalGradientShader = 0x18;

// Description: HeapCreateSweepGradientShader any %key, f32 $cx, f32 $cy, u32array %colors
export const kHeapCreateSweepGradientShader = 0x19;

// Description: HeapCreateSweepGradientShader2 any %key, f32 $cx, f32 $cy, u32array %colors, f32array %pos
export const kHeapCreateSweepGradientShader2 = 0x1a;

// Description: HeapCreatePerlinNoiseFractalNoiseShader any %key, f32 $baseFreqX, f32 $baseFreqY, u32 $numOctaves, f32 $seed
export const kHeapCreatePerlinNoiseFractalNoiseShader = 0x1b;

// Description: HeapCreatePerlinNoiseTurbulenceShader any %key, f32 $baseFreqX, f32 $baseFreqY, u32 $numOctaves, f32 $seed
export const kHeapCreatePerlinNoiseTurbulenceShader = 0x1c;

// Description: HeapCreateModeBlender       any %key, blendmode $mode
export const kHeapCreateModeBlender = 0x1d;

// Description: HeapCreateArithmeticBlender any %key, f32 $k1, f32 $k2, f32 $k3, f32 $k4, u8 $enforcePremul
export const kHeapCreateArithmeticBlender = 0x1e;

// Description: M44SetRows                  mat4x4 %m, vec4 %r0, vec4 %r1, vec4 %r2, vec4 %r3
export const kM44SetRows = 0x1f;

// Description: M44SetCols                  mat4x4 %m, vec4 %c0, vec4 %c1, vec4 %c2, vec4 %c3
export const kM44SetCols = 0x20;

// Description: M44SetTranslate             mat4x4 %m, f32 $x, f32 $y, f32 $z
export const kM44SetTranslate = 0x21;

// Description: M44SetScale                 mat4x4 %m, f32 $x, f32 $y, f32 $z
export const kM44SetScale = 0x22;

// Description: M44SetRectToRect            mat4x4 %m, rect %src, rect %dst
export const kM44SetRectToRect = 0x23;

// Description: M44SetLookAt                mat4x4 %m, vec3 %eye, vec3 %center, vec3 %up
export const kM44SetLookAt = 0x24;

// Description: M44SetPerspective           mat4x4 %m, f32 $near, f32 $far, f32 $angle
export const kM44SetPerspective = 0x25;

// Description: M44SetIdentity              mat4x4 %m
export const kM44SetIdentity = 0x26;

// Description: M44Concat                   mat4x4 %m, mat4x4 %other
export const kM44Concat = 0x27;

// Description: PaintReset                  paint %p
export const kPaintReset = 0x28;

// Description: PaintSetAntialias           paint %p, u8 $aa
export const kPaintSetAntialias = 0x29;

// Description: PaintSetDither              paint %p, u8 $dither
export const kPaintSetDither = 0x2a;

// Description: PaintSetStyleStroke         paint %p, u8 $stroke
export const kPaintSetStyleStroke = 0x2b;

// Description: PaintSetColor               paint %p, u32 $color
export const kPaintSetColor = 0x2c;

// Description: PaintSetAlphaf              paint %p, f32 $alpha
export const kPaintSetAlphaf = 0x2d;

// Description: PaintSetColorARGB           paint %p, u8 $a, u8 $r, u8 $g, u8 $b
export const kPaintSetColorARGB = 0x2e;

// Description: PaintSetStrokeWidth         paint %p, f32 $width
export const kPaintSetStrokeWidth = 0x2f;

// Description: PaintSetStrokeMiter         paint %p, f32 $miter
export const kPaintSetStrokeMiter = 0x30;

// Description: PaintSetStrokeCap           paint %p, paintcap $cap
export const kPaintSetStrokeCap = 0x31;

// Description: PaintSetStrokeJoin          paint %p, paintjoin $join
export const kPaintSetStrokeJoin = 0x32;

// Description: PaintSetShader              paint %p, shader %shader
export const kPaintSetShader = 0x33;

} // namespace Opcode
export namespace Constants {
export const PAINTCAP_BUTT = 0x0;
export const PAINTCAP_ROUND = 0x1;
export const PAINTCAP_SQUARE = 0x2;
export const PAINTJOIN_MITER = 0x0;
export const PAINTJOIN_ROUND = 0x1;
export const PAINTJOIN_BEVEL = 0x2;
export const BLENDMODE_CLEAR = 0x0;
export const BLENDMODE_SRC = 0x1;
export const BLENDMODE_DST = 0x2;
export const BLENDMODE_SRC_OVER = 0x3;
export const BLENDMODE_DST_OVER = 0x4;
export const BLENDMODE_SRC_IN = 0x5;
export const BLENDMODE_DST_IN = 0x6;
export const BLENDMODE_SRC_OUT = 0x7;
export const BLENDMODE_DST_OUT = 0x8;
export const BLENDMODE_SRC_ATOP = 0x9;
export const BLENDMODE_DST_ATOP = 0xa;
export const BLENDMODE_XOR = 0xb;
export const BLENDMODE_PLUS = 0xc;
export const BLENDMODE_MODULATE = 0xd;
export const BLENDMODE_SCREEN = 0xe;
export const BLENDMODE_OVERLAY = 0xf;
export const BLENDMODE_DARKEN = 0x10;
export const BLENDMODE_LIGHTEN = 0x11;
export const BLENDMODE_COLOR_DODGE = 0x12;
export const BLENDMODE_COLOR_BURN = 0x13;
export const BLENDMODE_HARD_LIGHT = 0x14;
export const BLENDMODE_SOFT_LIGHT = 0x15;
export const BLENDMODE_DIFFERENCE = 0x16;
export const BLENDMODE_EXCLUSION = 0x17;
export const BLENDMODE_MULTIPLY = 0x18;
export const BLENDMODE_HUE = 0x19;
export const BLENDMODE_SATURATION = 0x1a;
export const BLENDMODE_COLOR = 0x1b;
export const BLENDMODE_LUMINOSITY = 0x1c;
export const TILEMODE_CLAMP = 0x0;
export const TILEMODE_REPEAT = 0x1;
export const TILEMODE_MIRROR = 0x2;
export const TILEMODE_DECAL = 0x3;
} // namespace Constants

export type MemOp = number;
export class ProtoCodeEmitter {
  private writer: IProtoBufferWriter;
  constructor(writer: IProtoBufferWriter) {
    this.writer = writer;
  }

  public emitSwitchNextBuffer(): void {
    this.writer.performPossibleBufferSwitching(2);
    this.writer.writeUint16Unsafe(Opcode.kSwitchNextBuffer | 0x0);
  }
  public emitCommandPoolEnd(): void {
    this.writer.performPossibleBufferSwitching(2);
    this.writer.writeUint16Unsafe(Opcode.kCommandPoolEnd | 0x0);
  }
  public emitDrawBounds(width: number, height: number): void {
    this.writer.performPossibleBufferSwitching(10);
    this.writer.writeUint16Unsafe(Opcode.kDrawBounds | 0x200);
    this.writer.writeFloat32Unsafe(width);
    this.writer.writeFloat32Unsafe(height);
  }
  public emitHeapClone(from: MemOp, key: MemOp): void {
    this.writer.performPossibleBufferSwitching(10);
    this.writer.writeUint16Unsafe(Opcode.kHeapClone | 0x200);
    this.writer.writeUint32Unsafe(from);
    this.writer.writeUint32Unsafe(key);
  }
  public emitHeapFree(key: MemOp): void {
    this.writer.performPossibleBufferSwitching(6);
    this.writer.writeUint16Unsafe(Opcode.kHeapFree | 0x100);
    this.writer.writeUint32Unsafe(key);
  }
  public emitHeapCreateU32Array(key: MemOp, size: number): void {
    this.writer.performPossibleBufferSwitching(10);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreateU32Array | 0x200);
    this.writer.writeUint32Unsafe(key);
    this.writer.writeUint32Unsafe(size);
  }
  public emitHeapCreateF32Array(key: MemOp, size: number): void {
    this.writer.performPossibleBufferSwitching(10);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreateF32Array | 0x200);
    this.writer.writeUint32Unsafe(key);
    this.writer.writeUint32Unsafe(size);
  }
  public emitHeapU32ArrayStore(array: MemOp, idx: number, value: number): void {
    this.writer.performPossibleBufferSwitching(14);
    this.writer.writeUint16Unsafe(Opcode.kHeapU32ArrayStore | 0x300);
    this.writer.writeUint32Unsafe(array);
    this.writer.writeUint32Unsafe(idx);
    this.writer.writeUint32Unsafe(value);
  }
  public emitHeapF32ArrayStore(array: MemOp, idx: number, value: number): void {
    this.writer.performPossibleBufferSwitching(14);
    this.writer.writeUint16Unsafe(Opcode.kHeapF32ArrayStore | 0x300);
    this.writer.writeUint32Unsafe(array);
    this.writer.writeUint32Unsafe(idx);
    this.writer.writeFloat32Unsafe(value);
  }
  public emitHeapCreateM44(key: MemOp): void {
    this.writer.performPossibleBufferSwitching(6);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreateM44 | 0x100);
    this.writer.writeUint32Unsafe(key);
  }
  public emitHeapCreatePaint(key: MemOp): void {
    this.writer.performPossibleBufferSwitching(6);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreatePaint | 0x100);
    this.writer.writeUint32Unsafe(key);
  }
  public emitHeapCreatePath(key: MemOp): void {
    this.writer.performPossibleBufferSwitching(6);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreatePath | 0x100);
    this.writer.writeUint32Unsafe(key);
  }
  public emitHeapCreateVector2(key: MemOp, x: number, y: number): void {
    this.writer.performPossibleBufferSwitching(14);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreateVector2 | 0x300);
    this.writer.writeUint32Unsafe(key);
    this.writer.writeFloat32Unsafe(x);
    this.writer.writeFloat32Unsafe(y);
  }
  public emitHeapCreateVector3(key: MemOp, x: number, y: number, z: number): void {
    this.writer.performPossibleBufferSwitching(18);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreateVector3 | 0x400);
    this.writer.writeUint32Unsafe(key);
    this.writer.writeFloat32Unsafe(x);
    this.writer.writeFloat32Unsafe(y);
    this.writer.writeFloat32Unsafe(z);
  }
  public emitHeapCreateVector4(key: MemOp, x: number, y: number, z: number, w: number): void {
    this.writer.performPossibleBufferSwitching(22);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreateVector4 | 0x500);
    this.writer.writeUint32Unsafe(key);
    this.writer.writeFloat32Unsafe(x);
    this.writer.writeFloat32Unsafe(y);
    this.writer.writeFloat32Unsafe(z);
    this.writer.writeFloat32Unsafe(w);
  }
  public emitHeapCreateRect(key: MemOp, x: number, y: number, w: number, h: number): void {
    this.writer.performPossibleBufferSwitching(22);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreateRect | 0x500);
    this.writer.writeUint32Unsafe(key);
    this.writer.writeFloat32Unsafe(x);
    this.writer.writeFloat32Unsafe(y);
    this.writer.writeFloat32Unsafe(w);
    this.writer.writeFloat32Unsafe(h);
  }
  public emitHeapCreateEmptyShader(key: MemOp): void {
    this.writer.performPossibleBufferSwitching(6);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreateEmptyShader | 0x100);
    this.writer.writeUint32Unsafe(key);
  }
  public emitHeapCreateColorShader(key: MemOp, color: number): void {
    this.writer.performPossibleBufferSwitching(10);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreateColorShader | 0x200);
    this.writer.writeUint32Unsafe(key);
    this.writer.writeUint32Unsafe(color);
  }
  public emitHeapCreateBlendShader(key: MemOp, blender: MemOp, dst: MemOp, src: MemOp): void {
    this.writer.performPossibleBufferSwitching(18);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreateBlendShader | 0x400);
    this.writer.writeUint32Unsafe(key);
    this.writer.writeUint32Unsafe(blender);
    this.writer.writeUint32Unsafe(dst);
    this.writer.writeUint32Unsafe(src);
  }
  public emitHeapCreateLinearGradientShader(key: MemOp, start: MemOp, end: MemOp, colors: MemOp, mode: number): void {
    this.writer.performPossibleBufferSwitching(19);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreateLinearGradientShader | 0x500);
    this.writer.writeUint32Unsafe(key);
    this.writer.writeUint32Unsafe(start);
    this.writer.writeUint32Unsafe(end);
    this.writer.writeUint32Unsafe(colors);
    this.writer.writeUint8Unsafe(mode);
  }
  public emitHeapCreateLinearGradientShader2(key: MemOp, start: MemOp, end: MemOp, colors: MemOp, pos: MemOp, mode: number): void {
    this.writer.performPossibleBufferSwitching(23);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreateLinearGradientShader2 | 0x600);
    this.writer.writeUint32Unsafe(key);
    this.writer.writeUint32Unsafe(start);
    this.writer.writeUint32Unsafe(end);
    this.writer.writeUint32Unsafe(colors);
    this.writer.writeUint32Unsafe(pos);
    this.writer.writeUint8Unsafe(mode);
  }
  public emitHeapCreateRadialGradientShader(key: MemOp, center: MemOp, radius: number, colors: MemOp, mode: number): void {
    this.writer.performPossibleBufferSwitching(19);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreateRadialGradientShader | 0x500);
    this.writer.writeUint32Unsafe(key);
    this.writer.writeUint32Unsafe(center);
    this.writer.writeFloat32Unsafe(radius);
    this.writer.writeUint32Unsafe(colors);
    this.writer.writeUint8Unsafe(mode);
  }
  public emitHeapCreateRadialGradientShader2(key: MemOp, center: MemOp, radius: number, colors: MemOp, pos: MemOp, mode: number): void {
    this.writer.performPossibleBufferSwitching(23);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreateRadialGradientShader2 | 0x600);
    this.writer.writeUint32Unsafe(key);
    this.writer.writeUint32Unsafe(center);
    this.writer.writeFloat32Unsafe(radius);
    this.writer.writeUint32Unsafe(colors);
    this.writer.writeUint32Unsafe(pos);
    this.writer.writeUint8Unsafe(mode);
  }
  public emitHeapCreateTwoPointConicalGradientShader(key: MemOp, start: MemOp, startRadius: number, end: MemOp, endRadius: number, colors: MemOp, pos: MemOp, mode: number): void {
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
  public emitHeapCreateSweepGradientShader(key: MemOp, cx: number, cy: number, colors: MemOp): void {
    this.writer.performPossibleBufferSwitching(18);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreateSweepGradientShader | 0x400);
    this.writer.writeUint32Unsafe(key);
    this.writer.writeFloat32Unsafe(cx);
    this.writer.writeFloat32Unsafe(cy);
    this.writer.writeUint32Unsafe(colors);
  }
  public emitHeapCreateSweepGradientShader2(key: MemOp, cx: number, cy: number, colors: MemOp, pos: MemOp): void {
    this.writer.performPossibleBufferSwitching(22);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreateSweepGradientShader2 | 0x500);
    this.writer.writeUint32Unsafe(key);
    this.writer.writeFloat32Unsafe(cx);
    this.writer.writeFloat32Unsafe(cy);
    this.writer.writeUint32Unsafe(colors);
    this.writer.writeUint32Unsafe(pos);
  }
  public emitHeapCreatePerlinNoiseFractalNoiseShader(key: MemOp, baseFreqX: number, baseFreqY: number, numOctaves: number, seed: number): void {
    this.writer.performPossibleBufferSwitching(22);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreatePerlinNoiseFractalNoiseShader | 0x500);
    this.writer.writeUint32Unsafe(key);
    this.writer.writeFloat32Unsafe(baseFreqX);
    this.writer.writeFloat32Unsafe(baseFreqY);
    this.writer.writeUint32Unsafe(numOctaves);
    this.writer.writeFloat32Unsafe(seed);
  }
  public emitHeapCreatePerlinNoiseTurbulenceShader(key: MemOp, baseFreqX: number, baseFreqY: number, numOctaves: number, seed: number): void {
    this.writer.performPossibleBufferSwitching(22);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreatePerlinNoiseTurbulenceShader | 0x500);
    this.writer.writeUint32Unsafe(key);
    this.writer.writeFloat32Unsafe(baseFreqX);
    this.writer.writeFloat32Unsafe(baseFreqY);
    this.writer.writeUint32Unsafe(numOctaves);
    this.writer.writeFloat32Unsafe(seed);
  }
  public emitHeapCreateModeBlender(key: MemOp, mode: number): void {
    this.writer.performPossibleBufferSwitching(7);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreateModeBlender | 0x200);
    this.writer.writeUint32Unsafe(key);
    this.writer.writeUint8Unsafe(mode);
  }
  public emitHeapCreateArithmeticBlender(key: MemOp, k1: number, k2: number, k3: number, k4: number, enforcePremul: number): void {
    this.writer.performPossibleBufferSwitching(23);
    this.writer.writeUint16Unsafe(Opcode.kHeapCreateArithmeticBlender | 0x600);
    this.writer.writeUint32Unsafe(key);
    this.writer.writeFloat32Unsafe(k1);
    this.writer.writeFloat32Unsafe(k2);
    this.writer.writeFloat32Unsafe(k3);
    this.writer.writeFloat32Unsafe(k4);
    this.writer.writeUint8Unsafe(enforcePremul);
  }
  public emitM44SetRows(m: MemOp, r0: MemOp, r1: MemOp, r2: MemOp, r3: MemOp): void {
    this.writer.performPossibleBufferSwitching(22);
    this.writer.writeUint16Unsafe(Opcode.kM44SetRows | 0x500);
    this.writer.writeUint32Unsafe(m);
    this.writer.writeUint32Unsafe(r0);
    this.writer.writeUint32Unsafe(r1);
    this.writer.writeUint32Unsafe(r2);
    this.writer.writeUint32Unsafe(r3);
  }
  public emitM44SetCols(m: MemOp, c0: MemOp, c1: MemOp, c2: MemOp, c3: MemOp): void {
    this.writer.performPossibleBufferSwitching(22);
    this.writer.writeUint16Unsafe(Opcode.kM44SetCols | 0x500);
    this.writer.writeUint32Unsafe(m);
    this.writer.writeUint32Unsafe(c0);
    this.writer.writeUint32Unsafe(c1);
    this.writer.writeUint32Unsafe(c2);
    this.writer.writeUint32Unsafe(c3);
  }
  public emitM44SetTranslate(m: MemOp, x: number, y: number, z: number): void {
    this.writer.performPossibleBufferSwitching(18);
    this.writer.writeUint16Unsafe(Opcode.kM44SetTranslate | 0x400);
    this.writer.writeUint32Unsafe(m);
    this.writer.writeFloat32Unsafe(x);
    this.writer.writeFloat32Unsafe(y);
    this.writer.writeFloat32Unsafe(z);
  }
  public emitM44SetScale(m: MemOp, x: number, y: number, z: number): void {
    this.writer.performPossibleBufferSwitching(18);
    this.writer.writeUint16Unsafe(Opcode.kM44SetScale | 0x400);
    this.writer.writeUint32Unsafe(m);
    this.writer.writeFloat32Unsafe(x);
    this.writer.writeFloat32Unsafe(y);
    this.writer.writeFloat32Unsafe(z);
  }
  public emitM44SetRectToRect(m: MemOp, src: MemOp, dst: MemOp): void {
    this.writer.performPossibleBufferSwitching(14);
    this.writer.writeUint16Unsafe(Opcode.kM44SetRectToRect | 0x300);
    this.writer.writeUint32Unsafe(m);
    this.writer.writeUint32Unsafe(src);
    this.writer.writeUint32Unsafe(dst);
  }
  public emitM44SetLookAt(m: MemOp, eye: MemOp, center: MemOp, up: MemOp): void {
    this.writer.performPossibleBufferSwitching(18);
    this.writer.writeUint16Unsafe(Opcode.kM44SetLookAt | 0x400);
    this.writer.writeUint32Unsafe(m);
    this.writer.writeUint32Unsafe(eye);
    this.writer.writeUint32Unsafe(center);
    this.writer.writeUint32Unsafe(up);
  }
  public emitM44SetPerspective(m: MemOp, near: number, far: number, angle: number): void {
    this.writer.performPossibleBufferSwitching(18);
    this.writer.writeUint16Unsafe(Opcode.kM44SetPerspective | 0x400);
    this.writer.writeUint32Unsafe(m);
    this.writer.writeFloat32Unsafe(near);
    this.writer.writeFloat32Unsafe(far);
    this.writer.writeFloat32Unsafe(angle);
  }
  public emitM44SetIdentity(m: MemOp): void {
    this.writer.performPossibleBufferSwitching(6);
    this.writer.writeUint16Unsafe(Opcode.kM44SetIdentity | 0x100);
    this.writer.writeUint32Unsafe(m);
  }
  public emitM44Concat(m: MemOp, other: MemOp): void {
    this.writer.performPossibleBufferSwitching(10);
    this.writer.writeUint16Unsafe(Opcode.kM44Concat | 0x200);
    this.writer.writeUint32Unsafe(m);
    this.writer.writeUint32Unsafe(other);
  }
  public emitPaintReset(p: MemOp): void {
    this.writer.performPossibleBufferSwitching(6);
    this.writer.writeUint16Unsafe(Opcode.kPaintReset | 0x100);
    this.writer.writeUint32Unsafe(p);
  }
  public emitPaintSetAntialias(p: MemOp, aa: number): void {
    this.writer.performPossibleBufferSwitching(7);
    this.writer.writeUint16Unsafe(Opcode.kPaintSetAntialias | 0x200);
    this.writer.writeUint32Unsafe(p);
    this.writer.writeUint8Unsafe(aa);
  }
  public emitPaintSetDither(p: MemOp, dither: number): void {
    this.writer.performPossibleBufferSwitching(7);
    this.writer.writeUint16Unsafe(Opcode.kPaintSetDither | 0x200);
    this.writer.writeUint32Unsafe(p);
    this.writer.writeUint8Unsafe(dither);
  }
  public emitPaintSetStyleStroke(p: MemOp, stroke: number): void {
    this.writer.performPossibleBufferSwitching(7);
    this.writer.writeUint16Unsafe(Opcode.kPaintSetStyleStroke | 0x200);
    this.writer.writeUint32Unsafe(p);
    this.writer.writeUint8Unsafe(stroke);
  }
  public emitPaintSetColor(p: MemOp, color: number): void {
    this.writer.performPossibleBufferSwitching(10);
    this.writer.writeUint16Unsafe(Opcode.kPaintSetColor | 0x200);
    this.writer.writeUint32Unsafe(p);
    this.writer.writeUint32Unsafe(color);
  }
  public emitPaintSetAlphaf(p: MemOp, alpha: number): void {
    this.writer.performPossibleBufferSwitching(10);
    this.writer.writeUint16Unsafe(Opcode.kPaintSetAlphaf | 0x200);
    this.writer.writeUint32Unsafe(p);
    this.writer.writeFloat32Unsafe(alpha);
  }
  public emitPaintSetColorARGB(p: MemOp, a: number, r: number, g: number, b: number): void {
    this.writer.performPossibleBufferSwitching(10);
    this.writer.writeUint16Unsafe(Opcode.kPaintSetColorARGB | 0x500);
    this.writer.writeUint32Unsafe(p);
    this.writer.writeUint8Unsafe(a);
    this.writer.writeUint8Unsafe(r);
    this.writer.writeUint8Unsafe(g);
    this.writer.writeUint8Unsafe(b);
  }
  public emitPaintSetStrokeWidth(p: MemOp, width: number): void {
    this.writer.performPossibleBufferSwitching(10);
    this.writer.writeUint16Unsafe(Opcode.kPaintSetStrokeWidth | 0x200);
    this.writer.writeUint32Unsafe(p);
    this.writer.writeFloat32Unsafe(width);
  }
  public emitPaintSetStrokeMiter(p: MemOp, miter: number): void {
    this.writer.performPossibleBufferSwitching(10);
    this.writer.writeUint16Unsafe(Opcode.kPaintSetStrokeMiter | 0x200);
    this.writer.writeUint32Unsafe(p);
    this.writer.writeFloat32Unsafe(miter);
  }
  public emitPaintSetStrokeCap(p: MemOp, cap: number): void {
    this.writer.performPossibleBufferSwitching(7);
    this.writer.writeUint16Unsafe(Opcode.kPaintSetStrokeCap | 0x200);
    this.writer.writeUint32Unsafe(p);
    this.writer.writeUint8Unsafe(cap);
  }
  public emitPaintSetStrokeJoin(p: MemOp, join: number): void {
    this.writer.performPossibleBufferSwitching(7);
    this.writer.writeUint16Unsafe(Opcode.kPaintSetStrokeJoin | 0x200);
    this.writer.writeUint32Unsafe(p);
    this.writer.writeUint8Unsafe(join);
  }
  public emitPaintSetShader(p: MemOp, shader: MemOp): void {
    this.writer.performPossibleBufferSwitching(10);
    this.writer.writeUint16Unsafe(Opcode.kPaintSetShader | 0x200);
    this.writer.writeUint32Unsafe(p);
    this.writer.writeUint32Unsafe(shader);
  }
}
