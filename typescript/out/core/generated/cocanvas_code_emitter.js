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
    // Description: HeapCreateSamplingOptions   any %key, filtermode $filter, mipmapmode $mipmap
    Opcode.kHeapCreateSamplingOptions = 0xc;
    // Description: HeapCreateSamplingOptionsCubicMitchell any %key
    Opcode.kHeapCreateSamplingOptionsCubicMitchell = 0xd;
    // Description: HeapCreateSamplingOptionsCubicCatmullRom any %key
    Opcode.kHeapCreateSamplingOptionsCubicCatmullRom = 0xe;
    // Description: HeapCreatePath              any %key
    Opcode.kHeapCreatePath = 0xf;
    // Description: HeapCreateVector2           any %key, f32 $x, f32 $y
    Opcode.kHeapCreateVector2 = 0x10;
    // Description: HeapCreateVector3           any %key, f32 $x, f32 $y, f32 $z
    Opcode.kHeapCreateVector3 = 0x11;
    // Description: HeapCreateVector4           any %key, f32 $x, f32 $y, f32 $z, f32 $w
    Opcode.kHeapCreateVector4 = 0x12;
    // Description: HeapCreateRect              any %key, f32 $x, f32 $y, f32 $w, f32 $h
    Opcode.kHeapCreateRect = 0x13;
    // Description: HeapCreateRegion            any %key
    Opcode.kHeapCreateRegion = 0x14;
    // Description: HeapCreateRegion2           any %key, rect %rect
    Opcode.kHeapCreateRegion2 = 0x15;
    // Description: RegionOpRect                region %region, regionop $op, rect %rect
    Opcode.kRegionOpRect = 0x16;
    // Description: RegionOpRegion              region %region, regionop $op, region %rgn
    Opcode.kRegionOpRegion = 0x17;
    // Description: HeapCreateEmptyShader       any %key
    Opcode.kHeapCreateEmptyShader = 0x18;
    // Description: HeapCreateColorShader       any %key, u32 $color
    Opcode.kHeapCreateColorShader = 0x19;
    // Description: HeapCreateBlendShader       any %key, blender %blender, shader %dst, shader %src
    Opcode.kHeapCreateBlendShader = 0x1a;
    // Description: HeapCreateLinearGradientShader any %key, vec2 %start, vec2 %end, u32array %colors, f32array? %pos, tilemode $mode
    Opcode.kHeapCreateLinearGradientShader = 0x1b;
    // Description: HeapCreateRadialGradientShader any %key, vec2 %center, f32 $radius, u32array %colors, f32array? %pos, tilemode $mode
    Opcode.kHeapCreateRadialGradientShader = 0x1c;
    // Description: HeapCreateTwoPointConicalGradientShader any %key, vec2 %start, f32 $startRadius, vec2 %end, f32 $endRadius, u32array %colors, f32array %pos, tilemode $mode
    Opcode.kHeapCreateTwoPointConicalGradientShader = 0x1d;
    // Description: HeapCreateSweepGradientShader any %key, f32 $cx, f32 $cy, u32array %colors, f32array? %pos
    Opcode.kHeapCreateSweepGradientShader = 0x1e;
    // Description: HeapCreatePerlinNoiseFractalNoiseShader any %key, f32 $baseFreqX, f32 $baseFreqY, u32 $numOctaves, f32 $seed
    Opcode.kHeapCreatePerlinNoiseFractalNoiseShader = 0x1f;
    // Description: HeapCreatePerlinNoiseTurbulenceShader any %key, f32 $baseFreqX, f32 $baseFreqY, u32 $numOctaves, f32 $seed
    Opcode.kHeapCreatePerlinNoiseTurbulenceShader = 0x20;
    // Description: HeapCreateModeBlender       any %key, blendmode $mode
    Opcode.kHeapCreateModeBlender = 0x21;
    // Description: HeapCreateArithmeticBlender any %key, f32 $k1, f32 $k2, f32 $k3, f32 $k4, u8 $enforcePremul
    Opcode.kHeapCreateArithmeticBlender = 0x22;
    // Description: HeapCreateAlphaThresholdImageFilter any %key, region %region, f32 $innerMin, f32 $outerMax, imagefilter? %input, rect? %crop
    Opcode.kHeapCreateAlphaThresholdImageFilter = 0x23;
    // Description: HeapCreateArithmeticImageFilter any %key, f32 $k1, f32 $k2, f32 $k3, f32 $k4, u8 $enforcePMColor, imagefilter? %bg, imagefilter? %fg, rect? %crop
    Opcode.kHeapCreateArithmeticImageFilter = 0x24;
    // Description: HeapCreateBlendImageFilter  any %key, blender %blender, imagefilter? %bg, imagefilter? %fg, rect? %crop
    Opcode.kHeapCreateBlendImageFilter = 0x25;
    // Description: HeapCreateBlurImageFilter   any %key, f32 $sigmaX, f32 $sigmaY, tilemode $mode, imagefilter? %input, rect? %crop
    Opcode.kHeapCreateBlurImageFilter = 0x26;
    // Description: HeapCreateColorFilterImageFilter any %key, colorfilter %cf, imagefilter? %input, rect? %crop
    Opcode.kHeapCreateColorFilterImageFilter = 0x27;
    // Description: HeapCreateComposeImageFilter any %key, imagefilter %outer, imagefilter %inner
    Opcode.kHeapCreateComposeImageFilter = 0x28;
    // Description: HeapCreateDisplacementMapImageFilter any %key, colorchannel $xselector, colorchannel $yselector, f32 $scale, imagefilter? %displacement, imagefilter %color, rect? %crop
    Opcode.kHeapCreateDisplacementMapImageFilter = 0x29;
    // Description: HeapCreateDropShadowImageFilter any %key, f32 $dx, f32 $dy, f32 $sigmaX, f32 $sigmaY, u32 $color, imagefilter? %input, rect? %crop
    Opcode.kHeapCreateDropShadowImageFilter = 0x2a;
    // Description: HeapCreateDropShadowOnlyImageFilter any %key, f32 $dx, f32 $dy, f32 $sigmaX, f32 $sigmaY, u32 $color, imagefilter? %input, rect? %crop
    Opcode.kHeapCreateDropShadowOnlyImageFilter = 0x2b;
    // Description: HeapCreateMagnifierImageFilter any %key, rect %srcRect, f32 $inset, imagefilter? %input, rect? %crop
    Opcode.kHeapCreateMagnifierImageFilter = 0x2c;
    // Description: HeapCreateMatrixConvolutionImageFilter any %key, vec2 %kernelSize, f32array %kernel, f32 $gain, f32 $bias, vec2 %kernelOffset, tilemode $mode, u8 $useAlpha, imagefilter? %input, rect? %crop
    Opcode.kHeapCreateMatrixConvolutionImageFilter = 0x2d;
    // Description: HeapCreateMatrixTransformImageFilter any %key, mat4x4 %matrix, samplingoptions %sampling, imagefilter? %input
    Opcode.kHeapCreateMatrixTransformImageFilter = 0x2e;
    // Description: HeapCreateOffsetImageFilter any %key, f32 $dx, f32 $dy, imagefilter? %input, rect? %crop
    Opcode.kHeapCreateOffsetImageFilter = 0x2f;
    // Description: HeapCreateShaderImageFilter any %key, shader %shader, u8 $dither, rect? %crop
    Opcode.kHeapCreateShaderImageFilter = 0x30;
    // Description: HeapCreateTileImageFilter   any %key, rect %src, rect %dst, imagefilter? %input
    Opcode.kHeapCreateTileImageFilter = 0x31;
    // Description: HeapCreateDilateImageFilter any %key, f32 $radiusX, f32 $radiusY, imagefilter? %input, rect? %crop
    Opcode.kHeapCreateDilateImageFilter = 0x32;
    // Description: HeapCreateErodeImageFilter  any %key, f32 $radiusX, f32 $radiusY, imagefilter? %input, rect? %crop
    Opcode.kHeapCreateErodeImageFilter = 0x33;
    // Description: M44SetRows                  mat4x4 %m, vec4 %r0, vec4 %r1, vec4 %r2, vec4 %r3
    Opcode.kM44SetRows = 0x34;
    // Description: M44SetCols                  mat4x4 %m, vec4 %c0, vec4 %c1, vec4 %c2, vec4 %c3
    Opcode.kM44SetCols = 0x35;
    // Description: M44SetTranslate             mat4x4 %m, f32 $x, f32 $y, f32 $z
    Opcode.kM44SetTranslate = 0x36;
    // Description: M44SetScale                 mat4x4 %m, f32 $x, f32 $y, f32 $z
    Opcode.kM44SetScale = 0x37;
    // Description: M44SetRectToRect            mat4x4 %m, rect %src, rect %dst
    Opcode.kM44SetRectToRect = 0x38;
    // Description: M44SetLookAt                mat4x4 %m, vec3 %eye, vec3 %center, vec3 %up
    Opcode.kM44SetLookAt = 0x39;
    // Description: M44SetPerspective           mat4x4 %m, f32 $near, f32 $far, f32 $angle
    Opcode.kM44SetPerspective = 0x3a;
    // Description: M44SetIdentity              mat4x4 %m
    Opcode.kM44SetIdentity = 0x3b;
    // Description: M44Concat                   mat4x4 %m, mat4x4 %other
    Opcode.kM44Concat = 0x3c;
    // Description: PaintReset                  paint %p
    Opcode.kPaintReset = 0x3d;
    // Description: PaintSetAntialias           paint %p, u8 $aa
    Opcode.kPaintSetAntialias = 0x3e;
    // Description: PaintSetDither              paint %p, u8 $dither
    Opcode.kPaintSetDither = 0x3f;
    // Description: PaintSetStyleStroke         paint %p, u8 $stroke
    Opcode.kPaintSetStyleStroke = 0x40;
    // Description: PaintSetColor               paint %p, u32 $color
    Opcode.kPaintSetColor = 0x41;
    // Description: PaintSetAlphaf              paint %p, f32 $alpha
    Opcode.kPaintSetAlphaf = 0x42;
    // Description: PaintSetColorARGB           paint %p, u8 $a, u8 $r, u8 $g, u8 $b
    Opcode.kPaintSetColorARGB = 0x43;
    // Description: PaintSetStrokeWidth         paint %p, f32 $width
    Opcode.kPaintSetStrokeWidth = 0x44;
    // Description: PaintSetStrokeMiter         paint %p, f32 $miter
    Opcode.kPaintSetStrokeMiter = 0x45;
    // Description: PaintSetStrokeCap           paint %p, paintcap $cap
    Opcode.kPaintSetStrokeCap = 0x46;
    // Description: PaintSetStrokeJoin          paint %p, paintjoin $join
    Opcode.kPaintSetStrokeJoin = 0x47;
    // Description: PaintSetShader              paint %p, shader %shader
    Opcode.kPaintSetShader = 0x48;
    // Description: PaintSetColorFilter         paint %p, colorfilter %f
    Opcode.kPaintSetColorFilter = 0x49;
    // Description: PaintSetBlendMode           paint %p, blendmode $mode
    Opcode.kPaintSetBlendMode = 0x4a;
    // Description: PaintSetBlender             paint %p, blender %blender
    Opcode.kPaintSetBlender = 0x4b;
    // Description: PaintSetPathEffect          paint %p, patheffect %effect
    Opcode.kPaintSetPathEffect = 0x4c;
    // Description: PaintSetMaskFilter          paint %p, maskfilter %filter
    Opcode.kPaintSetMaskFilter = 0x4d;
    // Description: PaintSetImageFilter         paint %p, imagefilter %filter
    Opcode.kPaintSetImageFilter = 0x4e;
    // Description: PathSetPathFillType         path %p, pathfilltype $type
    Opcode.kPathSetPathFillType = 0x4f;
    // Description: PathToggleInverseFillType   path %p
    Opcode.kPathToggleInverseFillType = 0x50;
    // Description: PathReset                   path %p
    Opcode.kPathReset = 0x51;
    // Description: PathMoveTo                  path %p, f32 $x, f32 $y
    Opcode.kPathMoveTo = 0x52;
    // Description: PathRMoveTo                 path %p, f32 $dx, f32 $dy
    Opcode.kPathRMoveTo = 0x53;
    // Description: PathLineTo                  path %p, f32 $x, f32 $y
    Opcode.kPathLineTo = 0x54;
    // Description: PathRLineTo                 path %p, f32 $dx, f32 $dy
    Opcode.kPathRLineTo = 0x55;
    // Description: PathQuadTo                  path %p, f32 $x1, f32 $y1, f32 $x2, f32 $y2
    Opcode.kPathQuadTo = 0x56;
    // Description: PathRQuadTo                 path %p, f32 $dx1, f32 $dy1, f32 $dx2, f32 $dy2
    Opcode.kPathRQuadTo = 0x57;
    // Description: PathConicTo                 path %p, f32 $x1, f32 $y1, f32 $x2, f32 $y2, f32 $w
    Opcode.kPathConicTo = 0x58;
    // Description: PathRConicTo                path %p, f32 $dx1, f32 $dy1, f32 $dx2, f32 $dy2, f32 $w
    Opcode.kPathRConicTo = 0x59;
    // Description: PathCubicTo                 path %p, f32 $x1, f32 $y1, f32 $x2, f32 $y2, f32 $x3, f32 $y3
    Opcode.kPathCubicTo = 0x5a;
    // Description: PathRCubicTo                path %p, f32 $dx1, f32 $dy1, f32 $dx2, f32 $dy2, f32 $dx3, f32 $dy3
    Opcode.kPathRCubicTo = 0x5b;
    // Description: PathRectArcTo               path %p, rect %oval, f32 $startAngle, f32 $sweepAngle, u8 $forceMoveTo
    Opcode.kPathRectArcTo = 0x5c;
    // Description: PathTangentArcTo            path %p, f32 $x1, f32 $y1, f32 $x2, f32 $y2, f32 $radius
    Opcode.kPathTangentArcTo = 0x5d;
    // Description: PathRotateArcTo             path %p, f32 $rx, f32 $ry, f32 $xAxisRotate, u8 $largeArc, u8 $ccwSweep, f32 $x, f32 $y
    Opcode.kPathRotateArcTo = 0x5e;
    // Description: PathRRotateArcTo            path %p, f32 $rx, f32 $ry, f32 $xAxisRotate, u8 $largeArc, u8 $ccwSweep, f32 $dx, f32 $dy
    Opcode.kPathRRotateArcTo = 0x5f;
    // Description: PathClose                   path %p
    Opcode.kPathClose = 0x60;
    // Description: PathAddRect                 path %p, rect %rect, u8 $ccwDir
    Opcode.kPathAddRect = 0x61;
    // Description: PathAddOval                 path %p, rect %oval, u8 $ccwDir
    Opcode.kPathAddOval = 0x62;
    // Description: PathAddCircle               path %p, f32 $x, f32 $y, f32 $radius, u8 $ccwDir
    Opcode.kPathAddCircle = 0x63;
    // Description: PathAddArc                  path %p, rect %oval, f32 $startAngle, f32 $sweepAngle
    Opcode.kPathAddArc = 0x64;
    // Description: PathAddUniformRoundRect     path %p, rect %rect, f32 $rx, f32 $ry, u8 $ccwDir
    Opcode.kPathAddUniformRoundRect = 0x65;
    // Description: PathAddRoundRect            path %p, rect %rect, f32array %radii, u8 $ccwDir
    Opcode.kPathAddRoundRect = 0x66;
    // Description: PathAddPoly                 path %p, f32array %pts, u8 $close
    Opcode.kPathAddPoly = 0x67;
    // Description: PathTransform               path %p, mat4x4 %mat, path %dst, u8 $applyPerspectiveClip
    Opcode.kPathTransform = 0x68;
    // Description: Save
    Opcode.kSave = 0x69;
    // Description: Restore
    Opcode.kRestore = 0x6a;
    // Description: SetMatrix                   mat4x4 %matrix
    Opcode.kSetMatrix = 0x6b;
    // Description: Clear                       u32 $color
    Opcode.kClear = 0x6c;
    // Description: DrawPaint                   paint %p
    Opcode.kDrawPaint = 0x6d;
    // Description: DrawPath                    path %path, paint %paint
    Opcode.kDrawPath = 0x6e;
    // Description: DrawImage                   image %image, f32 $left, f32 $top, samplingoptions? %sampling, paint? %paint
    Opcode.kDrawImage = 0x6f;
    // Description: DrawImageRect               image %image, rect %src, rect %dst, samplingoptions? %sampling, paint? %paint, u8 $fastConstraint
    Opcode.kDrawImageRect = 0x70;
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
    Constants.REGIONOP_DIFFERENCE = 0x0;
    Constants.REGIONOP_INTERSECT = 0x1;
    Constants.REGIONOP_UNION = 0x2;
    Constants.REGIONOP_XOR = 0x3;
    Constants.REGIONOP_REVERSE_DIFFERENCE = 0x4;
    Constants.REGIONOP_REPLACE = 0x5;
    Constants.COLORCHANNEL_R = 0x0;
    Constants.COLORCHANNEL_G = 0x1;
    Constants.COLORCHANNEL_B = 0x2;
    Constants.COLORCHANNEL_A = 0x3;
    Constants.FILTERMODE_NEAREST = 0x0;
    Constants.FILTERMODE_LINEAR = 0x1;
    Constants.MIPMAPMODE_NONE = 0x0;
    Constants.MIPMAPMODE_NEAREST = 0x1;
    Constants.MIPMAPMODE_LINEAR = 0x2;
    Constants.PATHFILLTYPE_WINDING = 0x0;
    Constants.PATHFILLTYPE_EVEN_ODD = 0x1;
    Constants.PATHFILLTYPE_INVERSE_WINDING = 0x2;
    Constants.PATHFILLTYPE_INVERSE_EVEN_ODD = 0x3;
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
    emitHeapCreateSamplingOptions(key, filter, mipmap) {
        this.writer.performPossibleBufferSwitching(8);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateSamplingOptions | 0x300);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint8Unsafe(filter);
        this.writer.writeUint8Unsafe(mipmap);
    }
    emitHeapCreateSamplingOptionsCubicMitchell(key) {
        this.writer.performPossibleBufferSwitching(6);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateSamplingOptionsCubicMitchell | 0x100);
        this.writer.writeUint32Unsafe(key);
    }
    emitHeapCreateSamplingOptionsCubicCatmullRom(key) {
        this.writer.performPossibleBufferSwitching(6);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateSamplingOptionsCubicCatmullRom | 0x100);
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
    emitHeapCreateRegion(key) {
        this.writer.performPossibleBufferSwitching(6);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateRegion | 0x100);
        this.writer.writeUint32Unsafe(key);
    }
    emitHeapCreateRegion2(key, rect) {
        this.writer.performPossibleBufferSwitching(10);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateRegion2 | 0x200);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint32Unsafe(rect);
    }
    emitRegionOpRect(region, op, rect) {
        this.writer.performPossibleBufferSwitching(11);
        this.writer.writeUint16Unsafe(Opcode.kRegionOpRect | 0x300);
        this.writer.writeUint32Unsafe(region);
        this.writer.writeUint8Unsafe(op);
        this.writer.writeUint32Unsafe(rect);
    }
    emitRegionOpRegion(region, op, rgn) {
        this.writer.performPossibleBufferSwitching(11);
        this.writer.writeUint16Unsafe(Opcode.kRegionOpRegion | 0x300);
        this.writer.writeUint32Unsafe(region);
        this.writer.writeUint8Unsafe(op);
        this.writer.writeUint32Unsafe(rgn);
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
    emitHeapCreateLinearGradientShader(key, start, end, colors, pos, mode) {
        this.writer.performPossibleBufferSwitching(23);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateLinearGradientShader | 0x600);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint32Unsafe(start);
        this.writer.writeUint32Unsafe(end);
        this.writer.writeUint32Unsafe(colors);
        this.writer.writeUint32Unsafe(pos);
        this.writer.writeUint8Unsafe(mode);
    }
    emitHeapCreateRadialGradientShader(key, center, radius, colors, pos, mode) {
        this.writer.performPossibleBufferSwitching(23);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateRadialGradientShader | 0x600);
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
    emitHeapCreateSweepGradientShader(key, cx, cy, colors, pos) {
        this.writer.performPossibleBufferSwitching(22);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateSweepGradientShader | 0x500);
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
    emitHeapCreateAlphaThresholdImageFilter(key, region, innerMin, outerMax, input, crop) {
        this.writer.performPossibleBufferSwitching(26);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateAlphaThresholdImageFilter | 0x600);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint32Unsafe(region);
        this.writer.writeFloat32Unsafe(innerMin);
        this.writer.writeFloat32Unsafe(outerMax);
        this.writer.writeUint32Unsafe(input);
        this.writer.writeUint32Unsafe(crop);
    }
    emitHeapCreateArithmeticImageFilter(key, k1, k2, k3, k4, enforcePMColor, bg, fg, crop) {
        this.writer.performPossibleBufferSwitching(35);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateArithmeticImageFilter | 0x900);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeFloat32Unsafe(k1);
        this.writer.writeFloat32Unsafe(k2);
        this.writer.writeFloat32Unsafe(k3);
        this.writer.writeFloat32Unsafe(k4);
        this.writer.writeUint8Unsafe(enforcePMColor);
        this.writer.writeUint32Unsafe(bg);
        this.writer.writeUint32Unsafe(fg);
        this.writer.writeUint32Unsafe(crop);
    }
    emitHeapCreateBlendImageFilter(key, blender, bg, fg, crop) {
        this.writer.performPossibleBufferSwitching(22);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateBlendImageFilter | 0x500);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint32Unsafe(blender);
        this.writer.writeUint32Unsafe(bg);
        this.writer.writeUint32Unsafe(fg);
        this.writer.writeUint32Unsafe(crop);
    }
    emitHeapCreateBlurImageFilter(key, sigmaX, sigmaY, mode, input, crop) {
        this.writer.performPossibleBufferSwitching(23);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateBlurImageFilter | 0x600);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeFloat32Unsafe(sigmaX);
        this.writer.writeFloat32Unsafe(sigmaY);
        this.writer.writeUint8Unsafe(mode);
        this.writer.writeUint32Unsafe(input);
        this.writer.writeUint32Unsafe(crop);
    }
    emitHeapCreateColorFilterImageFilter(key, cf, input, crop) {
        this.writer.performPossibleBufferSwitching(18);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateColorFilterImageFilter | 0x400);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint32Unsafe(cf);
        this.writer.writeUint32Unsafe(input);
        this.writer.writeUint32Unsafe(crop);
    }
    emitHeapCreateComposeImageFilter(key, outer, inner) {
        this.writer.performPossibleBufferSwitching(14);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateComposeImageFilter | 0x300);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint32Unsafe(outer);
        this.writer.writeUint32Unsafe(inner);
    }
    emitHeapCreateDisplacementMapImageFilter(key, xselector, yselector, scale, displacement, color, crop) {
        this.writer.performPossibleBufferSwitching(24);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateDisplacementMapImageFilter | 0x700);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint8Unsafe(xselector);
        this.writer.writeUint8Unsafe(yselector);
        this.writer.writeFloat32Unsafe(scale);
        this.writer.writeUint32Unsafe(displacement);
        this.writer.writeUint32Unsafe(color);
        this.writer.writeUint32Unsafe(crop);
    }
    emitHeapCreateDropShadowImageFilter(key, dx, dy, sigmaX, sigmaY, color, input, crop) {
        this.writer.performPossibleBufferSwitching(34);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateDropShadowImageFilter | 0x800);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeFloat32Unsafe(dx);
        this.writer.writeFloat32Unsafe(dy);
        this.writer.writeFloat32Unsafe(sigmaX);
        this.writer.writeFloat32Unsafe(sigmaY);
        this.writer.writeUint32Unsafe(color);
        this.writer.writeUint32Unsafe(input);
        this.writer.writeUint32Unsafe(crop);
    }
    emitHeapCreateDropShadowOnlyImageFilter(key, dx, dy, sigmaX, sigmaY, color, input, crop) {
        this.writer.performPossibleBufferSwitching(34);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateDropShadowOnlyImageFilter | 0x800);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeFloat32Unsafe(dx);
        this.writer.writeFloat32Unsafe(dy);
        this.writer.writeFloat32Unsafe(sigmaX);
        this.writer.writeFloat32Unsafe(sigmaY);
        this.writer.writeUint32Unsafe(color);
        this.writer.writeUint32Unsafe(input);
        this.writer.writeUint32Unsafe(crop);
    }
    emitHeapCreateMagnifierImageFilter(key, srcRect, inset, input, crop) {
        this.writer.performPossibleBufferSwitching(22);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateMagnifierImageFilter | 0x500);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint32Unsafe(srcRect);
        this.writer.writeFloat32Unsafe(inset);
        this.writer.writeUint32Unsafe(input);
        this.writer.writeUint32Unsafe(crop);
    }
    emitHeapCreateMatrixConvolutionImageFilter(key, kernelSize, kernel, gain, bias, kernelOffset, mode, useAlpha, input, crop) {
        this.writer.performPossibleBufferSwitching(36);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateMatrixConvolutionImageFilter | 0xa00);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint32Unsafe(kernelSize);
        this.writer.writeUint32Unsafe(kernel);
        this.writer.writeFloat32Unsafe(gain);
        this.writer.writeFloat32Unsafe(bias);
        this.writer.writeUint32Unsafe(kernelOffset);
        this.writer.writeUint8Unsafe(mode);
        this.writer.writeUint8Unsafe(useAlpha);
        this.writer.writeUint32Unsafe(input);
        this.writer.writeUint32Unsafe(crop);
    }
    emitHeapCreateMatrixTransformImageFilter(key, matrix, sampling, input) {
        this.writer.performPossibleBufferSwitching(18);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateMatrixTransformImageFilter | 0x400);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint32Unsafe(matrix);
        this.writer.writeUint32Unsafe(sampling);
        this.writer.writeUint32Unsafe(input);
    }
    emitHeapCreateOffsetImageFilter(key, dx, dy, input, crop) {
        this.writer.performPossibleBufferSwitching(22);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateOffsetImageFilter | 0x500);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeFloat32Unsafe(dx);
        this.writer.writeFloat32Unsafe(dy);
        this.writer.writeUint32Unsafe(input);
        this.writer.writeUint32Unsafe(crop);
    }
    emitHeapCreateShaderImageFilter(key, shader, dither, crop) {
        this.writer.performPossibleBufferSwitching(15);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateShaderImageFilter | 0x400);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint32Unsafe(shader);
        this.writer.writeUint8Unsafe(dither);
        this.writer.writeUint32Unsafe(crop);
    }
    emitHeapCreateTileImageFilter(key, src, dst, input) {
        this.writer.performPossibleBufferSwitching(18);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateTileImageFilter | 0x400);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeUint32Unsafe(src);
        this.writer.writeUint32Unsafe(dst);
        this.writer.writeUint32Unsafe(input);
    }
    emitHeapCreateDilateImageFilter(key, radiusX, radiusY, input, crop) {
        this.writer.performPossibleBufferSwitching(22);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateDilateImageFilter | 0x500);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeFloat32Unsafe(radiusX);
        this.writer.writeFloat32Unsafe(radiusY);
        this.writer.writeUint32Unsafe(input);
        this.writer.writeUint32Unsafe(crop);
    }
    emitHeapCreateErodeImageFilter(key, radiusX, radiusY, input, crop) {
        this.writer.performPossibleBufferSwitching(22);
        this.writer.writeUint16Unsafe(Opcode.kHeapCreateErodeImageFilter | 0x500);
        this.writer.writeUint32Unsafe(key);
        this.writer.writeFloat32Unsafe(radiusX);
        this.writer.writeFloat32Unsafe(radiusY);
        this.writer.writeUint32Unsafe(input);
        this.writer.writeUint32Unsafe(crop);
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
    emitPaintSetColorFilter(p, f) {
        this.writer.performPossibleBufferSwitching(10);
        this.writer.writeUint16Unsafe(Opcode.kPaintSetColorFilter | 0x200);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint32Unsafe(f);
    }
    emitPaintSetBlendMode(p, mode) {
        this.writer.performPossibleBufferSwitching(7);
        this.writer.writeUint16Unsafe(Opcode.kPaintSetBlendMode | 0x200);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint8Unsafe(mode);
    }
    emitPaintSetBlender(p, blender) {
        this.writer.performPossibleBufferSwitching(10);
        this.writer.writeUint16Unsafe(Opcode.kPaintSetBlender | 0x200);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint32Unsafe(blender);
    }
    emitPaintSetPathEffect(p, effect) {
        this.writer.performPossibleBufferSwitching(10);
        this.writer.writeUint16Unsafe(Opcode.kPaintSetPathEffect | 0x200);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint32Unsafe(effect);
    }
    emitPaintSetMaskFilter(p, filter) {
        this.writer.performPossibleBufferSwitching(10);
        this.writer.writeUint16Unsafe(Opcode.kPaintSetMaskFilter | 0x200);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint32Unsafe(filter);
    }
    emitPaintSetImageFilter(p, filter) {
        this.writer.performPossibleBufferSwitching(10);
        this.writer.writeUint16Unsafe(Opcode.kPaintSetImageFilter | 0x200);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint32Unsafe(filter);
    }
    emitPathSetPathFillType(p, type) {
        this.writer.performPossibleBufferSwitching(7);
        this.writer.writeUint16Unsafe(Opcode.kPathSetPathFillType | 0x200);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint8Unsafe(type);
    }
    emitPathToggleInverseFillType(p) {
        this.writer.performPossibleBufferSwitching(6);
        this.writer.writeUint16Unsafe(Opcode.kPathToggleInverseFillType | 0x100);
        this.writer.writeUint32Unsafe(p);
    }
    emitPathReset(p) {
        this.writer.performPossibleBufferSwitching(6);
        this.writer.writeUint16Unsafe(Opcode.kPathReset | 0x100);
        this.writer.writeUint32Unsafe(p);
    }
    emitPathMoveTo(p, x, y) {
        this.writer.performPossibleBufferSwitching(14);
        this.writer.writeUint16Unsafe(Opcode.kPathMoveTo | 0x300);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeFloat32Unsafe(x);
        this.writer.writeFloat32Unsafe(y);
    }
    emitPathRMoveTo(p, dx, dy) {
        this.writer.performPossibleBufferSwitching(14);
        this.writer.writeUint16Unsafe(Opcode.kPathRMoveTo | 0x300);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeFloat32Unsafe(dx);
        this.writer.writeFloat32Unsafe(dy);
    }
    emitPathLineTo(p, x, y) {
        this.writer.performPossibleBufferSwitching(14);
        this.writer.writeUint16Unsafe(Opcode.kPathLineTo | 0x300);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeFloat32Unsafe(x);
        this.writer.writeFloat32Unsafe(y);
    }
    emitPathRLineTo(p, dx, dy) {
        this.writer.performPossibleBufferSwitching(14);
        this.writer.writeUint16Unsafe(Opcode.kPathRLineTo | 0x300);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeFloat32Unsafe(dx);
        this.writer.writeFloat32Unsafe(dy);
    }
    emitPathQuadTo(p, x1, y1, x2, y2) {
        this.writer.performPossibleBufferSwitching(22);
        this.writer.writeUint16Unsafe(Opcode.kPathQuadTo | 0x500);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeFloat32Unsafe(x1);
        this.writer.writeFloat32Unsafe(y1);
        this.writer.writeFloat32Unsafe(x2);
        this.writer.writeFloat32Unsafe(y2);
    }
    emitPathRQuadTo(p, dx1, dy1, dx2, dy2) {
        this.writer.performPossibleBufferSwitching(22);
        this.writer.writeUint16Unsafe(Opcode.kPathRQuadTo | 0x500);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeFloat32Unsafe(dx1);
        this.writer.writeFloat32Unsafe(dy1);
        this.writer.writeFloat32Unsafe(dx2);
        this.writer.writeFloat32Unsafe(dy2);
    }
    emitPathConicTo(p, x1, y1, x2, y2, w) {
        this.writer.performPossibleBufferSwitching(26);
        this.writer.writeUint16Unsafe(Opcode.kPathConicTo | 0x600);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeFloat32Unsafe(x1);
        this.writer.writeFloat32Unsafe(y1);
        this.writer.writeFloat32Unsafe(x2);
        this.writer.writeFloat32Unsafe(y2);
        this.writer.writeFloat32Unsafe(w);
    }
    emitPathRConicTo(p, dx1, dy1, dx2, dy2, w) {
        this.writer.performPossibleBufferSwitching(26);
        this.writer.writeUint16Unsafe(Opcode.kPathRConicTo | 0x600);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeFloat32Unsafe(dx1);
        this.writer.writeFloat32Unsafe(dy1);
        this.writer.writeFloat32Unsafe(dx2);
        this.writer.writeFloat32Unsafe(dy2);
        this.writer.writeFloat32Unsafe(w);
    }
    emitPathCubicTo(p, x1, y1, x2, y2, x3, y3) {
        this.writer.performPossibleBufferSwitching(30);
        this.writer.writeUint16Unsafe(Opcode.kPathCubicTo | 0x700);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeFloat32Unsafe(x1);
        this.writer.writeFloat32Unsafe(y1);
        this.writer.writeFloat32Unsafe(x2);
        this.writer.writeFloat32Unsafe(y2);
        this.writer.writeFloat32Unsafe(x3);
        this.writer.writeFloat32Unsafe(y3);
    }
    emitPathRCubicTo(p, dx1, dy1, dx2, dy2, dx3, dy3) {
        this.writer.performPossibleBufferSwitching(30);
        this.writer.writeUint16Unsafe(Opcode.kPathRCubicTo | 0x700);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeFloat32Unsafe(dx1);
        this.writer.writeFloat32Unsafe(dy1);
        this.writer.writeFloat32Unsafe(dx2);
        this.writer.writeFloat32Unsafe(dy2);
        this.writer.writeFloat32Unsafe(dx3);
        this.writer.writeFloat32Unsafe(dy3);
    }
    emitPathRectArcTo(p, oval, startAngle, sweepAngle, forceMoveTo) {
        this.writer.performPossibleBufferSwitching(19);
        this.writer.writeUint16Unsafe(Opcode.kPathRectArcTo | 0x500);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint32Unsafe(oval);
        this.writer.writeFloat32Unsafe(startAngle);
        this.writer.writeFloat32Unsafe(sweepAngle);
        this.writer.writeUint8Unsafe(forceMoveTo);
    }
    emitPathTangentArcTo(p, x1, y1, x2, y2, radius) {
        this.writer.performPossibleBufferSwitching(26);
        this.writer.writeUint16Unsafe(Opcode.kPathTangentArcTo | 0x600);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeFloat32Unsafe(x1);
        this.writer.writeFloat32Unsafe(y1);
        this.writer.writeFloat32Unsafe(x2);
        this.writer.writeFloat32Unsafe(y2);
        this.writer.writeFloat32Unsafe(radius);
    }
    emitPathRotateArcTo(p, rx, ry, xAxisRotate, largeArc, ccwSweep, x, y) {
        this.writer.performPossibleBufferSwitching(28);
        this.writer.writeUint16Unsafe(Opcode.kPathRotateArcTo | 0x800);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeFloat32Unsafe(rx);
        this.writer.writeFloat32Unsafe(ry);
        this.writer.writeFloat32Unsafe(xAxisRotate);
        this.writer.writeUint8Unsafe(largeArc);
        this.writer.writeUint8Unsafe(ccwSweep);
        this.writer.writeFloat32Unsafe(x);
        this.writer.writeFloat32Unsafe(y);
    }
    emitPathRRotateArcTo(p, rx, ry, xAxisRotate, largeArc, ccwSweep, dx, dy) {
        this.writer.performPossibleBufferSwitching(28);
        this.writer.writeUint16Unsafe(Opcode.kPathRRotateArcTo | 0x800);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeFloat32Unsafe(rx);
        this.writer.writeFloat32Unsafe(ry);
        this.writer.writeFloat32Unsafe(xAxisRotate);
        this.writer.writeUint8Unsafe(largeArc);
        this.writer.writeUint8Unsafe(ccwSweep);
        this.writer.writeFloat32Unsafe(dx);
        this.writer.writeFloat32Unsafe(dy);
    }
    emitPathClose(p) {
        this.writer.performPossibleBufferSwitching(6);
        this.writer.writeUint16Unsafe(Opcode.kPathClose | 0x100);
        this.writer.writeUint32Unsafe(p);
    }
    emitPathAddRect(p, rect, ccwDir) {
        this.writer.performPossibleBufferSwitching(11);
        this.writer.writeUint16Unsafe(Opcode.kPathAddRect | 0x300);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint32Unsafe(rect);
        this.writer.writeUint8Unsafe(ccwDir);
    }
    emitPathAddOval(p, oval, ccwDir) {
        this.writer.performPossibleBufferSwitching(11);
        this.writer.writeUint16Unsafe(Opcode.kPathAddOval | 0x300);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint32Unsafe(oval);
        this.writer.writeUint8Unsafe(ccwDir);
    }
    emitPathAddCircle(p, x, y, radius, ccwDir) {
        this.writer.performPossibleBufferSwitching(19);
        this.writer.writeUint16Unsafe(Opcode.kPathAddCircle | 0x500);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeFloat32Unsafe(x);
        this.writer.writeFloat32Unsafe(y);
        this.writer.writeFloat32Unsafe(radius);
        this.writer.writeUint8Unsafe(ccwDir);
    }
    emitPathAddArc(p, oval, startAngle, sweepAngle) {
        this.writer.performPossibleBufferSwitching(18);
        this.writer.writeUint16Unsafe(Opcode.kPathAddArc | 0x400);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint32Unsafe(oval);
        this.writer.writeFloat32Unsafe(startAngle);
        this.writer.writeFloat32Unsafe(sweepAngle);
    }
    emitPathAddUniformRoundRect(p, rect, rx, ry, ccwDir) {
        this.writer.performPossibleBufferSwitching(19);
        this.writer.writeUint16Unsafe(Opcode.kPathAddUniformRoundRect | 0x500);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint32Unsafe(rect);
        this.writer.writeFloat32Unsafe(rx);
        this.writer.writeFloat32Unsafe(ry);
        this.writer.writeUint8Unsafe(ccwDir);
    }
    emitPathAddRoundRect(p, rect, radii, ccwDir) {
        this.writer.performPossibleBufferSwitching(15);
        this.writer.writeUint16Unsafe(Opcode.kPathAddRoundRect | 0x400);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint32Unsafe(rect);
        this.writer.writeUint32Unsafe(radii);
        this.writer.writeUint8Unsafe(ccwDir);
    }
    emitPathAddPoly(p, pts, close) {
        this.writer.performPossibleBufferSwitching(11);
        this.writer.writeUint16Unsafe(Opcode.kPathAddPoly | 0x300);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint32Unsafe(pts);
        this.writer.writeUint8Unsafe(close);
    }
    emitPathTransform(p, mat, dst, applyPerspectiveClip) {
        this.writer.performPossibleBufferSwitching(15);
        this.writer.writeUint16Unsafe(Opcode.kPathTransform | 0x400);
        this.writer.writeUint32Unsafe(p);
        this.writer.writeUint32Unsafe(mat);
        this.writer.writeUint32Unsafe(dst);
        this.writer.writeUint8Unsafe(applyPerspectiveClip);
    }
    emitSave() {
        this.writer.performPossibleBufferSwitching(2);
        this.writer.writeUint16Unsafe(Opcode.kSave | 0x0);
    }
    emitRestore() {
        this.writer.performPossibleBufferSwitching(2);
        this.writer.writeUint16Unsafe(Opcode.kRestore | 0x0);
    }
    emitSetMatrix(matrix) {
        this.writer.performPossibleBufferSwitching(6);
        this.writer.writeUint16Unsafe(Opcode.kSetMatrix | 0x100);
        this.writer.writeUint32Unsafe(matrix);
    }
    emitClear(color) {
        this.writer.performPossibleBufferSwitching(6);
        this.writer.writeUint16Unsafe(Opcode.kClear | 0x100);
        this.writer.writeUint32Unsafe(color);
    }
    emitDrawPaint(p) {
        this.writer.performPossibleBufferSwitching(6);
        this.writer.writeUint16Unsafe(Opcode.kDrawPaint | 0x100);
        this.writer.writeUint32Unsafe(p);
    }
    emitDrawPath(path, paint) {
        this.writer.performPossibleBufferSwitching(10);
        this.writer.writeUint16Unsafe(Opcode.kDrawPath | 0x200);
        this.writer.writeUint32Unsafe(path);
        this.writer.writeUint32Unsafe(paint);
    }
    emitDrawImage(image, left, top, sampling, paint) {
        this.writer.performPossibleBufferSwitching(22);
        this.writer.writeUint16Unsafe(Opcode.kDrawImage | 0x500);
        this.writer.writeUint32Unsafe(image);
        this.writer.writeFloat32Unsafe(left);
        this.writer.writeFloat32Unsafe(top);
        this.writer.writeUint32Unsafe(sampling);
        this.writer.writeUint32Unsafe(paint);
    }
    emitDrawImageRect(image, src, dst, sampling, paint, fastConstraint) {
        this.writer.performPossibleBufferSwitching(23);
        this.writer.writeUint16Unsafe(Opcode.kDrawImageRect | 0x600);
        this.writer.writeUint32Unsafe(image);
        this.writer.writeUint32Unsafe(src);
        this.writer.writeUint32Unsafe(dst);
        this.writer.writeUint32Unsafe(sampling);
        this.writer.writeUint32Unsafe(paint);
        this.writer.writeUint8Unsafe(fastConstraint);
    }
}
