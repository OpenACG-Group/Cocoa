import * as _event from 'event';
type i8 = number;
type u8 = number;
type i16 = number;
type u16 = number;
type i32 = number;
type u32 = number;
type i64 = number;
type u64 = number;
type f32 = number;
type f64 = number;
type SkSLChild = (Shader | ColorFilter | Blender);
type Color4f = [f32, f32, f32, f32];
type ColorU32 = u32;
export declare enum ImageMemoryMutability {
    MutableAndCopy,
    ImmutableAndShare,
}
export declare enum VecDegenerate {
    Discard,
    Div,
    Mul,
}
export declare enum ColorType {
    Unknown,
    Alpha_8,
    RGB_565,
    ARGB_4444,
    RGBA_8888,
    RGB_888x,
    BGRA_8888,
    RGBA_1010102,
    BGRA_1010102,
    RGB_101010x,
    BGR_101010x,
    BGR_101010x_XR,
    RGBA_10x6,
    Gray_8,
    RGBA_F16Norm,
    RGBA_F32,
    R8G8_unorm,
    A16_float,
    R16G16_float,
    A16_unorm,
    R16G16_unorm,
    R16G16B16A16_unorm,
    SRGBA_8888,
    R8_unorm,
    N32,
}
export declare enum AlphaType {
    Unknown,
    Opaque,
    Premul,
    Unpremul,
}
export declare enum FilterMode {
    Nearest,
    Linear,
}
export declare enum MipmapMode {
    None,
    Nearest,
    Linear,
}
export declare enum PixelGeometry {
    Unknown,
    RGB_H,
    BGR_H,
    RGB_V,
    BGR_V,
}
export declare enum ImageRescaleGamma {
    Src,
    Linear,
}
export declare enum ImageRescaleMode {
    Nearest,
    Linear,
    RepeatedLinear,
    RepeatedCubic,
}
export declare enum YUVColorSpace {
    JPEG_Full,
    Rec601_Limited,
    Rec709_Full,
    Rec709_Limited,
    BT2020_8bit_Full,
    BT2020_8bit_Limited,
    BT2020_10bit_Full,
    BT2020_10bit_Limited,
    BT2020_12bit_Full,
    BT2020_12bit_Limited,
    Identity,
}
export declare enum BlendMode {
    Clear,
    Src,
    Dst,
    SrcOver,
    DstOver,
    SrcIn,
    DstIn,
    SrcOut,
    DstOut,
    SrcATop,
    DstATop,
    Xor,
    Plus,
    Modulate,
    Screen,
    Overlay,
    Darken,
    Lighten,
    ColorDodge,
    ColorBurn,
    HardLight,
    SoftLight,
    Difference,
    Exclusion,
    Multiply,
    Hue,
    Saturation,
    Color,
    Luminosity,
}
export declare enum TextureCompressionType {
    ETC2_RGB8_UNORM,
    BC1_RGB8_UNORM,
    BC1_RGBA8_UNORM,
    None,
}
export declare enum Style {
    Fill,
    Stroke,
    StrokeAndFill,
}
export declare enum LineCap {
    Butt,
    Round,
    Square,
    Default,
}
export declare enum LineJoin {
    Miter,
    Round,
    Bevel,
    Default,
}
export declare enum SaveLayerFlags {
    PreserveLCDText,
    InitWithPrevious,
    F16ColorType,
}
export declare enum ClipOp {
    Difference,
    Intersect,
}
export declare enum PointMode {
    Points,
    Lines,
    Polygon,
}
export declare enum PathFillType {
    Winding,
    EvenOdd,
    InverseWinding,
    InverseEvenOdd,
}
export declare enum PathDirection {
    CW,
    CCW,
}
export declare enum PathSegmentMask {
    Line,
    Quad,
    Conic,
    Cubic,
}
export declare enum PathVerb {
    Move,
    Line,
    Quad,
    Conic,
    Cubic,
    Close,
}
export declare enum AddPathMode {
    Append,
    Extend,
}
export declare enum Path1DEffectStyle {
    Translate,
    Rotate,
    Morph,
}
export declare enum MapDirection {
    Forward,
    Reverse,
}
export declare enum TileMode {
    Clamp,
    Repeat,
    Mirror,
    Decal,
}
export declare enum ColorChannel {
    R,
    G,
    B,
    A,
}
export declare enum HighContrastInvertStyle {
    No,
    Brightness,
    Lightness,
}
export declare enum GradientInterpColorSpace {
    Destination,
    SRGBLinear,
    Lab,
    OKLab,
    OKLabGamutMap,
    LCH,
    OKLCH,
    OKLCHGamutMap,
    SRGB,
    HSL,
    HWB,
}
export declare enum GradientInterpHueMethod {
    Shorter,
    Longer,
    Increasing,
    Decreasing,
}
export declare enum SkSLUniformType {
    Float,
    Float2,
    Float3,
    Float4,
    Float2x2,
    Float3x3,
    Float4x4,
    Int,
    Int2,
    Int3,
    Int4,
}
export declare enum SkSLUniformFlags {
    Array,
    Color,
    Vertex,
    Fragment,
    HalfPrecision,
}
export declare enum SkSLChildType {
    Shader,
    ColorFilter,
    Blender,
}
export declare enum FontWeight {
    Invisible,
    Thin,
    ExtraLight,
    Light,
    Normal,
    Medium,
    SemiBold,
    Bold,
    ExtraBold,
    Black,
    ExtraBlack,
}
export declare enum FontWidth {
    UltraCondensed,
    ExtraCondensed,
    Condensed,
    SemiCondensed,
    Normal,
    SemiExpanded,
    Expanded,
    ExtraExpanded,
    UltraExpanded,
}
export declare enum FontSlant {
    Upright,
    Italic,
    Oblique,
}
export declare enum TypefaceSerializeBehavior {
    IncludeData,
    NotIncludeData,
    IncludeDataIfLocal,
}
export declare enum TextEncoding {
    UTF8,
    UTF16,
    UTF32,
    GlyphID,
}
export declare enum FontEdging {
    Alias,
    AntiAlias,
    SubpixelAntiAlias,
}
export declare enum FontHinting {
    None,
    Slight,
    Normal,
    Full,
}
export declare enum VertexMode {
    Triangles,
    TriangleStrip,
    TriangleFan,
}
export interface Serializers {
    onImage?: ((image: Image) => (null | Uint8Array));
    onPicture?: ((picture: Picture) => (null | Uint8Array));
    onTypeface?: ((typeface: Typeface) => (null | Uint8Array));
}
export interface Deserializers {
    onImage?: ((buffer: Uint8Array) => (null | Image));
    onPicture?: ((buffer: Uint8Array) => (null | Picture));
    onTypeface?: ((buffer: Uint8Array) => (null | Typeface));
    allowSkSL?: boolean;
}
export interface FontStyle {
    weight: i32;
    width: i32;
    slant: FontSlant;
}
export interface GradientInterpolation {
    inPremul?: boolean;
    colorSpace?: GradientInterpColorSpace;
    hueMethod?: GradientInterpHueMethod;
}
export interface ImageBatchResult {
    success: boolean;
    error?: string;
    image?: Image;
}
export interface MakeWithFilterResult {
    outSubset: Rect;
    offset: [i32, i32];
    image: Image;
}
export interface RRect {
    rect: Rect;
    uniformRadii: boolean;
    borderRadii: f32[];
}
export interface SamplingOptions {
    readonly maxAniso?: i32;
    readonly useCubic?: boolean;
    readonly cubicB?: f32;
    readonly cubicC?: f32;
    readonly filter?: FilterMode;
    readonly mipmap?: MipmapMode;
}
export interface MeasureTextResult {
    advanceWidth: f32;
    bounds?: Rect;
}
export interface MeasureGlyphsResult {
    widths?: f32[];
    bounds?: Rect[];
}
export interface FontMetrics {
    hasUnderlineThickness: boolean;
    hasUnderlinePosition: boolean;
    hasStrikeoutThickness: boolean;
    hasStrikeoutPosition: boolean;
    hasBounds: boolean;
    top: f32;
    ascent: f32;
    descent: f32;
    bottom: f32;
    leading: f32;
    avgCharWidth: f32;
    maxCharWidth: f32;
    xMin: f32;
    xMax: f32;
    xHeight: f32;
    capHeight: f32;
    underlineThickness: f32;
    underlinePosition: f32;
    strikeoutThickness: f32;
    strikeoutPosition: f32;
}
export interface SurfaceProps {
    useDeviceIndependentFonts?: boolean;
    dynamicMSAA?: boolean;
    alwaysDither?: boolean;
    pixelGeometry?: PixelGeometry;
}
export interface HighContrastConfig {
    grayscale: boolean;
    invertStyle: HighContrastInvertStyle;
    contrast: f32;
}
export interface SkSLReflectUniform {
    readonly name: string;
    readonly offset: u32;
    readonly sizeInBytes: u32;
    readonly type: SkSLUniformType;
    readonly count: i32;
    readonly flags: u32;
}
export interface SkSLReflectChild {
    readonly name: string;
    readonly type: SkSLChildType;
    readonly index: i32;
}
export interface Pixmap {
    pixels: Uint8Array;
    rowBytes: u64;
    imageInfo: ImageInfo;
}
export interface LocalizedFamilyName {
    name: string;
    language: string;
}
export interface VariationFontAxisInfo {
    min: f32;
    def: f32;
    max: f32;
    hidden: boolean;
}
export interface FontArguments {
    collectionIndex?: i32;
    variationDesignPosition?: Map<string, f32>;
    paletteIndex?: i32;
    paletteOverrides?: Map<i32, ColorU32>;
}
export interface SaveLayerRec {
    bounds?: Rect;
    paint?: Paint;
    flags?: SaveLayerFlags;
    backdrop?: ImageFilter;
    filters?: ImageFilter[];
}
export class Flattenable {
    protected constructor();
    serialize(serializers: (null | Serializers)): Uint8Array;
}
export class FontStyleSet {
    private constructor();
    readonly count: i32;
    static CreateEmpty(): FontStyleSet;
    getStyle(index: i32): [FontStyle, string];
    createTypeface(index: i32): Typeface;
    matchStyle(pattern: FontStyle): (Typeface | null);
}
export class FontMgr {
    private constructor();
    static GetGlobal(): FontMgr;
    static GetEmpty(): FontMgr;
    countFamilies(): i32;
    getFamilyName(index: i32): string;
    createStyleSet(index: i32): FontStyleSet;
    matchFamily(familyName: (string | null)): FontStyleSet;
    matchFamilyStyle(familyName: (string | null), style: FontStyle): (Typeface | null);
    matchFamilyStyleCharacter(familyName: (string | null), style: FontStyle, bcp47: string[], unicodeChar: i32): (Typeface | null);
    makeFromData(data: Uint8Array, ttcIndex: i32): Typeface;
    makeFromFile(path: string, ttcIndex: i32): Typeface;
}
export class ImageAsyncReadResult {
    private constructor();
    readonly count: i32;
    dispose(): void;
    dimensionsOf(plane: i32): [i32, i32];
    bytesPerPixelOf(plane: i32): i32;
    readPlane(plane: i32, dst: Uint8Array, dstRowBytes: i32, srcRect: Rect): void;
}
export class PictureRecorder {
    constructor();
    beginRecording(bounds: Rect): Canvas;
    getRecordingCanvas(): (null | Canvas);
    finishRecordingAsPicture(): Picture;
    finishRecordingAsPictureWithCull(cull: Rect): Picture;
}
export class Shader extends Flattenable {
    private constructor();
    readonly isOpaque: boolean;
    static Empty(): Shader;
    static Color(color: Color4f, cs: ColorSpace): Shader;
    static Blend(mode: BlendMode, dst: Shader, src: Shader): Shader;
    static Blender(blender: Blender, dst: Shader, src: Shader): Shader;
    static CoordClamp(shader: Shader, subset: Rect): Shader;
    static Image(image: Image, tmx: TileMode, tmy: TileMode, options: SamplingOptions, localMatrix: (Mat3x3 | null)): Shader;
    static RawImage(image: Image, tmx: TileMode, tmy: TileMode, options: SamplingOptions, localMatrix: (Mat3x3 | null)): Shader;
    static LinearGradient(pts: [f32, f32, f32, f32], colors: Color4f[], colorSpace: ColorSpace, pos: f32[], mode: TileMode, interpolation: GradientInterpolation, localMatrix: (Mat3x3 | null)): Shader;
    static RadialGradient(center: [f32, f32], radius: f32, colors: Color4f[], colorSpace: ColorSpace, pos: f32[], mode: TileMode, interpolation: GradientInterpolation, localMatrix: (Mat3x3 | null)): Shader;
    static TwoPointConicalGradient(start: [f32, f32], startRadius: f32, end: [f32, f32], endRadius: f32, colors: Color4f[], colorSpace: ColorSpace, pos: f32[], mode: TileMode, interpolation: GradientInterpolation, localMatrix: (Mat3x3 | null)): Shader;
    static SweepGradient(center: [f32, f32], startAngle: f32, endAngle: f32, colors: Color4f[], colorSpace: ColorSpace, pos: f32[], mode: TileMode, interpolation: GradientInterpolation, localMatrix: (Mat3x3 | null)): Shader;
    static FractalNoise(baseFrequencyX: f32, baseFrequencyY: f32, numOctaves: i32, seed: f32, tileSize: ([f32, f32] | null)): Shader;
    static Turbulence(baseFrequencyX: f32, baseFrequencyY: f32, numOctaves: i32, seed: f32, tileSize: ([f32, f32] | null)): Shader;
    static Deserialize(memory: Uint8Array, deserializers: (Deserializers | null)): (null | Shader);
    dispose(): void;
    makeWithLocalMatrix(matrix: Mat3x3): Shader;
    makeWithColorFilter(cf: ColorFilter): Shader;
    makeWithWorkingColorSpace(cs: ColorSpace): Shader;
}
export class PathBuilder {
    constructor();
    clone(): PathBuilder;
    computeBounds(): Rect;
    snapshot(): Path;
    detach(): Path;
    setFillType(ft: PathFillType): PathBuilder;
    setIsVolatile(value: boolean): PathBuilder;
    reset(): PathBuilder;
    moveTo(x: f32, y: f32): PathBuilder;
    lineTo(x: f32, y: f32): PathBuilder;
    quadTo(x1: f32, y1: f32, x2: f32, y2: f32): PathBuilder;
    conicTo(x1: f32, y1: f32, x2: f32, y2: f32, w: f32): PathBuilder;
    cubicTo(x1: f32, y1: f32, x2: f32, y2: f32, x3: f32, y3: f32): PathBuilder;
    close(): PathBuilder;
    rLineTo(x: f32, y: f32): PathBuilder;
    rQuadTo(x1: f32, y1: f32, x2: f32, y2: f32): PathBuilder;
    rConicTo(x1: f32, y1: f32, x2: f32, y2: f32, w: f32): PathBuilder;
    rCubicTo(x1: f32, y1: f32, x2: f32, y2: f32, x3: f32, y3: f32): PathBuilder;
    ovalArcTo(oval: Rect, startAngleDeg: f32, sweepAngleDeg: f32, forceMoveTo: boolean): PathBuilder;
    tangentArcTo(x1: f32, y1: f32, x2: f32, y2: f32, radius: f32): PathBuilder;
    rotateOvalArcTo(rx: f32, ry: f32, rotationDeg: f32, largeArc: boolean, sweep: PathDirection, x1: f32, y1: f32): PathBuilder;
    addArc(oval: Rect, startAngleDeg: f32, sweepAngleDeg: f32): PathBuilder;
    addRect(rect: Rect, dir: PathDirection, startIndex: u32): PathBuilder;
    addOval(oval: Rect, dir: PathDirection, startIndex: u32): PathBuilder;
    addRRect(rrect: RRect, dir: PathDirection, startIndex: u32): PathBuilder;
    addCircle(cx: f32, cy: f32, radius: f32, dir: PathDirection): PathBuilder;
    addPolygon(pts: Float32Array, isClosed: boolean): PathBuilder;
    addPath(path: Path): PathBuilder;
    incReserve(extraPtCount: i32, extraVerbCount: i32): PathBuilder;
    offset(dx: f32, dy: f32): PathBuilder;
    toggleInverseFillType(): PathBuilder;
}
export class Image {
    private constructor();
    readonly imageInfo: ImageInfo;
    readonly width: i32;
    readonly height: i32;
    readonly bounds: Rect;
    readonly uniqueID: u32;
    readonly hasMipmaps: boolean;
    readonly isProtected: boolean;
    readonly isLazyGenerated: boolean;
    static FromPixmap(mutability: ImageMemoryMutability, pixmap: Pixmap): Image;
    static FromCompressedTextureData(data: Uint8Array, width: i32, height: i32, type: TextureCompressionType): Image;
    static DeferredFromEncodedData(encoded: Uint8Array, alphaType: (AlphaType | null)): Image;
    static DeferredFromEncodedFile(path: string, alphaType: (AlphaType | null)): Image;
    static BatchFromEncodedFiles(paths: string[]): Promise<ImageBatchResult[]>;
    static DeferredFromPicture(picture: Picture, dimensions: [i32, i32], matrix: (null | Mat3x3), paint: (null | Paint), f16BitDepth: boolean, colorSpace: ColorSpace): Image;
    static MakeWithFilter(context: (null | GpuDirectContext), src: Image, filter: ImageFilter, subset: Rect, clipBounds: Rect): MakeWithFilterResult;
    dispose(): void;
    makeShader(tmx: TileMode, tmy: TileMode, sampling: SamplingOptions, localMatrix: (null | Mat3x3)): Shader;
    makeRawShader(tmx: TileMode, tmy: TileMode, sampling: SamplingOptions, localMatrix: (null | Mat3x3)): Shader;
    isTextureBacked(): boolean;
    textureSize(): u64;
    isValid(context: (GpuDirectContext | null)): boolean;
    readPixels(ctx: (GpuDirectContext | null), dst: Pixmap, srcX: i32, srcY: i32, allowCaching: boolean): boolean;
    asyncRescaleAndReadPixels(info: ImageInfo, srcRect: Rect, rescaleGamma: ImageRescaleGamma, rescaleMode: ImageRescaleMode): Promise<ImageAsyncReadResult>;
    asyncRescaleAndReadPixelsYUV420(yuvColorSpace: YUVColorSpace, dstColorSpace: ColorSpace, srcRect: Rect, dstSize: [i32, i32], rescaleGamma: ImageRescaleGamma, rescaleMode: ImageRescaleMode): Promise<ImageAsyncReadResult>;
    asyncRescaleAndReadPixelsYUVA420(yuvColorSpace: YUVColorSpace, dstColorSpace: ColorSpace, srcRect: Rect, dstSize: [i32, i32], rescaleGamma: ImageRescaleGamma, rescaleMode: ImageRescaleMode): Promise<ImageAsyncReadResult>;
    scalePixels(dst: Pixmap, sampling: SamplingOptions, allowCaching: boolean): boolean;
    makeSubset(context: (GpuDirectContext | null), subset: Rect): (Image | null);
    withDefaultMipmaps(): (Image | null);
    makeNonTextureImage(context: (GpuDirectContext | null)): (Image | null);
    makeRasterImage(context: (GpuDirectContext | null), allowCaching: boolean): (Image | null);
    makeColorSpace(context: (GpuDirectContext | null), target: ColorSpace): (Image | null);
    makeColorTypeAndColorSpace(context: (GpuDirectContext | null), targetColorType: ColorType, targetCS: ColorSpace): (Image | null);
    reinterpretColorSpace(cs: ColorSpace): (Image | null);
}
export class Paint {
    constructor();
    antiAlias: boolean;
    dither: boolean;
    style: Style;
    color: u32;
    alphaf: f32;
    alpha: u8;
    strokeWidth: f32;
    strokeMiter: f32;
    strokeCap: LineCap;
    strokeJoin: LineJoin;
    blendMode: (null | BlendMode);
    pathEffect: (null | PathEffect);
    imageFilter: (null | ImageFilter);
    colorFilter: (null | ColorFilter);
    blender: (null | Blender);
    shader: (null | Shader);
    clone(): Paint;
    equalTo(other: Paint): boolean;
    reset(): void;
    getColor4f(): Color4f;
    setColor4f(color: Color4f, cs: (ColorSpace | null)): void;
    nothingToDraw(): boolean;
    canComputeFastBounds(): boolean;
    computeFastBounds(orig: Rect): Rect;
}
export class PathEffect extends Flattenable {
    private constructor();
    static MakeSum(first: PathEffect, second: PathEffect): PathEffect;
    static MakeCompose(outer: PathEffect, inner: PathEffect): PathEffect;
    static Make1D(path: Path, advance: f32, phase: f32, style: Path1DEffectStyle): PathEffect;
    static MakeLine2D(width: f32, matrix: Mat3x3): PathEffect;
    static MakePath2D(matrix: Mat3x3, path: Path): PathEffect;
    static MakeCorner(radius: f32): PathEffect;
    static MakeTrim(startT: f32, stopT: f32, inverted: boolean): PathEffect;
    static MakeDiscrete(segLength: f32, dev: f32, seedAssist: u32): PathEffect;
    static MakeDash(intervals: Float32Array, phase: f32): PathEffect;
    static Deserialize(memory: Uint8Array, deserializers: (Deserializers | null)): (null | PathEffect);
}
export class ImageInfo {
    private constructor();
    readonly width: i32;
    readonly height: i32;
    readonly colorType: ColorType;
    readonly alphaType: AlphaType;
    readonly bytesPerPixel: i32;
    readonly shiftPerPixel: i32;
    readonly minRowBytes: u64;
    static Make(width: i32, height: i32, colorType: ColorType, alphaType: AlphaType, colorSpace: (ColorSpace | null)): ImageInfo;
    static MakeN32(width: i32, height: i32, alphaType: AlphaType, colorSpace: (ColorSpace | null)): ImageInfo;
    static MakeS32(width: i32, height: i32, alphaType: AlphaType): ImageInfo;
    static MakeN32Premul(width: i32, height: i32, colorSpace: (ColorSpace | null)): ImageInfo;
    static MakeA8(width: i32, height: i32): ImageInfo;
    static MakeUnknown(width: i32, height: i32): ImageInfo;
    isEmpty(): boolean;
    isOpaque(): boolean;
    refColorSpace(): (ColorSpace | null);
    gammaCloseToSRGB(): boolean;
    makeWH(width: i32, height: i32): ImageInfo;
    makeAlphaType(alphaType: AlphaType): ImageInfo;
    makeColorType(colorType: ColorType): ImageInfo;
    makeColorSpace(cs: (ColorSpace | null)): ImageInfo;
    computeOffset(x: i32, y: i32, rowBytes: u64): u64;
    equalsTo(other: ImageInfo): boolean;
    computeByteSize(rowBytes: u64): u64;
    computeMinByteSize(): u64;
    validRowBytes(rowBytes: u64): boolean;
    reset(): void;
}
export class CubicSamplers {
    private constructor();
    static Mitchell(): SamplingOptions;
    static CatmullRom(): SamplingOptions;
}
export class Font {
    constructor(typeface: Typeface, size: f32, scaleX: f32, skewX: f32);
    isForceAutoHinting: boolean;
    isEmbeddedBitmaps: boolean;
    isSubpixel: boolean;
    isLinearMetrics: boolean;
    isEmbolden: boolean;
    isBaselineSnap: boolean;
    edging: FontEdging;
    hinting: FontHinting;
    size: f32;
    scaleX: f32;
    skewX: f32;
    typeface: Typeface;
    readonly spacing: boolean;
    readonly metrics: FontMetrics;
    equalTo(font: Font): boolean;
    makeWithSize(size: f32): Font;
    measureText(text: string, requireBounds: boolean, paint: (Paint | null)): MeasureTextResult;
    measureGlyphs(glyphs: Uint16Array, requireWidths: boolean, requireBounds: boolean, paint: (Paint | null)): MeasureGlyphsResult;
    getPos(glyphs: Uint16Array, points: Float32Array, originX: f32, originY: f32): void;
    getXPos(glyphs: Uint16Array, xpos: Float32Array, originX: f32): void;
    getIntercepts(glyphs: Uint16Array, points: Float32Array, top: f32, bottom: f32, paint: (Paint | null)): f32[];
    getPath(glyph: u16, path: Path): boolean;
}
export class Surface {
    private constructor();
    readonly width: i32;
    readonly height: i32;
    readonly imageInfo: ImageInfo;
    readonly generationID: u32;
    readonly canvas: Canvas;
    static MakeRaster(info: ImageInfo, props: (null | SurfaceProps)): Surface;
    static MakeFromPixmap(pixmap: Pixmap, props: (null | SurfaceProps)): Surface;
    static MakeNull(width: i32, height: i32): Surface;
    /**
     * Destroy the Surface and free all its memory. The attached `Canvas` instance
     * will also be disposed automatically.
     */
    dispose(): void;
    notifyContentWillChange(discard: boolean): void;
    makeImageSnapshot(bounds: (Rect | null)): (Image | null);
    peekPixels(): (null | Pixmap);
    readPixels(dst: Pixmap, srcX: i32, srcY: i32): boolean;
    writePixels(src: Pixmap, srcX: i32, srcY: i32): void;
    asyncRescaleAndReadPixels(info: ImageInfo, srcRect: Rect, rescaleGamma: ImageRescaleGamma, rescaleMode: ImageRescaleMode): Promise<ImageAsyncReadResult>;
    asyncRescaleAndReadPixelsYUV420(yuvColorSpace: YUVColorSpace, dstColorSpace: ColorSpace, srcRect: Rect, dstSize: [i32, i32], rescaleGamma: ImageRescaleGamma, rescaleMode: ImageRescaleMode): Promise<ImageAsyncReadResult>;
    asyncRescaleAndReadPixelsYUVA420(yuvColorSpace: YUVColorSpace, dstColorSpace: ColorSpace, srcRect: Rect, dstSize: [i32, i32], rescaleGamma: ImageRescaleGamma, rescaleMode: ImageRescaleMode): Promise<ImageAsyncReadResult>;
}
export class Vec2 {
    constructor(x: f32, y: f32);
    readonly x: f32;
    readonly y: f32;
    static Dot(u: Vec2, v: Vec2): f32;
    static Cross(u: Vec2, v: Vec2): f32;
    static AngleCos(u: Vec2, v: Vec2): f32;
    static AngleSin(u: Vec2, v: Vec2): f32;
    length(): f32;
    lengthSquared(): f32;
    add(v: Vec2): Vec2;
    sub(v: Vec2): Vec2;
    dot(v: Vec2): f32;
    cross(v: Vec2): f32;
    normalize(): Vec2;
    neg(): Vec2;
    mul(lambda: f32): Vec2;
    rotate(rad: f32): Vec2;
    angleCos(v: Vec2): Vec2;
    angleSin(v: Vec2): Vec2;
    equalTo(other: Vec2): boolean;
    clone(): Vec2;
}
export class Vec3 {
    constructor(x: f32, y: f32, z: f32);
    readonly x: f32;
    readonly y: f32;
    readonly z: f32;
    static Dot(u: Vec3, v: Vec3): f32;
    static Cross(u: Vec3, v: Vec3): Vec3;
    static AngleCos(u: Vec3, v: Vec3): f32;
    static AngleSin(u: Vec3, v: Vec3): f32;
    length(): f32;
    lengthSquared(): f32;
    add(v: Vec3): Vec3;
    sub(v: Vec3): Vec3;
    dot(v: Vec3): f32;
    cross(v: Vec3): Vec3;
    normalize(): Vec3;
    neg(): Vec3;
    mul(lambda: f32): Vec3;
    angleCos(v: Vec3): f32;
    angleSin(v: Vec3): f32;
    equalTo(other: Vec3): boolean;
    clone(): Vec3;
    degenerate(opt: VecDegenerate): Vec2;
}
export class Vec4 {
    constructor(x: f32, y: f32, z: f32, w: f32);
    readonly x: f32;
    readonly y: f32;
    readonly z: f32;
    readonly w: f32;
    static Dot(a: f32, b: f32): f32;
    clone(): Vec4;
    add(v: Vec4): Vec4;
    sub(v: Vec4): Vec4;
    neg(): Vec4;
    mul(lambda: f32): Vec4;
    lengthSquared(): f32;
    length(): f32;
    normalize(): Vec4;
    dot(v: Vec4): f32;
    equalTo(v: Vec4): boolean;
    degenerate(opt: VecDegenerate): Vec3;
}
export class Rect {
    private constructor();
    readonly top: f32;
    readonly left: f32;
    readonly right: f32;
    readonly bottom: f32;
    readonly x: f32;
    readonly y: f32;
    readonly width: f32;
    readonly height: f32;
    readonly center: Vec2;
    readonly quadUpperLeft: Vec2;
    readonly quadUpperRight: Vec2;
    readonly quadLowerLeft: Vec2;
    readonly quadLowerRight: Vec2;
    static MakeXYWH(x: f32, y: f32, w: f32, h: f32): Rect;
    static MakeLTRB(l: f32, t: f32, r: f32, b: f32): Rect;
    static MakeWH(w: f32, h: f32): Rect;
    static MakeEmpty(): Rect;
    static Clone(from: Rect): Rect;
    static Union(a: Rect, b: Rect): Rect;
    static Intersect(a: Rect, b: Rect): Rect;
    equalTo(other: Rect): boolean;
    isEmpty(): boolean;
    makeWH(): Rect;
    clone(): Rect;
    union(other: Rect): Rect;
    makeOffset(dx: f32, dy: f32): Rect;
    makeOffsetv(offset: Vec2): Rect;
    makeInset(dx: f32, dy: f32): Rect;
    makeOutset(dx: f32, dy: f32): Rect;
    contains(x: f32, y: f32): boolean;
    containsRect(other: Rect): boolean;
    intersect(other: Rect): Rect;
}
export class Mat3x3 {
    private constructor();
    readonly underlyingArray: Float32Array;
    static Identity(): Mat3x3;
    static RowMajor(r: ArrayLike<f32>): Mat3x3;
    static ColMajor(r: ArrayLike<f32>): Mat3x3;
    static Rows(r0: Vec3, r1: Vec3, r2: Vec3): Mat3x3;
    static Cols(c0: Vec3, c1: Vec3, c2: Vec3): Mat3x3;
    static Translate(x: f32, y: f32): Mat3x3;
    static Scale(sx: f32, sy: f32): Mat3x3;
    static Rotate(center: Vec2, rad: f32): Mat3x3;
    static Concat(a: Mat3x3, b: Mat3x3): Mat3x3;
    equalTo(other: Mat3x3): boolean;
    clone(): Mat3x3;
    at(r: i32, c: i32): f32;
    setAt(r: i32, c: i32, value: f32): void;
    row(i: i32): Vec3;
    setRow(i: i32, r: Vec3): void;
    col(i: i32): Vec3;
    setCol(i: i32, c: Vec3): void;
    determinant(): f32;
    setIdentity(): Mat3x3;
    setTranslate(x: f32, y: f32): Mat3x3;
    setScale(x: f32, y: f32): Mat3x3;
    setRotate(center: Vec2, rad: f32): Mat3x3;
    setConcat(a: Mat3x3, b: Mat3x3): Mat3x3;
    preConcat(m: Mat3x3): Mat3x3;
    postConcat(m: Mat3x3): Mat3x3;
    isFinite(): boolean;
    hasPerspective(): boolean;
    invert(): (Mat3x3 | null);
    transpose(): Mat3x3;
    transposeSelf(): Mat3x3;
    map(x: f32, y: f32, z: f32): Vec3;
    mapVec3(v: Vec3): Vec3;
    mapVec2(v: Vec2): Vec2;
    mapAffineVec2(v: Vec2): Vec2;
    mapPoint(v: Vec2): Vec2;
    mapAffineRect(src: Rect): Rect;
}
export class Mat4x4 {
    private constructor();
    readonly underlyingArray: Float32Array;
    static Identity(): Mat4x4;
    static RowMajor(r: ArrayLike<f32>): Mat4x4;
    static ColMajor(c: ArrayLike<f32>): Mat4x4;
    static Rows(r0: Vec4, r1: Vec4, r2: Vec4, r3: Vec4): Mat4x4;
    static Cols(c0: Vec4, c1: Vec4, c2: Vec4, c3: Vec4): Mat4x4;
    static Translate(x: f32, y: f32, z: f32): Mat4x4;
    static Scale(x: f32, y: f32, z: f32): Mat4x4;
    static RectToRect(src: Rect, dst: Rect): Mat4x4;
    static Rotate(axis: Vec3, radians: f32): Mat4x4;
    static LookAt(eye: Vec3, center: Vec3, up: Vec3): Mat4x4;
    static Perspective(near: f32, far: f32, angle: f32): Mat4x4;
    static Concat(a: Mat4x4, b: Mat4x4): Mat4x4;
    equalTo(other: Mat4x4): boolean;
    clone(): Mat4x4;
    at(r: i32, c: i32): f32;
    setAt(r: i32, c: i32, value: f32): void;
    row(i: i32): Vec4;
    col(i: i32): Vec4;
    setRow(i: i32, v: Vec4): void;
    setCol(i: i32, v: Vec4): void;
    setIdentity(): Mat4x4;
    setTranslate(x: f32, y: f32, z: f32): Mat4x4;
    setScale(x: f32, y: f32, z: f32): Mat4x4;
    setRotateUnitSinCos(axis: Vec3, sinA: f32, cosA: f32): Mat4x4;
    setRotateUnit(axis: Vec3, radians: f32): Mat4x4;
    setRotate(axis: Vec3, radians: f32): Mat4x4;
    setConcat(a: Mat4x4, b: Mat4x4): Mat4x4;
    preConcat(m: Mat4x4): Mat4x4;
    postConcat(m: Mat4x4): Mat4x4;
    normalizePerspective(): void;
    invert(): (Mat4x4 | null);
    transpose(): Mat4x4;
    transposeSelf(): Mat4x4;
    toMat3x3(): Mat3x3;
    map(x: f32, y: f32, z: f32, w: f32): Vec4;
    mapVec3(v: Vec3): Vec3;
    mapVec4(v: Vec4): Vec4;
    isFinite(): boolean;
    determinant(): f32;
}
export class RuntimeEffectBuilder {
    constructor(effect: RuntimeEffect);
    setChild(name: string, v: SkSLChild): RuntimeEffectBuilder;
    setUniformFloat(name: string, v: f32): RuntimeEffectBuilder;
    setUniformFloatN(name: string, v: ArrayLike<f32>): RuntimeEffectBuilder;
    setUniformFloat2(name: string, x: f32, y: f32): RuntimeEffectBuilder;
    setUniformFloat2v(name: string, v: Vec2): RuntimeEffectBuilder;
    setUniformFloat2N(name: string, v: ArrayLike<f32>): RuntimeEffectBuilder;
    setUniformFloat2vN(name: string, v: ArrayLike<Vec2>): RuntimeEffectBuilder;
    setUniformFloat3(name: string, x: f32, y: f32, z: f32): RuntimeEffectBuilder;
    setUniformFloat3v(name: string, v: Vec3): RuntimeEffectBuilder;
    setUniformFloat3N(name: string, v: ArrayLike<f32>): RuntimeEffectBuilder;
    setUniformFloat3vN(name: string, v: ArrayLike<Vec3>): RuntimeEffectBuilder;
    setUniformFloat4(name: string, x: f32, y: f32, z: f32, w: f32): RuntimeEffectBuilder;
    setUniformFloat4v(name: string, v: Vec4): RuntimeEffectBuilder;
    setUniformFloat4N(name: string, v: ArrayLike<f32>): RuntimeEffectBuilder;
    setUniformFloat4vN(name: string, v: ArrayLike<Vec4>): RuntimeEffectBuilder;
    setUniformInt(name: string, v: i32): RuntimeEffectBuilder;
    setUniformIntN(name: string, v: ArrayLike<i32>): RuntimeEffectBuilder;
    setUniformInt2(name: string, x: i32, y: i32): RuntimeEffectBuilder;
    setUniformInt2N(name: string, v: ArrayLike<i32>): RuntimeEffectBuilder;
    setUniformInt3(name: string, x: i32, y: i32, z: i32): RuntimeEffectBuilder;
    setUniformInt3N(name: string, v: ArrayLike<i32>): RuntimeEffectBuilder;
    setUniformInt4(name: string, x: i32, y: i32, z: i32, w: i32): RuntimeEffectBuilder;
    setUniformInt4N(name: string, v: ArrayLike<i32>): RuntimeEffectBuilder;
    setUniformFloat2x2(name: string, r1c1: f32, r1c2: f32, r2c1: f32, r2c2: f32): RuntimeEffectBuilder;
    setUniformFloat3x3(name: string, mat: Mat3x3): RuntimeEffectBuilder;
    setUniformFloat4x4(name: string, mat: Mat4x4): RuntimeEffectBuilder;
    setUniformFloat2x2N(name: string, v: ArrayLike<f32>): RuntimeEffectBuilder;
    setUniformFloat3x3N(name: string, mat: Mat3x3[]): RuntimeEffectBuilder;
    setUniformFloat4x4N(name: string, mat: Mat4x4[]): RuntimeEffectBuilder;
    makeShader(localMatrix: (null | Mat3x3)): Shader;
    makeColorFilter(): ColorFilter;
    makeBlender(): Blender;
}
export class RSXformArray {
    constructor(count: i32);
    readonly count: i32;
    set(index: i32, scos: f32, ssin: f32, tx: f32, ty: f32): void;
    setFromRadians(index: f32, scale: f32, rad: f32, tx: f32, ty: f32, centerX: f32, centerY: f32): void;
    setIdentity(index: f32): void;
}
export class Vertices {
    private constructor();
    readonly uniqueID: u32;
    readonly approximateSize: u64;
    readonly bounds: Rect;
    static MakeCopy(mode: VertexMode, positions: Float32Array, texCoords: (Float32Array | null), colors: (Uint32Array | null), indices: (Uint16Array | null)): Vertices;
}
export class GpuDirectContext {
    private constructor();
    dispose(): void;
}
export class ColorFilter extends Flattenable {
    private constructor();
    static Compose(outer: ColorFilter, inner: ColorFilter): ColorFilter;
    static Blend(c: Color4f, cs: (ColorSpace | null), mode: BlendMode): ColorFilter;
    static Matrix(matrix: ColorMatrix): ColorFilter;
    static HSLAMatrix(matrix: ColorMatrix): ColorFilter;
    static LinearToSRGBGamma(): ColorFilter;
    static SRGBToLinearGamma(): ColorFilter;
    static Lerp(t: f32, dst: ColorFilter, src: ColorFilter): ColorFilter;
    static Table(table: Uint8Array): ColorFilter;
    static TableARGB(tableA: Uint8Array, tableR: Uint8Array, tableG: Uint8Array, tableB: Uint8Array): ColorFilter;
    static Lighting(mul: u32, add: u32): ColorFilter;
    static HighContrast(config: HighContrastConfig): ColorFilter;
    static Luma(): ColorFilter;
    static Overdraw(colors: u32[]): ColorFilter;
    static Deserialize(memory: Uint8Array, deserializers: (Deserializers | null)): (null | ColorFilter);
    asAColorMode(): (null | [u32, BlendMode]);
    asAColorMatrix(matrix: Float32Array): boolean;
    filterColor4f(src: Color4f, srcCS: ColorSpace, dstCS: ColorSpace): Color4f;
    makeComposed(inner: ColorFilter): ColorFilter;
    makeWithWorkingColorSpace(cs: ColorSpace): ColorFilter;
}
export class Picture extends Flattenable {
    private constructor();
    readonly cullRect: Rect;
    readonly uniqueID: u32;
    static Deserialize(memory: Uint8Array, deserializers: (Deserializers | null)): (null | Picture);
    static MakePlaceholder(cull: Rect): Picture;
    playback(canvas: Canvas): void;
    approximateOpCount(nested: boolean): i32;
    approximateBytesUsed(): u64;
    makeShader(tmx: TileMode, tmy: TileMode, mode: FilterMode, localMatrix: (null | Mat3x3), tileRect: (null | Rect)): Shader;
}
/**
 * Blender represents a custom blend function in the Skia pipeline. When a Blender is
 * present in a paint, the BlendMode is ignored. A blender combines a source color (the
 * result of our paint) and destination color (from the canvas) into a final color.
 */
export class Blender extends Flattenable {
    private constructor();
    /** Create a blender that implements the specified BlendMode. */
    static Mode(mode: BlendMode): Blender;
    /**
     * Create a blender that implements the following: k1 * src * dst + k2 * src + k3 * dst + k4.
     * 
     * @param k1 Coefficient.
     * @param k2 Coefficient.
     * @param k3 Coefficient.
     * @param k4 Coefficient.
     * @param enforcePremul If true, the RGB channels will be clamped to the calculated alpha.
     */
    static Arithmetic(k1: f32, k2: f32, k3: f32, k4: f32, enforcePremul: boolean): Blender;
    static Deserialize(memory: Uint8Array, deserializers: (Deserializers | null)): (null | Blender);
}
export class Path {
    constructor();
    fillType: PathFillType;
    readonly isConvex: boolean;
    readonly isEmpty: boolean;
    readonly isLastContourClosed: boolean;
    readonly isFinite: boolean;
    isVolatile: boolean;
    readonly roughBounds: Rect;
    readonly segmentMasks: u32;
    readonly generationID: u32;
    static Make(points: Float32Array, verbs: Uint8Array, weights: Float32Array, fillType: PathFillType, isVolatile: boolean): Path;
    static Rect(rect: Rect, dir: PathDirection, startIndex: u32): Path;
    static Oval(rect: Rect, dir: PathDirection, startIndex: u32): Path;
    static Circle(centerX: f32, centerY: f32, radius: f32, dir: PathDirection): Path;
    static RRect(rrect: RRect, dir: PathDirection, startIndex: u32): Path;
    static Polygon(points: Float32Array, isClosed: boolean, fillType: PathFillType, isVolatile: boolean): Path;
    static Line(x1: f32, y1: f32, x2: f32, y2: f32): Path;
    clone(): Path;
    equalTo(other: Path): boolean;
    isInterpolatable(compare: Path): boolean;
    interpolate(ending: Path, weight: f32): (Path | null);
    toggleInverseFillType(): void;
    asOval(): (Rect | null);
    asRRect(): (RRect | null);
    reset(): void;
    rewind(): void;
    countPoints(): i32;
    getPoint(index: i32): [f32, f32];
    getPoints(dst: Float32Array, maxPointNum: i32): i32;
    countVerbs(): i32;
    getVerbs(dst: Uint8Array, maxVerbNum: i32): i32;
    computeTightBounds(): Rect;
    conservativelyContainsRect(rect: Rect): boolean;
    asRect(): ([Rect, boolean, PathDirection] | null);
    addPathOffset(src: Path, dx: f32, dy: f32, mode: AddPathMode): void;
    addPath(src: Path, matrix: Mat3x3, mode: AddPathMode): void;
    reverseAddPath(src: Path): void;
    transform(matrix: Mat3x3, perspectiveClip: boolean): void;
    makeTransform(matrix: Mat3x3, perspectiveClip: boolean): Path;
    contains(x: f32, y: f32): boolean;
    fillWithPaint(paint: Paint, cull: (Rect | null), ctm: (Mat3x3 | null)): (Path | null);
    serialize(): Uint8Array;
    serializeToMemory(buffer: Uint8Array): u64;
    static Deserialize(buffer: Uint8Array): [(Path | null), u64];
    isValid(): boolean;
}
export class ImageFilter extends Flattenable {
    private constructor();
    readonly canComputeFastBounds: boolean;
    static Arithmetic(k1: f32, k2: f32, k3: f32, k4: f32, enforcePMColor: boolean, background: (ImageFilter | null), foreground: (ImageFilter | null), cropRect: (Rect | null)): ImageFilter;
    static Blend(mode: BlendMode, background: (ImageFilter | null), foreground: (ImageFilter | null), cropRect: (Rect | null)): ImageFilter;
    static Blender(blender: Blender, background: (ImageFilter | null), foreground: (ImageFilter | null), cropRect: (Rect | null)): ImageFilter;
    static Blur(sigmaX: f32, sigmaY: f32, tileMode: TileMode, input: (null | ImageFilter), cropRect: (null | Rect)): ImageFilter;
    static ColorFilter(cf: ColorFilter, input: (null | ImageFilter), cropRect: (null | Rect)): ImageFilter;
    static Compose(outer: ImageFilter, inner: ImageFilter): ImageFilter;
    static Crop(rect: Rect, tileMode: TileMode, input: (ImageFilter | null)): ImageFilter;
    static DisplacementMap(xChannelSelector: ColorChannel, yChannelSelector: ColorChannel, scale: f32, displacement: (ImageFilter | null), color: (ImageFilter | null), cropRect: (Rect | null)): ImageFilter;
    static DropShadow(dx: f32, dy: f32, sigmaX: f32, sigmaY: f32, color: u32, input: (ImageFilter | null), cropRect: (Rect | null)): ImageFilter;
    static DropShadowOnly(dx: f32, dy: f32, sigmaX: f32, sigmaY: f32, color: u32, input: (ImageFilter | null), cropRect: (Rect | null)): ImageFilter;
    static Empty(): ImageFilter;
    static Image(image: Image, srcRect: Rect, dstRect: Rect, sampling: SamplingOptions): ImageFilter;
    static Magnifier(lensBounds: Rect, zoomAmount: f32, inset: f32, sampling: SamplingOptions, input: (ImageFilter | null), cropRect: (Rect | null)): ImageFilter;
    static MatrixConvolution(kernelSize: [i32, i32], kernel: Float32Array, gain: f32, bias: f32, kernelOffset: [i32, i32], tileMode: TileMode, convolveAlpha: boolean, input: (ImageFilter | null), cropRect: (Rect | null)): ImageFilter;
    static MatrixTransform(matrix: Mat3x3, sampling: SamplingOptions, input: (ImageFilter | null)): ImageFilter;
    static Merge(filters: ImageFilter[], cropRect: (Rect | null)): ImageFilter;
    static Offset(dx: f32, dy: f32, input: (ImageFilter | null), cropRect: (Rect | null)): ImageFilter;
    static Picture(pic: Picture, targetRect: Rect): ImageFilter;
    static Shader(shader: Shader, dither: boolean, cropRect: (null | Rect)): ImageFilter;
    static Tile(src: Rect, dst: Rect, input: (null | ImageFilter)): ImageFilter;
    static Dilate(radiusX: f32, radiusY: f32, input: (null | ImageFilter), cropRect: (Rect | null)): ImageFilter;
    static Erode(radiusX: f32, radiusY: f32, input: (null | ImageFilter), cropRect: (Rect | null)): ImageFilter;
    static DistantLitDiffuse(direction: [f32, f32, f32], lightColor: u32, surfaceScale: f32, kd: f32, input: (null | ImageFilter), cropRect: (Rect | null)): ImageFilter;
    static PointLitDiffuse(location: [f32, f32, f32], lightColor: u32, surfaceScale: f32, kd: f32, input: (null | ImageFilter), cropRect: (Rect | null)): ImageFilter;
    static SpotLitDiffuse(location: [f32, f32, f32], target: [f32, f32, f32], falloffExponent: f32, cutoffAngle: f32, lightColor: u32, surfaceScale: f32, kd: f32, input: (null | ImageFilter), cropRect: (Rect | null)): ImageFilter;
    static DistantLitSpecular(direction: [f32, f32, f32], lightColor: u32, surfaceScale: f32, ks: f32, shininess: f32, input: (null | ImageFilter), cropRect: (Rect | null)): ImageFilter;
    static PointLitSpecular(location: [f32, f32, f32], lightColor: u32, surfaceScale: f32, ks: f32, shininess: f32, input: (null | ImageFilter), cropRect: (Rect | null)): ImageFilter;
    static SpotLitSpecular(location: [f32, f32, f32], target: [f32, f32, f32], falloffExponent: f32, cutoffAngle: f32, lightColor: u32, surfaceScale: f32, ks: f32, shininess: f32, input: (null | ImageFilter), cropRect: (Rect | null)): ImageFilter;
    static Deserialize(memory: Uint8Array, deserializers: (Deserializers | null)): (null | ImageFilter);
    filterBounds(src: Rect, ctm: Mat3x3, dir: MapDirection, inputRect: (Rect | null)): Rect;
    computeFastBounds(bounds: Rect): Rect;
    makeWithLocalMatrix(matrix: Mat3x3): (null | ImageFilter);
}
export class RuntimeEffect {
    private constructor();
    readonly source: string;
    readonly uniformsByteSize: u32;
    readonly uniforms: SkSLReflectUniform[];
    readonly children: SkSLReflectChild[];
    readonly allowShader: boolean;
    readonly allowColorFilter: boolean;
    readonly allowBlender: boolean;
    static CompileColorFilter(sksl: string): [RuntimeEffect, string];
    static CompileShader(sksl: string): [RuntimeEffect, string];
    static CompileBlender(sksl: string): [RuntimeEffect, string];
    makeShader(uniforms: (null | Uint8Array), children: SkSLChild[], localMatrix: (Mat3x3 | null)): Shader;
    makeColorFilter(uniforms: (null | Uint8Array), children: SkSLChild[]): ColorFilter;
    makeBlender(uniforms: (null | Uint8Array), children: SkSLChild[]): Blender;
    findUniform(name: string): (SkSLReflectUniform | null);
    findChild(name: string): (SkSLReflectChild | null);
}
export class ColorSpace {
    private constructor();
    static MakeSRGB(): ColorSpace;
    static MakeSRGBLinear(): ColorSpace;
    static Equals(cs1: ColorSpace, cs2: ColorSpace): boolean;
    dispose(): void;
    clone(): ColorSpace;
    gammaCloseToSRGB(): boolean;
    gammaIsLinear(): boolean;
    makeLinearGamma(): ColorSpace;
    makeSRGBGamma(): ColorSpace;
    makeColorSpin(): ColorSpace;
    isSRGB(): boolean;
}
export class ColorMatrix {
    constructor(m00: f32, m01: f32, m02: f32, m03: f32, m10: f32, m11: f32, m12: f32, m13: f32, m20: f32, m21: f32, m22: f32, m23: f32, m30: f32, m31: f32, m32: f32, m33: f32, m40: f32, m41: f32, m42: f32, m43: f32);
    static RGBtoYUV(cs: YUVColorSpace): ColorMatrix;
    static YUVtoRGB(cs: YUVColorSpace): ColorMatrix;
    setIdentity(): void;
    setScale(sr: f32, sg: f32, sb: f32, sa: f32): void;
    postTranslate(dr: f32, dg: f32, db: f32, da: f32): void;
    setConcat(a: ColorMatrix, b: ColorMatrix): void;
    preConcat(mat: ColorMatrix): void;
    postConcat(mat: ColorMatrix): void;
    setSaturation(sat: f32): void;
    setRowMajor(src: Float32Array): void;
    getRowMajor(dst: Float32Array): void;
}
export class PixmapUtils {
    private constructor();
    static ComputeIsOpaque(src: Pixmap): boolean;
    static Copy(src: Pixmap, dst: Pixmap, srcX: i32, srcY: i32): boolean;
    static EraseSubset(dst: Pixmap, color: u32, subset: Rect): boolean;
    static Erase(color: u32): boolean;
    static Scale(src: Pixmap, dst: Pixmap, sampling: SamplingOptions): boolean;
    static PickColor(src: Pixmap, x: i32, y: i32): u32;
}
export class Typeface {
    private constructor();
    readonly fontStyle: FontStyle;
    readonly isFixedPitch: boolean;
    readonly uniqueID: u32;
    readonly glyphsCount: i32;
    readonly tablesCount: i32;
    readonly tableTags: string[];
    readonly unitsPerEm: i32;
    readonly familyName: string;
    readonly postScriptName: (string | null);
    readonly bounds: Rect;
    static MakeEmpty(): Typeface;
    static Deserialize(data: Uint8Array, lastResortMgr: (FontMgr | null)): Typeface;
    getLocalizedFamilyNames(): LocalizedFamilyName[];
    equalTo(other: Typeface): boolean;
    getVariationDesignPosition(): Map<string, f32>;
    getVariationDesignParameters(): Map<string, VariationFontAxisInfo>;
    makeClone(arguments: FontArguments): Typeface;
    serialize(behavior: TypefaceSerializeBehavior): Uint8Array;
    unicharsToGlyphs(uni: Int32Array, outGlyphs: Uint16Array): void;
    textToGlyphs(text: string, outGlyphs: Uint16Array): void;
    unicharToGlyph(unichar: i32): u16;
    getTableSize(tag: string): u64;
    copyTableDataTo(tag: string, dst: Uint8Array, offset: u64, length: u64): u64;
    getKerningPairAdjustments(glyphs: Uint16Array, adjustments: Int32Array): boolean;
}
/**
 * Canvas provides an interface for drawing, and how the drawing is clipped and transformed.
 * Canvas contains a stack of Mat3x3/Mat4x4 and clip values.
 * 
 * `Canvas` and `Paint` together provide the state to draw into `Surface` or other targets.
 * Each Canvas draw call transforms the geometry of the object by the concatenation of all
 * matrix values in the stack. The transformed geometry is clipped by the intersection
 * of all of clip values in the stack. The Canvas draw calls use Paint to supply drawing
 * state such as color, Typeface, text size, stroke width, Shader and so on.
 * 
 * `Canvas` should be created by its parent, like `Surface`, `PictureRecorder`, etc.
 * Draw calls are sent to the parent that creates Canvas, and the parent also manages the
 * lifetime of `Canvas`. Once the parent is disposed, the Canvas will be disposed together
 * immediately.
 * 
 * `Canvas` is an event emitter of the following events:
 * @event release(): when the parent of Canvas is disposed.
 */
export class Canvas extends _event.EventEmitterBase {
    private constructor();
    /** Parent, an object that creates the Canvas and manages its lifetime. */
    readonly parent: object;
    /**
     * Bounds of clip in local coordinates, transformed by inverse of matrix.
     * If clip is empty, return `Rect.MakeEmpty()`, where all `Rect` sides equal zero.
     * 
     * `Rect` returned is outset by one to account for partial pixel coverage if clip
     * is anti-aliased.
     */
    readonly localClipBounds: Rect;
    /**
     * bounds of clip in base device coordinates, unaffected by matrix.
     * If clip is empty, return `Rect.MakeEmpty()`, where all `Rect` sides equal zero.
     * 
     * Unlike `Canvas.localClipBounds`, returned `Rect` is not outset.
     */
    readonly deviceClipBounds: Rect;
    /**
     * Saves matrix and clip.
     * Calling restore() discards changes to matrix and clip,
     * restoring the matrix and clip to their state when save() was called.
     * 
     * Matrix may be changed by `translate()`, `scale()`, `rotate()`, `skew()`, `concat()`,
     * `setMatrix()`, and `resetMatrix()`. Clip may be changed by `clipRect()`, `clipRRect()`,
     * `clipPath()`.
     * 
     * Saved Canvas state is put on a stack; multiple calls to `save()` should be balanced
     * by an equal number of calls to `restore()`.
     * 
     * Call `restoreToCount()` with the result to restore this and subsequent saves.
     * 
     * @return  depth of saved stack
     */
    save(): i32;
    /**
     * Saves matrix and clip, and allocates a Surface for subsequent drawing.
     * Calling `restore()` discards changes to matrix and clip, and draws the Surface.
     * 
     * Rect bounds suggests but does not define the Surface size. To clip drawing to
     * a specific rectangle, use `clipRect()`.
     * 
     * Optional paint applies alpha, ColorFilter, ImageFilter, and
     * BlendMode when `restore()` is called.
     * 
     * Call `restoreToCount()` with returned value to restore this and subsequent saves.
     * 
     * @param bounds  hint to limit the size of the layer; may be `null`
     * @param paint   graphics state for layer; may be `null`
     * @return        depth of saved stack
     */
    saveLayer(bounds: (Rect | null), paint: (null | Paint)): i32;
    /** Variant of `saveLayer`, accepts `alpha` instead of `Paint`. */
    saveLayerAlphaf(bounds: (Rect | null), alpha: f32): i32;
    /**
     * Variant of `saveLayer`, accepts `SaveLayerRec` that contains the state used to create
     * the layer.
     */
    saveLayerRec(rec: SaveLayerRec): i32;
    /**
     * Removes changes to matrix and clip since Canvas state was last saved.
     * The state is removed from the stack.
     * 
     * Does nothing if the stack is empty.
     */
    restore(): void;
    /**
     * Returns the number of saved states, each containing: matrix and clip.
     * Equals the number of save() calls less the number of `restore()` calls plus one.
     * The save count of a new canvas is one.
     * 
     * @return  depth of save state stack
     */
    getSaveCount(): i32;
    /**
     * Restores state to matrix and clip values when `save()`, `saveLayer()`,
     * `saveLayerAlphaf()`, or `saveLayerRec()` returned `saveCount`.
     * 
     * Does nothing if saveCount is greater than state stack count.
     * Restores state to initial values if saveCount is less than or equal to one.
     * 
     * @param saveCount  depth of state stack to restore
     */
    restoreToCount(saveCount: i32): void;
    /**
     * Translates matrix by dx along the x-axis and dy along the y-axis.
     * 
     * Mathematically, replaces local CTM with a translation matrix
     * premultiplied with the current local CTM.
     * 
     * This has the effect of moving the drawing by (dx, dy) before transforming
     * the result with the previous local CTM.
     * 
     * @param dx  distance to translate on x-axis
     * @param dy  distance to translate on y-axis
     */
    translate(dx: f32, dy: f32): void;
    /**
     * Scales matrix by sx on the x-axis and sy on the y-axis.
     * 
     * Mathematically, replaces local CTM with a scale matrix
     * premultiplied with the current local CTM.
     * 
     * This has the effect of scaling the drawing by (sx, sy) before transforming
     * the result with the original local CTM.
     * 
     * @param sx  amount to scale on x-axis
     * @param sy  amount to scale on y-axis
     */
    scale(sx: f32, sy: f32): void;
    /**
     * Rotates matrix by degrees. Positive degrees rotates clockwise.
     * 
     * Mathematically, replaces local CTM with a rotation matrix
     * premultiplied with the current local CTM.
     * 
     * This has the effect of rotating the drawing by degrees before transforming
     * the result with the original local CTM.
     * 
     * @param degrees  amount to rotate, in degrees
     */
    rotate(degrees: f32): void;
    /** Variant of `rotate()`, accepts a point (px, py) to rotate about. */
    rotatePivot(degrees: f32, px: f32, py: f32): void;
    /**
     * Skews matrix by sx on the x-axis and sy on the y-axis. A positive value of sx
     * skews the drawing right as y-axis values increase; a positive value of sy skews
     * the drawing down as x-axis values increase.
     * 
     * Mathematically, replaces local CTM with a skew matrix premultiplied with the
     * current local CTM.
     * 
     * This has the effect of skewing the drawing by (sx, sy) before transforming
     * the result with the original local CTM.
     * 
     * @param sx  amount to skew on x-axis
     * @param sy  amount to skew on y-axis
     */
    skew(sx: f32, sy: f32): void;
    /**
     * Replaces matrix with matrix premultiplied with the existing one.
     * The internal matrix storage is always `Mat4x4`, `concat33()` will expand
     * the 3x3 matrix to 4x4 before multiplying.
     * 
     * @param mat  matrix to premultiply with existing SkMatrix
     */
    concat33(mat: Mat3x3): void;
    /** A variant of `concat33()`, accepts a `Mat4x4` matrix. */
    concat44(mat: Mat4x4): void;
    setMatrix(mat: Mat4x4): void;
    /**
     * Sets matrix to the identity matrix.
     * Any prior matrix state is overwritten.
     */
    resetMatrix(): void;
    /**
     * Replaces clip with the intersection or difference of clip and rect,
     * with an aliased or anti-aliased clip edge. rect is transformed by SkMatrix
     * before it is combined with clip.
     * 
     * @param rect         `Rect` to combine with clip
     * @param op           `ClipOp` to apply to clip
     * @param doAntiAlias  true if clip is to be anti-aliased
     */
    clipRect(rect: Rect, op: ClipOp, doAntiAlias: boolean): void;
    /**
     * Replaces clip with the intersection or difference of clip and rrect,
     * with an aliased or anti-aliased clip edge.
     * rrect is transformed by matrix before it is combined with clip.
     * 
     * @param rrect        `RRect` to combine with clip
     * @param op           `ClipOp` to apply to clip
     * @param doAntiAlias  true if clip is to be anti-aliased
     */
    clipRRect(rrect: RRect, op: ClipOp, doAntiAlias: boolean): void;
    /**
     * Replaces clip with the intersection or difference of clip and path,
     * with an aliased or anti-aliased clip edge. `Path.fillType` determines if path
     * describes the area inside or outside its contours; and if path contour overlaps
     * itself or another path contour, whether the overlaps form part of the area.
     * path is transformed by matrix before it is combined with clip.
     * 
     * @param path         `Path` to combine with clip
     * @param op           `ClipOp` to apply to clip
     * @param doAntiAlias  true if clip is to be anti-aliased
     */
    clipPath(path: Path, op: ClipOp, doAntiAlias: boolean): void;
    /**
     * Replaces clip with the intersection or difference of clip and shader.
     * The result of shader will be converted into grayscale first,
     * and multiplies the pixel value with the grayscale to generate the final pixel.
     * In some situations, this may also be called "Shader Mask".
     * 
     * @param shader       `Shader` to combine with clip.
     * @param op           `ClipOp` to apply to clip.
     */
    clipShader(shader: Shader, op: ClipOp): void;
    /**
     * Returns true if `rect`, transformed by matrix, can be quickly determined to be
     * outside of clip. May return false even though `rect` is outside of clip.
     * 
     * Use to check if an area to be drawn is clipped out, to skip subsequent draw calls.
     * 
     * @param rect  `Rect` area to test
     * @return      true if rect, transformed by matrix, does not intersect clip
     */
    quickRejectRect(rect: Rect): boolean;
    /** A variant of `quickRejectRect()`, accepts a `Path` instead of `Rect`. */
    quickRejectPath(path: Path): boolean;
    /**
     * Fills clip with color color.
     * `mode` determines how ARGB is combined with destination.
     * 
     * @param color  unpremultiplied ARGB
     * @param mode   `BlendMode` used to combine source color and destination
     */
    drawColor(color: u32, mode: BlendMode): void;
    /** A variant of `drawColor()`, accepts a Color4F quadruple. */
    drawColor4f(color: Color4f, mode: BlendMode): void;
    /**
     * A variant of `drawColor4f()`, with `mode == BlendMode.Src`.
     * Useful for erasing the whole clip with a particular color.
     */
    clear(color: Color4f): void;
    /**
     * Makes Canvas contents undefined. Subsequent calls that read Canvas pixels,
     * such as drawing with `BlendMode`, return undefined results. `discard()` does
     * not change clip or matrix.
     * 
     * `discard()` may do nothing, depending on the implementation that created Canvas.
     * 
     * `discard()` allows optimized performance on subsequent draws by removing
     * cached data associated with the underlying implementation.
     * It is not necessary to call `discard()` once done with Canvas;
     * any cached data is deleted when owning parent of Canvas is deleted.
     */
    discard(): void;
    drawPaint(paint: Paint): void;
    drawPoints(mode: PointMode, pts: Float32Array, paint: Paint): void;
    drawPoint(x: f32, y: f32, paint: Paint): void;
    drawLine(x0: f32, y0: f32, x1: f32, y1: f32, paint: Paint): void;
    drawRect(rect: Rect, paint: Paint): void;
    drawOval(oval: Rect, paint: Paint): void;
    drawRRect(rrect: RRect, paint: Paint): void;
    drawDRRect(outer: RRect, inner: RRect, paint: Paint): void;
    drawCircle(cx: f32, cy: f32, radius: f32, paint: Paint): void;
    drawArc(oval: Rect, startAngleDeg: f32, sweepAngleDeg: f32, useCenter: boolean, paint: Paint): void;
    drawRoundRect(rect: Rect, rx: f32, ry: f32, paint: Paint): void;
    drawPath(path: Path, paint: Paint): void;
    drawImage(image: Image, left: f32, top: f32, sampling: SamplingOptions, paint: (Paint | null)): void;
    drawImageRect(image: Image, dst: Rect, sampling: SamplingOptions, paint: (Paint | null)): void;
    drawImageRectToRect(image: Image, src: Rect, dst: Rect, sampling: SamplingOptions, paint: (Paint | null), strictConstraint: boolean): void;
    drawPicture(picture: Picture, matrix: (Mat3x3 | null), paint: (Paint | null)): void;
    drawString(text: string, x: f32, y: f32, font: Font, paint: Paint): void;
    drawGlyphs(glyphs: Uint16Array, xforms: RSXformArray, originX: f32, originY: f32, font: Font, paint: Paint): void;
    drawVertices(vertices: Vertices, mode: BlendMode, paint: Paint): void;
}
export function ColorTypeBytesPerPixel(ct: ColorType): i32;
export function ColorTypeIsAlwaysOpaque(ct: ColorType): boolean;
