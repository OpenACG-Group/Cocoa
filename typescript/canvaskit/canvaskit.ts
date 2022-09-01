/**
 * This module generates GSKIR (GSK Intermediate Representation) bytecode
 * and submit it to native rendering engine, who executes the submitted bytecode and
 * generates drawing instructions. Those instructions will be performed by CPU or GPU
 * asynchronously depending on whether the hardware-acceleration is available.
 */

import * as std from 'core';
import * as Glamor from 'glamor';
import { IProtoBufferWriter } from "./canvaskit_iface";
import { Opcode, Constants, ProtoCodeEmitter, MemOp } from "./generated/code_emitter";
import * as M from './comath';

const PROTO_BUFFER_UNIT_BUFFER_SIZE = 4096;
const PROTO_OPCODE_BYTE_SIZE = 2;

const MEMOP_NULL = 0;

export namespace Features {
    export const CODEGEN_HEAP_PROFILING = 'codegen-heap-profiling';
    export const CODEGEN_REDUNDANT_ALLOCATION_ANALYSIS = 'codegen-redundant-allocation-analysis';
}

let _features = new Map<string, boolean>([
    [ Features.CODEGEN_HEAP_PROFILING, false ],
    [ Features.CODEGEN_REDUNDANT_ALLOCATION_ANALYSIS, false ]
]);

/**
 * Enable or disable features of CanvasKit.
 * @param name     Name of feature. See also namespace `Features`.
 * @param value    `true` to enable the feature, while `false` to disable it.
 */
export function setCanvasFeature(name: string, value: boolean) {
    if (!_features.has(name))
        throw Error('Invalid canvas feature \'' + name + '\'');
    _features.set(name, value);
}

/**
 * Construct U32 ARGB color from unnormalized ARGB components.
 * @param a     Alpha component in [0, 255)
 * @param r     Red component in [0, 255)
 * @param g     Green component in [0, 255)
 * @param b     Blue component in [0, 255)
 * @returns     Constructed U32 ARGB value
 */
export function U32ColorFromUnorm(a: M.Int, r: M.Int, g: M.Int, b: M.Int): M.Int {
    a = (a | 0) & 0xff;
    r = (r | 0) & 0xff;
    g = (g | 0) & 0xff;
    b = (b | 0) & 0xff;
    return (a << 24) | (r << 16) | (g << 8) | b;
}

/**
 * Construct U32 ARGB color from normalized ARGB components.
 * @param a     Alpha component in [0, 1]
 * @param r     Red component in [0, 1]
 * @param g     Green component in [0, 1]
 * @param b     Blue component in [0, 1]
 * @returns     Constructed U32 ARGB value
 */
export function U32ColorFromNorm(a: M.Scalar, r: M.Scalar, g: M.Scalar, b: M.Scalar): M.Int {
    let u8a = M.linearInterpolate(0, 0xff, a);
    let u8r = M.linearInterpolate(0, 0xff, r);
    let u8g = M.linearInterpolate(0, 0xff, g);
    let u8b = M.linearInterpolate(0, 0xff, b);
    return U32ColorFromUnorm(u8a, u8r, u8g, u8b);
}

/**
 * A convenient function of `U32ColorFromNorm` with `v = (A, R, G, B)`.
 */
export function U32ColorFromVector4Norm(v: M.Vector4): M.Int {
    return U32ColorFromNorm(v.x, v.y, v.z, v.w);
}

/**
 * A convenient function of `U32ColorFromNorm` with `v = (R, G, B)`.
 */
export function U32ColorFromVector3Norm(v: M.Vector3, alpha: M.Scalar = 1.0): M.Int {
    return U32ColorFromNorm(alpha, v.x, v.y, v.z);
}

export class HeapNullReferenceError extends Error {
    public readonly typename: string;
    constructor(typename: string) {
        super(`Reference to heap object '${typename}' should not be NULL`);
        this.typename = typename;
    }
}

export class AssemblerError extends Error { constructor(what: string) { super(what); } }
export class HeapObjectOwnershipError extends Error { constructor(what: string) { super(what); } }

function BoolNumber(v: boolean): number {
    return v ? 1 : 0;
}

interface IndexedBuffer {
    index: number;
    buffer: std.Buffer;
}

class ProtoBufferWriter implements IProtoBufferWriter {
    private context: MemoryResourceGroup;
    private buffersVector: IndexedBuffer[];
    private currentBuffer: std.Buffer;
    private currentBufferPos: number;
    private dataView: DataView;

    constructor(context: MemoryResourceGroup) {
        this.context = context;
        let bufIndex = context.acquireBuffer();

        this.buffersVector = [ {index: bufIndex, buffer: context.getBufferFromIndex(bufIndex)} ];
        this.currentBuffer = this.buffersVector[0].buffer;
        this.currentBufferPos = 0;
        this.dataView = this.currentBuffer.toDataView();
    }

    public dispose(): void {
        this.currentBuffer = null;
        this.currentBufferPos = 0;
        for (let buffer of this.buffersVector) {
            this.context.releaseBuffer(buffer.index);
        }
        this.buffersVector = [];
        this.dataView = null;
    }

    public getBuffers(): std.Buffer[] {
        let result = [];
        for (let obj of this.buffersVector) {
            result.push(obj.buffer);
        }
        return result;
    }

    private switchToNextBuffer(): void {
        let bufIndex = this.context.acquireBuffer();
        this.buffersVector.push({
            index: bufIndex,
            buffer: this.context.getBufferFromIndex(bufIndex)
        });
        this.currentBuffer = this.context.getBufferFromIndex(bufIndex);
        this.currentBufferPos = 0;
        this.dataView = this.currentBuffer.toDataView();
    }

    public performPossibleBufferSwitching(requireSize: number): void {
        let size = requireSize + PROTO_OPCODE_BYTE_SIZE;
        let remaining = this.currentBuffer.length - this.currentBufferPos;
        if (size >= remaining) {
            this.writeUint16Unsafe(Opcode.kSwitchNextBuffer | 0x0);
            this.switchToNextBuffer();
        }
    }

    public writeInt8Unsafe(x: number): void {
        this.dataView.setInt8(this.currentBufferPos, x | 0);
        this.currentBufferPos += 1;
    }

    public writeUint8Unsafe(x: number): void {
        this.dataView.setUint8(this.currentBufferPos, x | 0);
        this.currentBufferPos += 1;
    }

    public writeInt16Unsafe(x: number): void {
        this.dataView.setInt16(this.currentBufferPos, x | 0, true);
        this.currentBufferPos += 2;
    }

    public writeUint16Unsafe(x: number): void {
        this.dataView.setUint16(this.currentBufferPos, x | 0, true);
        this.currentBufferPos += 2;
    }

    public writeInt32Unsafe(x: number): void {
        this.dataView.setInt32(this.currentBufferPos, x | 0, true);
        this.currentBufferPos += 4;
    }

    public writeUint32Unsafe(x: number): void {
        this.dataView.setUint32(this.currentBufferPos, x | 0, true);
        this.currentBufferPos += 4;
    }

    public writeFloat32Unsafe(x: number): void {
        this.dataView.setFloat32(this.currentBufferPos, x, true);
        this.currentBufferPos += 4;
    }

    public writeFloat64Unsafe(x: number): void {
        this.dataView.setFloat64(this.currentBufferPos, x, true);
        this.currentBufferPos += 8;
    }

    public writeInt64Unsafe(x: bigint): void {
        this.dataView.setBigInt64(this.currentBufferPos, x, true);
        this.currentBufferPos += 8;
    }

    public writeUint64Unsafe(x: bigint): void {
        this.dataView.setBigUint64(this.currentBufferPos, x, true);
        this.currentBufferPos += 8;
    }
}

export class MemoryResourceGroup {
    private pool: Array<std.Buffer>;
    private statusBitmap: Uint8Array;
    private singleBufferSize: number;

    constructor() {
        this.pool = new Array<std.Buffer>();
        this.statusBitmap = new Uint8Array(1024);
        this.singleBufferSize = PROTO_BUFFER_UNIT_BUFFER_SIZE;
        // TODO: adjust `singleBufferSize` based on history allocations at runtime
        //       to reduce memory pressure.
    }

    public acquireBuffer(): number {
        let group = -1;
        for (let i = 0; i < this.statusBitmap.length; i++) {
            if (this.statusBitmap[i] != 0xff) {
                group = i;
                break;
            }
        }
        if (group < 0) {
            throw new Error('Current memory resource group is full');
        }

        let bits = this.statusBitmap[group];
        let idx = group << 3;                   // idx = group * 8;
        while (bits & 1) {
            bits >>>= 1;                        // bits *= 2
            idx++;
        }
        this.statusBitmap[group] |= 1 << (idx & 7);

        // std.print(`# Allocate index ${idx}, group ${group}\n`);

        if (idx > this.pool.length) {
            throw new Error('Corrupted memory resource group');
        } else if (idx == this.pool.length) {
            this.pool.push(new std.Buffer(this.singleBufferSize));
        }
        return idx;
    }

    public releaseBuffer(idx: number): void {
        if (!Number.isInteger(idx) || idx >= this.pool.length) {
            throw new TypeError('Invalid index of buffer in pool');
        }
        this.statusBitmap[idx >> 3] &= ~(1 << (idx & 7));
    }

    public getBufferFromIndex(idx: number): std.Buffer {
        if (!Number.isInteger(idx) || idx >= this.pool.length) {
            throw new TypeError('Invalid index of buffer in pool');
        }
        return this.pool[idx];
    }
}

enum MemOpTypes {
    kString     = 'string',
    kU32Array   = 'u32array',
    kF32Array   = 'f32array',
    kMatrix3x3  = 'mat3x3',
    kMatrix4x4  = 'mat4x4',
    kVector2    = 'vec2',
    kVector3    = 'vec3',
    kVector4    = 'vec4',
    kRect       = 'rect',
    kRRect      = 'rrect',
    kRegion     = 'region',
    kPaint      = 'paint',
    kPath       = 'path',
    kSamplingOptions = 'samplingoptions',
    kShader      = 'shader',
    kBlender     = 'blender',
    kColorFilter = 'colorfilter',
    kImageFilter = 'imagefilter',
    kMaskFilter  = 'maskfilter',
    kPathEffect  = 'patheffect',
    kImage       = 'image',
    kBitmap      = 'bitmap',
    kPicture     = 'picture'
}
interface IRTypedMemOp {
    type: MemOpTypes;
    memOp: MemOp;
}

class IRAssemblerBase {
    protected emitter: ProtoCodeEmitter;
    protected memOpRefCounts: Map<MemOp, number>;
    protected memOpIdCounter: MemOp;

    constructor(emitter: ProtoCodeEmitter) {
        this.emitter = emitter;
        this.memOpIdCounter = 0;
        this.memOpRefCounts = new Map<MemOp, number>();
    }

    public getEmitter(): ProtoCodeEmitter {
        return this.emitter;
    }

    public refMemOp(op: IRTypedMemOp): void {
        if (op.memOp == MEMOP_NULL) {
            throw new HeapNullReferenceError(op.type);
        }
        let v = this.memOpRefCounts.has(op.memOp) ? this.memOpRefCounts.get(op.memOp) : 0;
        this.memOpRefCounts.set(op.memOp, v + 1);
    }

    public unrefMemOp(op: IRTypedMemOp): void {
        if (op.memOp == MEMOP_NULL) {
            throw new HeapNullReferenceError(op.type);
        }
        if (!this.memOpRefCounts.has(op.memOp)) {
            throw new AssemblerError('Unref a memory operand which is not managed by current assembler');
        }

        let v = this.memOpRefCounts.get(op.memOp) - 1;
        if (v <= 0) {
            this.emitter.emitHeapFree(op.memOp);
            this.memOpRefCounts.delete(op.memOp);
        } else {
            this.memOpRefCounts.set(op.memOp, v);
        }
    }

    public allocateMemOpWithRef(factoryFunc: (op: MemOp, emitter: ProtoCodeEmitter) => MemOpTypes): MemOp {
        let op = ++this.memOpIdCounter;
        this.refMemOp({ type: factoryFunc(op, this.emitter), memOp: op });
        return op;
    }

    public allocateLocalMemOpWithoutRef(factoryFunc: (op: MemOp, emitter: ProtoCodeEmitter) => MemOpTypes): MemOp {
        let op = ++this.memOpIdCounter;
        factoryFunc(op, this.emitter);
        return op;
    }

    public allocateLocalRect(rect: M.Rect): MemOp {
        return this.allocateLocalMemOpWithoutRef((op, e) => {
            this.emitter.emitHeapCreateRect(op, rect.x(), rect.y(), rect.width(), rect.height());
            return MemOpTypes.kRect;
        });
    }

    public allocateLocalVector2(v: M.Vector2): MemOp {
        return this.allocateLocalMemOpWithoutRef((op, e) => {
            this.emitter.emitHeapCreateVector2(op, v.x, v.y);
            return MemOpTypes.kVector2;
        });
    }

    public allocateLocalVector3(v: M.Vector3): MemOp {
        return this.allocateLocalMemOpWithoutRef((op, e) => {
            this.emitter.emitHeapCreateVector3(op, v.x, v.y, v.z);
            return MemOpTypes.kVector3;
        });
    }

    public allocateLocalVector4(v: M.Vector4): MemOp {
        return this.allocateLocalMemOpWithoutRef((op, e) => {
            this.emitter.emitHeapCreateVector4(op, v.x, v.y, v.z, v.w);
            return MemOpTypes.kVector4;
        });
    }

    public checkAssemblerOwnership(heapObject: HeapObjectWithAssemblerBase): void {
        if (heapObject.getAssembler() != this) {
            throw new HeapObjectOwnershipError('Heap object is not owned by current assembler');
        }
    }
}

export interface ReferenceCountedHeapObject {
    ref(): void;
    unref(): void;
}

/* IR Heap */
type HeapObjectFactoryFunc = (op: MemOp, assembler: IRAssemblerBase) => void;
class HeapObjectWithAssemblerBase implements ReferenceCountedHeapObject {
    protected heapMemOp: IRTypedMemOp;
    protected assembler: IRAssemblerBase;

    constructor(assembler: IRAssemblerBase, type: MemOpTypes, factory: HeapObjectFactoryFunc) {
        this.assembler = assembler;
        let resultOp = this.assembler.allocateMemOpWithRef((op) => {
            factory(op, this.assembler);
            return type;
        });
        this.heapMemOp = { type: type, memOp: resultOp };
    }

    public getEmitter(): ProtoCodeEmitter {
        return this.assembler.getEmitter();
    }

    public getMemOperand(): MemOp {
        return this.heapMemOp.memOp;
    }

    public getAssembler(): IRAssemblerBase {
        return this.assembler;
    }

    public ref(): void {
        this.assembler.refMemOp(this.heapMemOp);
    }

    public unref(): void {
        this.assembler.unrefMemOp(this.heapMemOp);
    }
}

/**
 * Usage of the stateless immutable heap object.
 * 
 * Supposing `T` is the class implementation of a stateless immutable heap object:
 * @example
 * ```typescript
 * class T extends StatelessImmutableHeapObjectBase {
 *     <some code...>
 * }
 * ```
 * 
 * We use `T` like this:
 * @example
 * ```typescript
 * let v = new T(..);        // Now `v` is unassociated, and you don't need an assembler to construct it
 * v.ref();                  // Error! `v` has not been associated with any assembler
 * 
 * paint.setT(v);            // Now `v` is associated with `canvas`
 * v.unref();                // OK
 * 
 * paint2.setT(v);           // Error! `v` cannot be associated with multiple assemblers
 * 
 * v.resetAssociation();
 * paint2.setT(v);           // `v` can be associated with another assembler after `resetAssociation`
 * ```
 * 
 * Compared with other normal heap objects like `Paint`:
 * @example
 * ```typescript
 * let paint = new Paint(canvas); // You need an assembler to construct it
 * ```
 */
class StatelessImmutableHeapObjectBase implements ReferenceCountedHeapObject {
    private type: MemOpTypes;
    private factory: HeapObjectFactoryFunc;
    private associatedAssembler: IRAssemblerBase;
    private heapMemOp: IRTypedMemOp;

    constructor(type: MemOpTypes, factory: HeapObjectFactoryFunc) {
        this.type = type;
        this.factory = factory;
        this.associatedAssembler = null;
        this.heapMemOp = null;
    }

    public associateAssembler(assembler: IRAssemblerBase): void {
        if (assembler == this.associatedAssembler) {
            return;
        }
        if (this.associatedAssembler) {
            throw new HeapObjectOwnershipError('Associate an associated immutable heap object with another assembler');
        }

        this.associatedAssembler = assembler;
        let resultOp = this.associatedAssembler.allocateMemOpWithRef((op) => {
            this.factory(op, this.associatedAssembler);
            return this.type;
        });
        this.heapMemOp = { type: this.type, memOp: resultOp };
    }

    public ref(): void {
        if (!this.heapMemOp) {
            throw new HeapNullReferenceError('ref an unassociated immutable heap object');
        }
        this.associatedAssembler.refMemOp(this.heapMemOp);
    }

    public unref(): void {
        if (!this.heapMemOp) {
            throw new HeapNullReferenceError('unref an unassociated immutable heap object');
        }
        this.associatedAssembler.unrefMemOp(this.heapMemOp);
    }

    public isAssociated(): boolean {
        return (this.associatedAssembler != null);
    }

    public resetAssociation(): void {
        this.associatedAssembler = null;
        this.heapMemOp = null;
    }

    public getMemOperand(): MemOp {
        if (!this.heapMemOp) {
            throw new HeapNullReferenceError('Unassociated immutable heap object');
        }
        return this.heapMemOp.memOp;
    }
}

/* Paint parameters */
export enum PaintCap {
    kButt   = Constants.PAINTCAP_BUTT,
    kRound  = Constants.PAINTCAP_ROUND,
    kSquare = Constants.PAINTCAP_SQUARE
}

export enum PaintJoin {
    kMiter   = Constants.PAINTJOIN_MITER,
    kRound   = Constants.PAINTJOIN_ROUND,
    kBevel   = Constants.PAINTJOIN_BEVEL
}

export enum BlendMode {
    kClear      = Constants.BLENDMODE_CLEAR,
    kSrc        = Constants.BLENDMODE_SRC,
    kDst        = Constants.BLENDMODE_DST,
    kDstOver    = Constants.BLENDMODE_DST_OVER,
    kSrcOver    = Constants.BLENDMODE_SRC_OVER,
    kSrcIn      = Constants.BLENDMODE_SRC_IN,
    kDstIn      = Constants.BLENDMODE_DST_IN,
    kSrcOut     = Constants.BLENDMODE_SRC_OUT,
    kDstOut     = Constants.BLENDMODE_DST_OUT,
    kSrcATop    = Constants.BLENDMODE_SRC_ATOP,
    kDstATop    = Constants.BLENDMODE_DST_ATOP,
    kXor        = Constants.BLENDMODE_XOR,
    kPlus       = Constants.BLENDMODE_PLUS,
    kModulate   = Constants.BLENDMODE_MODULATE,
    kScreen     = Constants.BLENDMODE_SCREEN,
    kOverlay    = Constants.BLENDMODE_OVERLAY,
    kLighten    = Constants.BLENDMODE_LIGHTEN,
    kColorDodge = Constants.BLENDMODE_COLOR_DODGE,
    kColorBurn  = Constants.BLENDMODE_COLOR_BURN,
    kHardLight  = Constants.BLENDMODE_HARD_LIGHT,
    kSoftLight  = Constants.BLENDMODE_SOFT_LIGHT,
    kDifference = Constants.BLENDMODE_DIFFERENCE,
    kExclusion  = Constants.BLENDMODE_EXCLUSION,
    kMultiply   = Constants.BLENDMODE_MULTIPLY,
    kHue        = Constants.BLENDMODE_HUE,
    kSaturation = Constants.BLENDMODE_SATURATION,
    kColor      = Constants.BLENDMODE_COLOR,
    kLuminosity = Constants.BLENDMODE_LUMINOSITY
}

export enum TileMode {
    kClamp   = Constants.TILEMODE_CLAMP,
    kRepeat  = Constants.TILEMODE_REPEAT,
    kMirror  = Constants.TILEMODE_MIRROR,
    kDecal   = Constants.TILEMODE_DECAL
}

export enum RegionOp {
    kDifference         = Constants.REGIONOP_DIFFERENCE,
    kIntersect          = Constants.REGIONOP_INTERSECT,
    kUnion              = Constants.REGIONOP_UNION,
    kXor                = Constants.REGIONOP_XOR,
    kReverseDifference  = Constants.REGIONOP_REVERSE_DIFFERENCE,
    kReplace            = Constants.REGIONOP_REPLACE
}

export enum ColorChannel {
    kA = Constants.COLORCHANNEL_A,
    kR = Constants.COLORCHANNEL_G,
    kG = Constants.COLORCHANNEL_B,
    kB = Constants.COLORCHANNEL_B
}

export enum FilterMode {
    kNearest  = Constants.FILTERMODE_NEAREST,
    kLinear   = Constants.FILTERMODE_LINEAR
}

export enum MipmapMode {
    kNone     = Constants.MIPMAPMODE_NONE,
    kNearest  = Constants.MIPMAPMODE_NEAREST,
    kLinear   = Constants.MIPMAPMODE_LINEAR
}

export enum PathFillType {
    kWinding         = Constants.PATHFILLTYPE_WINDING,
    kEvenOdd         = Constants.PATHFILLTYPE_EVEN_ODD,
    kInverseWinding  = Constants.PATHFILLTYPE_INVERSE_WINDING,
    kInverseEvenOdd  = Constants.PATHFILLTYPE_INVERSE_EVEN_ODD
}

export enum ClipOp {
    kDifference     = Constants.CLIPOP_DIFFERENCE,
    kIntersect      = Constants.CLIPOP_INTERSECT
}

export enum CanvasStatus {
    kRecording,
    kFinished,
    kDisposed
}

/**
 * Canvas provides an interface for drawing, and how the drawing is
 * clipped and transformed. Canvas contains a stack of matrix and clip values.
 * 
 * Canvas generates GSKIR bytecode for most draw calls, which is usually stored
 * in memory and can be submitted to native rendering layer. The native
 * rendering layer can "execute" GSKIR bytecode to generate a GskPicture object.
 * That object can be serialized, rasterized, or drawn on other Canvas.
 * 
 * All the Canvases created from the same `MemoryResourceGroup` share a common
 * memory pool. Binary representation of GSKIR will be written into that memory
 * pool.
 */
export class Canvas extends IRAssemblerBase {
    private writer: ProtoBufferWriter;
    private status: CanvasStatus;
    private heapBinder: Glamor.MoeHeapObjectBinder;
    private breakpointIdCounter = 0;
    private breakpointsMap: Map<number, Function>;

    constructor(ctx: MemoryResourceGroup, width: number, height: number) {
        let writer = new ProtoBufferWriter(ctx);
        super(new ProtoCodeEmitter(writer));

        this.writer = writer;
        this.status = CanvasStatus.kRecording;
        this.heapBinder = new Glamor.MoeHeapObjectBinder();
        this.breakpointsMap = new Map<number, Function>();

        this.getEmitter().emitDrawBounds(width, height);
    }

    private checkStatus(expects: CanvasStatus): void {
        if (this.status != expects) {
            throw new Error(`Canvas is in a wrong state`);
        }
    }

    public save(): void {
        this.emitter.emitSave();
    }

    public restore(): void {
        this.emitter.emitRestore();
    }

    public translate(dx: M.Scalar, dy: M.Scalar): void {
        this.emitter.emitTranslate(dx, dy);
    }

    public scale(dx: M.Scalar, dy: M.Scalar): void {
        this.emitter.emitScale(dx, dy);
    }

    public rotate(degrees: M.Scalar, px?: M.Scalar, py?: M.Scalar): void {
        if (px != undefined && py != undefined) {
            this.emitter.emitRotate2(degrees, px, py);
        } else {
            this.emitter.emitRotate(degrees);
        }
    }

    public skew(sx: M.Scalar, sy: M.Scalar): void {
        this.emitter.emitSkew(sx, sy);
    }

    public concat(matrix: Matrix4x4): void {
        this.checkAssemblerOwnership(matrix);
        this.emitter.emitConcat(matrix.getMemOperand());
    }

    public resetMatrix(): void {
        this.emitter.emitResetMatrix();
    }

    public setMatrix(matrix: Matrix4x4): void {
        this.checkAssemblerOwnership(matrix);
        this.emitter.emitSetMatrix(matrix.getMemOperand());
    }

    public clipRect(rect: M.Rect, op: ClipOp, doAntialias: boolean): void {
        let mr = this.allocateLocalRect(rect);
        this.emitter.emitClipRect(mr, op, BoolNumber(doAntialias));
        this.emitter.emitHeapFree(mr);
    }

    public clipRRect(rrect: RRect, op: ClipOp, doAntialias: boolean): void {
        this.checkAssemblerOwnership(rrect);
        this.emitter.emitClipRRect(rrect.getMemOperand(), op, BoolNumber(doAntialias));
    }

    public clipPath(path: Path, op: ClipOp, doAntialias: boolean): void {
        this.checkAssemblerOwnership(path);
        this.emitter.emitClipPath(path.getMemOperand(), op, BoolNumber(doAntialias));
    }

    // TODO: clipRegion

    public drawColor(color: M.Int, mode: BlendMode): void {
        this.emitter.emitDrawColor(color, mode);
    }

    public clear(color: M.Int): void {
        if (!Number.isInteger(color)) {
            throw new Error('Argument \'color\ should be an integer');
        }
        this.emitter.emitClear((color | 0) & 0xffffffff);
    }

    public drawPaint(paint: Paint): void {
        this.checkAssemblerOwnership(paint);
        this.emitter.emitDrawPaint(paint.getMemOperand());
    }

    public drawLine(x0: M.Scalar, y0: M.Scalar, x1: M.Scalar, y1: M.Scalar, paint: Paint): void {
        this.checkAssemblerOwnership(paint);
        this.emitter.emitDrawLine(x0, y0, x1, y1, paint.getMemOperand());
    }

    public drawRect(rect: M.Rect, paint: Paint): void {
        this.checkAssemblerOwnership(paint);
        let mr = this.allocateLocalRect(rect);
        this.emitter.emitDrawRect(mr, paint.getMemOperand());
        this.emitter.emitHeapFree(mr);
    }

    // TODO: drawRegion

    public drawOval(oval: M.Rect, paint: Paint): void {
        this.checkAssemblerOwnership(paint);
        let mr = this.allocateLocalRect(oval);
        this.emitter.emitDrawOval(mr, paint.getMemOperand());
        this.emitter.emitHeapFree(mr);
    }

    public drawRRect(rrect: RRect, paint: Paint): void {
        this.checkAssemblerOwnership(rrect);
        this.emitter.emitDrawRRect(rrect.getMemOperand(), paint.getMemOperand());
    }

    public drawDRRect(outer: RRect, inner: RRect, paint: Paint): void {
        this.checkAssemblerOwnership(outer);
        this.checkAssemblerOwnership(inner);
        this.checkAssemblerOwnership(paint);
        this.emitter.emitDrawDRRect(outer.getMemOperand(), inner.getMemOperand(), paint.getMemOperand());
    }

    public drawCircle(cx: M.Scalar, cy: M.Scalar, radius: M.Scalar, paint: Paint): void {
        this.checkAssemblerOwnership(paint);
        this.emitter.emitDrawCircle(cx, cy, radius, paint.getMemOperand());
    }

    public drawCirclev(c: M.Vector2, radius: M.Scalar, paint: Paint): void {
        this.drawCircle(c.x, c.y, radius, paint);
    }

    public drawArc(oval: M.Rect, startAngle: M.Scalar, sweepAngle: M.Scalar, useCenter: boolean, paint: Paint): void {
        let mr = this.allocateLocalRect(oval);
        this.emitter.emitDrawArc(mr, startAngle, sweepAngle, BoolNumber(useCenter), paint.getMemOperand());
        this.emitter.emitHeapFree(mr);
    }

    public drawPath(path: Path, paint: Paint): void {
        this.checkAssemblerOwnership(path);
        this.checkAssemblerOwnership(paint);
        this.emitter.emitDrawPath(path.getMemOperand(), paint.getMemOperand());
    }

    public drawPicture(picture: Glamor.CkPicture, matrix?: Matrix4x4, paint?: Paint): void {
        let mMatrix = MEMOP_NULL, mPaint = MEMOP_NULL;
        if (matrix) {
            this.checkAssemblerOwnership(matrix);
            mMatrix = matrix.getMemOperand();
        }
        if (paint) {
            this.checkAssemblerOwnership(paint);
            mPaint = paint.getMemOperand();
        }
        let mPicture = this.allocateLocalMemOpWithoutRef((op, e) => {
            this.heapBinder.bindPicture(op, picture);
            return MemOpTypes.kPicture;
        });
        
        this.emitter.emitDrawPicture(mPicture, mMatrix, mPaint);
    }

    public drawImage(image: Glamor.CkImage, left: M.Scalar, top: M.Scalar, sampling?: SamplingOptions, paint?: Paint): void {
        let mPaint = MEMOP_NULL, mSampling = MEMOP_NULL;
        if (paint) {
            this.checkAssemblerOwnership(paint);
            mPaint = paint.getMemOperand();
        }
        if (sampling) {
            sampling.associateAssembler(this);
            mSampling = sampling.getMemOperand();
        }
        let mImage = this.allocateLocalMemOpWithoutRef((op, e) => {
            this.heapBinder.bindImage(op, image);
            return MemOpTypes.kImage;
        });
        this.emitter.emitDrawImage(mImage, left, top, mSampling, mPaint);
    }

    public drawImageRect(image: Glamor.CkImage, dst: M.Rect, sampling?: SamplingOptions, paint?: Paint): void {
        let mDst = this.allocateLocalRect(dst);
        let mPaint = MEMOP_NULL, mSampling = MEMOP_NULL;
        if (paint) {
            this.checkAssemblerOwnership(paint);
            mPaint = paint.getMemOperand();
        }
        if (sampling) {
            sampling.associateAssembler(this);
            mSampling = sampling.getMemOperand();
        }
        let mImage = this.allocateLocalMemOpWithoutRef((op, e) => {
            this.heapBinder.bindImage(op, image);
            return MemOpTypes.kImage;
        });
        this.emitter.emitDrawImageRect(mImage, mDst, mSampling, mPaint);
        this.emitter.emitHeapFree(mDst);
    }

    // TODO: Other drawing operations...

    public insertDebugBreakpoint(func: () => void): void {
        this.breakpointIdCounter++;
        this.emitter.emitDebugBreakpoint(this.breakpointIdCounter);
        this.breakpointsMap.set(this.breakpointIdCounter, func);
    }

    /**
     * Recording of drawing operations is finished and destroy used
     * resources. This method must be called before `submit` or `toString()`.
     * Note that the IR buffers will NOT be released util `dispose` is called.
     */
    public finish(): void {
        if (this.status == CanvasStatus.kFinished ||
            this.status == CanvasStatus.kDisposed) {
            return;
        }
        this.emitter.emitCommandPoolEnd();
        this.status = CanvasStatus.kFinished;
    }

    /**
     * Release all the resources (underlying buffers) that it holds.
     * `dispose` must be called when the Canvas will not be used anymore
     * to avoid memory leaking.
     */
    public dispose(): void {
        if (this.status == CanvasStatus.kDisposed)
            return;
        if (this.status == CanvasStatus.kRecording)
            this.finish();
        this.writer.dispose();
        this.status = CanvasStatus.kDisposed;
    }

    /**
     * Submit the contents of Canvas to generate a GskPicture which can be
     * drawn on other Canvas, serialized, and rasterized.
     * 
     * @returns     An instance of GskIRCompileResult.
     *              Property `heapProfiling` is available if feature
     *              `CODEGEN_HEAP_PROFILING` is enabled.
     * 
     * @throws      {Error} if canvas is disposed or has not been finished.
     * @throws      {Error} if any error occurs while IR execution.
     */
    public submit(): Glamor.MoeTranslationResult {
        this.checkStatus(CanvasStatus.kFinished);
        let heapProfiling = _features.get(Features.CODEGEN_HEAP_PROFILING);

        // TODO: passing a proper breakpoint callback function
        return Glamor.MoeTranslationToolchain.Interpreter(this.writer.getBuffers(),
                                                          this.heapBinder,
                                                          () => {},
                                                          heapProfiling);
    }

    public compressToBuffer(): std.Buffer {
        this.checkStatus(CanvasStatus.kFinished);
        return Glamor.MoeTranslationToolchain.Compress(this.writer.getBuffers());
    }

    /**
     * Get a string representation of generated IR code.
     * 
     * @throws      Error if canvas is disposed or has not been finished.
     */
    public toString(): string {
        this.checkStatus(CanvasStatus.kFinished);
        return Glamor.MoeTranslationToolchain.Disassemble(this.writer.getBuffers());
    }
}

export class Matrix4x4 extends HeapObjectWithAssemblerBase {
    constructor(canvas: Canvas) {
        super(canvas, MemOpTypes.kMatrix4x4, (op, as) => {
            as.getEmitter().emitHeapCreateM44(op);
        });
    }

    public setRows(r0: M.Vector4, r1: M.Vector4, r2: M.Vector4, r3: M.Vector4): void {
        let mr0 = this.getAssembler().allocateLocalVector4(r0),
            mr1 = this.getAssembler().allocateLocalVector4(r1),
            mr2 = this.getAssembler().allocateLocalVector4(r2),
            mr3 = this.getAssembler().allocateLocalVector4(r3);
        this.getEmitter().emitM44SetRows(this.getMemOperand(), mr0, mr1, mr2, mr3);
        this.getEmitter().emitHeapFree(mr0);
        this.getEmitter().emitHeapFree(mr1);
        this.getEmitter().emitHeapFree(mr2);
        this.getEmitter().emitHeapFree(mr3);
    }

    public setCols(c0: M.Vector4, c1: M.Vector4, c2: M.Vector4, c3: M.Vector4): void {
        let mc0 = this.getAssembler().allocateLocalVector4(c0),
            mc1 = this.getAssembler().allocateLocalVector4(c1),
            mc2 = this.getAssembler().allocateLocalVector4(c2),
            mc3 = this.getAssembler().allocateLocalVector4(c3);
        this.getEmitter().emitM44SetCols(this.getMemOperand(), mc0, mc1, mc2, mc3);
        this.getEmitter().emitHeapFree(mc0);
        this.getEmitter().emitHeapFree(mc1);
        this.getEmitter().emitHeapFree(mc2);
        this.getEmitter().emitHeapFree(mc3);
    }

    public setTranslate(x: M.Scalar, y: M.Scalar, z: M.Scalar): void {
        this.getEmitter().emitM44SetTranslate(this.getMemOperand(), x, y, z);
    }

    public setScale(x: M.Scalar, y: M.Scalar, z: M.Scalar): void {
        this.getEmitter().emitM44SetScale(this.getMemOperand(), x, y, z);
    }

    public setRectToRect(src: M.Rect, dst: M.Rect): void {
        let mSrc = this.getAssembler().allocateLocalRect(src),
            mDst = this.getAssembler().allocateLocalRect(dst);
        this.getEmitter().emitM44SetRectToRect(this.getMemOperand(), mSrc, mDst);
        this.getEmitter().emitHeapFree(mSrc);
        this.getEmitter().emitHeapFree(mDst);
    }

    public setLookAt(eye: M.Vector3, center: M.Vector3, up: M.Vector3): void {
        let mEye = this.getAssembler().allocateLocalVector3(eye),
            mCenter = this.getAssembler().allocateLocalVector3(center),
            mUp = this.getAssembler().allocateLocalVector3(up);
        this.getEmitter().emitM44SetLookAt(this.getMemOperand(), mEye, mCenter, mUp);
        this.getEmitter().emitHeapFree(mEye);
        this.getEmitter().emitHeapFree(mCenter);
        this.getEmitter().emitHeapFree(mUp);
    }

    public setPerspective(near: M.Scalar, far: M.Scalar, angle: M.Scalar): void {
        this.getEmitter().emitM44SetPerspective(this.getMemOperand(), near, far, angle);
    }

    public setIdentity(): void {
        this.getEmitter().emitM44SetIdentity(this.getMemOperand());
    }

    public concat(other: Matrix4x4): void {
        this.getAssembler().checkAssemblerOwnership(other);
        this.getEmitter().emitM44Concat(this.getMemOperand(), other.getMemOperand());
    }
}

export class SamplingOptions extends StatelessImmutableHeapObjectBase {
    private constructor(factory: (op: MemOp, as: IRAssemblerBase) => void) {
        super(MemOpTypes.kSamplingOptions, factory);
    }

    public static Make(filterMode: FilterMode, mipmapMode: MipmapMode): SamplingOptions {
        return new SamplingOptions((op, as) => {
            as.getEmitter().emitHeapCreateSamplingOptions(op, filterMode, mipmapMode);
        });
    }

    public static MakeCubicMitchell(): SamplingOptions {
        return new SamplingOptions((op, as) => {
            as.getEmitter().emitHeapCreateSamplingOptionsCubicMitchell(op);
        });
    }

    public static MakeCubicCatmullRom(): SamplingOptions {
        return new SamplingOptions((op, as) => {
            as.getEmitter().emitHeapCreateSamplingOptionsCubicCatmullRom(op);
        });
    }
}

export class RRect extends HeapObjectWithAssemblerBase {
    constructor(canvas: Canvas) {
        super(canvas, MemOpTypes.kRRect, (op, as) => {
            as.getEmitter().emitHeapCreateRRectEmpty(op);
        });
    }

    public setOval(oval: M.Rect): void {
        let mr = this.getAssembler().allocateLocalRect(oval);
        this.getEmitter().emitRRectSetOval(this.getMemOperand(), mr);
        this.getEmitter().emitHeapFree(mr);
    }

    public setNinePatch(rect: M.Rect, leftRad: M.Scalar, topRad: M.Scalar,
                        rightRad: M.Scalar, bottomRad: M.Scalar): void {
        let mr = this.getAssembler().allocateLocalRect(rect);
        this.getEmitter().emitRRectSetNinePatch(this.getMemOperand(), mr,
                                                leftRad, topRad, rightRad, bottomRad);
        this.getEmitter().emitHeapFree(mr);
    }

    public setRectXY(rect: M.Rect, xRad: M.Scalar, yRad: M.Scalar): void {
        let mr = this.getAssembler().allocateLocalRect(rect);
        this.getEmitter().emitRRectSetRectXY(this.getMemOperand(), mr, xRad, yRad);
        this.getEmitter().emitHeapFree(mr);
    }

    public inset(dx: M.Scalar, dy: M.Scalar): void {
        this.getEmitter().emitRRectInset(this.getMemOperand(), dx, dy);
    }

    public outset(dx: M.Scalar, dy: M.Scalar): void {
        this.getEmitter().emitRRectOutset(this.getMemOperand(), dx, dy);
    }

    public offset(dx: M.Scalar, dy: M.Scalar): void {
        this.getEmitter().emitRRectOffset(this.getMemOperand(), dx, dy);
    }
}

export class Path extends HeapObjectWithAssemblerBase {
    constructor(canvas: Canvas) {
        super(canvas, MemOpTypes.kPath, (op, assembler) => {
            assembler.getEmitter().emitHeapCreatePath(op);
        });
    }

    public setPathFillType(type: PathFillType): void {
        this.getEmitter().emitPathSetPathFillType(this.getMemOperand(), type);
    }

    public toggleInverseFillType(): void {
        this.getEmitter().emitPathToggleInverseFillType(this.getMemOperand());
    }

    public reset(): void {
        this.getEmitter().emitPathReset(this.getMemOperand());
    }


    public moveTo(x: M.Scalar, y: M.Scalar): void {
        this.getEmitter().emitPathMoveTo(this.getMemOperand(), x, y);
    }

    public moveTov(vec: M.Vector2): void {
        this.getEmitter().emitPathMoveTo(this.getMemOperand(), vec.x, vec.y);
    }

    public rMoveTo(x: M.Scalar, y: M.Scalar): void {
        this.getEmitter().emitPathRMoveTo(this.getMemOperand(), x, y);
    }

    public rMoveTov(vec: M.Vector2): void {
        this.getEmitter().emitPathRMoveTo(this.getMemOperand(), vec.x, vec.y);
    }

    public lineTo(x: M.Scalar, y: M.Scalar): void {
        this.getEmitter().emitPathLineTo(this.getMemOperand(), x, y);
    }

    public lineTov(vec: M.Vector2): void {
        this.getEmitter().emitPathLineTo(this.getMemOperand(), vec.x, vec.y);
    }

    public rLineTo(x: M.Scalar, y: M.Scalar): void {
        this.getEmitter().emitPathRLineTo(this.getMemOperand(), x, y);
    }

    public rLineTov(vec: M.Vector2): void {
        this.getEmitter().emitPathRLineTo(this.getMemOperand(), vec.x, vec.y);
    }

    public quadTo(x1: M.Scalar, y1: M.Scalar, x2: M.Scalar, y2: M.Scalar): void {
        this.getEmitter().emitPathQuadTo(this.getMemOperand(), x1, y1, x2, y2);
    }

    public quadTov(v1: M.Vector2, v2: M.Vector2): void {
        this.getEmitter().emitPathQuadTo(this.getMemOperand(), v1.x, v1.y, v2.x, v2.y);
    }

    public rQuadTo(x1: M.Scalar, y1: M.Scalar, x2: M.Scalar, y2: M.Scalar): void {
        this.getEmitter().emitPathRQuadTo(this.getMemOperand(), x1, y1, x2, y2);
    }

    public rQuadTov(v1: M.Vector2, v2: M.Vector2): void {
        this.getEmitter().emitPathRQuadTo(this.getMemOperand(), v1.x, v1.y, v2.x, v2.y);
    }

    public conicTo(x1: M.Scalar, y1: M.Scalar, x2: M.Scalar, y2: M.Scalar, w: M.Scalar): void {
        this.getEmitter().emitPathConicTo(this.getMemOperand(), x1, y1, x2, y2, w);
    }

    public conicTov(v1: M.Vector2, v2: M.Vector2, w: M.Scalar): void {
        this.getEmitter().emitPathConicTo(this.getMemOperand(), v1.x, v1.y, v2.x, v2.y, w);
    }

    public rConicTo(x1: M.Scalar, y1: M.Scalar, x2: M.Scalar, y2: M.Scalar, w: M.Scalar): void {
        this.getEmitter().emitPathRConicTo(this.getMemOperand(), x1, y1, x2, y2, w);
    }

    public rConicTov(v1: M.Vector2, v2: M.Vector2, w: M.Scalar): void {
        this.getEmitter().emitPathRConicTo(this.getMemOperand(), v1.x, v1.y, v2.x, v2.y, w);
    }

    public cubicTo(x1: M.Scalar, y1: M.Scalar, x2: M.Scalar, y2: M.Scalar, x3: M.Scalar, y3: M.Scalar): void {
        this.getEmitter().emitPathCubicTo(this.getMemOperand(), x1, y1, x2, y2, x3, y3);
    }

    public cubicTov(v1: M.Vector2, v2: M.Vector2, v3: M.Vector2): void {
        this.getEmitter().emitPathCubicTo(this.getMemOperand(), v1.x, v1.y, v2.x, v2.y, v3.x, v3.y);
    }

    public rCubicTo(x1: M.Scalar, y1: M.Scalar, x2: M.Scalar, y2: M.Scalar, x3: M.Scalar, y3: M.Scalar): void {
        this.getEmitter().emitPathRCubicTo(this.getMemOperand(), x1, y1, x2, y2, x3, y3);
    }

    public rCubicTov(v1: M.Vector2, v2: M.Vector2, v3: M.Vector2): void {
        this.getEmitter().emitPathRCubicTo(this.getMemOperand(), v1.x, v1.y, v2.x, v2.y, v3.x, v3.y);
    }

    public rectArcTo(oval: M.Rect, startAngle: M.Scalar, sweepAngle: M.Scalar, forceMoveTo: boolean): void {
        let ovalMemOp = this.getAssembler().allocateLocalRect(oval);
        this.getEmitter().emitPathRectArcTo(this.getMemOperand(), ovalMemOp, startAngle,
                                            sweepAngle, BoolNumber(forceMoveTo));
        this.getEmitter().emitHeapFree(ovalMemOp);
    }

    public tangentArcTo(x1: M.Scalar, y1: M.Scalar, x2: M.Scalar, y2: M.Scalar, radius: M.Scalar): void {
        this.getEmitter().emitPathTangentArcTo(this.getMemOperand(), x1, y1, x2, y2, radius);
    }

    public tangentArcTov(v1: M.Vector2, v2: M.Vector2, radius: M.Scalar): void {
        this.getEmitter().emitPathTangentArcTo(this.getMemOperand(), v1.x, v1.y, v2.x, v2.y, radius);
    }

    public rotateArcTo(rx: M.Scalar, ry: M.Scalar, xAxisRotate: M.Scalar, largeArc: boolean, ccwSweep: boolean,
                       x: M.Scalar, y: M.Scalar): void {
        this.getEmitter().emitPathRotateArcTo(this.getMemOperand(), rx, ry, xAxisRotate, BoolNumber(largeArc),
                                              BoolNumber(ccwSweep), x, y);
    }

    public rotateArcTov(rv: M.Vector2, xAxisRotate: M.Scalar, largeArc: boolean, ccwSweep: boolean, pos: M.Vector2): void {
        this.getEmitter().emitPathRotateArcTo(this.getMemOperand(), rv.x, rv.y, xAxisRotate, BoolNumber(largeArc),
                                              BoolNumber(ccwSweep), pos.x, pos.y);
    }

    public rRotateArcTo(rx: M.Scalar, ry: M.Scalar, xAxisRotate: M.Scalar, largeArc: boolean, ccwSweep: boolean, x: M.Scalar, y: M.Scalar): void {
        this.getEmitter().emitPathRRotateArcTo(this.getMemOperand(), rx, ry, xAxisRotate, BoolNumber(largeArc),
                                               BoolNumber(ccwSweep), x, y);
    }

    public rRotateArcTov(rv: M.Vector2, xAxisRotate: M.Scalar, largeArc: boolean, ccwSweep: boolean, pos: M.Vector2): void {
        this.getEmitter().emitPathRRotateArcTo(this.getMemOperand(), rv.x, rv.y, xAxisRotate, BoolNumber(largeArc),
                                               BoolNumber(ccwSweep), pos.x, pos.y);
    }

    public close(): void {
        this.getEmitter().emitPathClose(this.getMemOperand());
    }

    public addRect(rect: M.Rect, ccw: boolean = false): void {
        let rectMemOp = this.getAssembler().allocateLocalRect(rect);
        this.getEmitter().emitPathAddRect(this.getMemOperand(), rectMemOp, BoolNumber(ccw));
        this.getEmitter().emitHeapFree(rectMemOp);
    }

    public addOval(oval: M.Rect, ccw: boolean = false): void {
        let ovalMemOp = this.getAssembler().allocateLocalRect(oval);
        this.getEmitter().emitPathAddOval(this.getMemOperand(), ovalMemOp, BoolNumber(ccw));
        this.getEmitter().emitHeapFree(ovalMemOp);
    }

    public addCircle(x: M.Scalar, y: M.Scalar, radius: M.Scalar, ccw: boolean = false): void {
        this.getEmitter().emitPathAddCircle(this.getMemOperand(), x, y, radius, BoolNumber(ccw));
    }

    public addCirclev(v: M.Vector2, radius: M.Scalar, ccw: boolean = false): void {
        this.getEmitter().emitPathAddCircle(this.getMemOperand(), v.x, v.y, radius, BoolNumber(ccw));
    }

    public addArc(oval: M.Rect, startAngle: M.Scalar, sweepAngle: M.Scalar): void {
        let ovalMemOp = this.getAssembler().allocateLocalRect(oval);
        this.getEmitter().emitPathAddArc(this.getMemOperand(), ovalMemOp, startAngle, sweepAngle);
        this.getEmitter().emitHeapFree(ovalMemOp);
    }

    public addUniformRoundRect(rect: M.Rect, rx: M.Scalar, ry: M.Scalar, ccw: boolean = false): void {
        let rectMemOp = this.getAssembler().allocateLocalRect(rect);
        this.getEmitter().emitPathAddUniformRoundRect(this.getMemOperand(), rectMemOp, rx, ry, BoolNumber(ccw));
        this.getEmitter().emitHeapFree(rectMemOp);
    }

    public addUniformRoundRectv(rect: M.Rect, rv: M.Vector2, ccw: boolean = false): void {
        this.addUniformRoundRect(rect, rv.x, rv.y, ccw);
    }

    public addRoundRect(rect: M.Rect, radii: M.Scalar[], ccw: boolean = false): void {
        if (radii.length != 8) {
            throw new Error('Argument \'radii\' must be an array of 8 numbers');
        }
        let rectMemOp = this.getAssembler().allocateLocalRect(rect);
        let arrayMemOp = this.getAssembler().allocateLocalMemOpWithoutRef((op, e) => {
            e.emitHeapCreateF32Array(op, 8);
            return MemOpTypes.kF32Array;
        });
        for (let i = 0; i < radii.length; i++) {
            this.getEmitter().emitHeapF32ArrayStore(arrayMemOp, i, radii[i]);
        }
        this.getEmitter().emitPathAddRoundRect(this.getMemOperand(), rectMemOp, arrayMemOp, BoolNumber(ccw));
        this.getEmitter().emitHeapFree(rectMemOp);
        this.getEmitter().emitHeapFree(arrayMemOp);
    }

    public addPoly(vertXY: M.Scalar[], close: boolean): void {
        if (vertXY.length & 1) {
            throw new Error('\'vertXY\' array has invalid length (expecting even number)');
        }
        let arrayMemOp = this.getAssembler().allocateLocalMemOpWithoutRef((op, e) => {
            e.emitHeapCreateF32Array(op, vertXY.length);
            return MemOpTypes.kF32Array;
        });
        for (let i = 0; i < vertXY.length; i++) {
            this.getEmitter().emitHeapF32ArrayStore(arrayMemOp, i, vertXY[i]);
        }
        this.getEmitter().emitPathAddPoly(this.getMemOperand(), arrayMemOp, BoolNumber(close));
        this.getEmitter().emitHeapFree(arrayMemOp);
    }

    public addPolyv(vert: M.Vector2[], close: boolean): void {
        let extractedArray = Array<M.Scalar>(vert.length * 2);
        for (let i = 0; i < vert.length; i++) {
            extractedArray[i * 2] = vert[i].x;
            extractedArray[i * 2 + 1] = vert[i].y;
        }
        this.addPoly(extractedArray, close);
    }

    public transform(mat: Matrix4x4, dst: Path, applyPerspectiveClip: boolean): void {
        this.getAssembler().checkAssemblerOwnership(mat);
        this.getAssembler().checkAssemblerOwnership(dst);
        this.getEmitter().emitPathTransform(this.getMemOperand(),
                                            mat.getMemOperand(),
                                            dst.getMemOperand(),
                                            BoolNumber(applyPerspectiveClip));
    }
}

export class Paint extends HeapObjectWithAssemblerBase {
    constructor(canvas: Canvas) {
        super(canvas, MemOpTypes.kPaint, (op, as) => {
            as.getEmitter().emitHeapCreatePaint(op);
        });
    }

    public reset(): void {
        this.getEmitter().emitPaintReset(this.getMemOperand());
    }

    public setAntialias(aa: boolean): void {
        this.getEmitter().emitPaintSetAntialias(this.getMemOperand(), BoolNumber(aa));
    }

    public setDither(dither: boolean): void {
        this.getEmitter().emitPaintSetDither(this.getMemOperand(), BoolNumber(dither));
    }

    public setStyleStroke(stroke: boolean): void {
        this.getEmitter().emitPaintSetStyleStroke(this.getMemOperand(), BoolNumber(stroke));
    }

    public setColor(color: M.Int): void {
        if (!Number.isInteger(color)) {
            throw new Error('Argument \'color\' must be an integer');
        }
        this.getEmitter().emitPaintSetColor(this.getMemOperand(), (color | 0) & 0xffffffff);
    }

    public setAlphaf(alpha: M.Scalar): void {
        alpha = Math.min(Math.max(0, alpha));
        this.getEmitter().emitPaintSetAlphaf(this.getMemOperand(), alpha);
    }

    public setStrokeWidth(width: M.Scalar): void {
        this.getEmitter().emitPaintSetStrokeWidth(this.getMemOperand(), width);
    }

    public setStrokeMiter(miter: M.Scalar): void {
        this.getEmitter().emitPaintSetStrokeMiter(this.getMemOperand(), miter);
    }

    public setStrokeCap(cap: PaintCap): void {
        this.getEmitter().emitPaintSetStrokeCap(this.getMemOperand(), cap);
    }

    public setStrokeJoin(join: PaintJoin): void {
        this.getEmitter().emitPaintSetStrokeJoin(this.getMemOperand(), join);
    }

    public setBlendMode(mode: BlendMode): void {
        this.getEmitter().emitPaintSetBlendMode(this.getMemOperand(), mode);
    }

    public setShader(shader: Shader): void {
        shader.associateAssembler(this.assembler);
        this.getEmitter().emitPaintSetShader(this.getMemOperand(), shader.getMemOperand());
    }

    public setBlender(blender: Blender): void {
        blender.associateAssembler(this.assembler);
        this.getEmitter().emitPaintSetBlender(this.getMemOperand(), blender.getMemOperand());
    }

    public setColorFilter(filter: ColorFilter): void {
        filter.associateAssembler(this.assembler);
        this.getEmitter().emitPaintSetColorFilter(this.getMemOperand(), filter.getMemOperand());
    }

    public setImageFilter(filter: ImageFilter): void {
        filter.associateAssembler(this.assembler);
        this.getEmitter().emitPaintSetImageFilter(this.getMemOperand(), filter.getMemOperand());
    }

    public setPathEffect(effect: PathEffect): void {
        effect.associateAssembler(this.assembler);
        this.getEmitter().emitPaintSetPathEffect(this.getMemOperand(), effect.getMemOperand());
    }

    public setMaskFilter(filter: MaskFilter): void {
        filter.associateAssembler(this.assembler);
        this.getEmitter().emitPaintSetMaskFilter(this.getMemOperand(), filter.getMemOperand());
    }
}

export class Shader extends StatelessImmutableHeapObjectBase {
    private constructor(creator: (operand: MemOp, as: IRAssemblerBase) => void) {
        super(MemOpTypes.kShader, (op, as) => {
            creator(op, as);
        });
    }

    public static MakeEmpty(): Shader {
        return new Shader((op, as) => {
            return as.getEmitter().emitHeapCreateEmptyShader(op);
        });
    }

    public static MakeColor(color: M.Int): Shader {
        return new Shader((op, as) => {
            return as.getEmitter().emitHeapCreateColorShader(op, color);
        });
    }

    public static MakeBlend(blender: Blender, dst: Shader, src: Shader): Shader {
        return new Shader((op, as) => {
            blender.associateAssembler(as);
            dst.associateAssembler(as);
            src.associateAssembler(as);
            return as.getEmitter().emitHeapCreateBlendShader(op, blender.getMemOperand(),
                                                             dst.getMemOperand(), src.getMemOperand());
        });
    }

    // TODO: other shaders.
}

export class Blender extends StatelessImmutableHeapObjectBase {
}

export class ColorFilter extends StatelessImmutableHeapObjectBase {
}

export class ImageFilter extends StatelessImmutableHeapObjectBase {
    private constructor(factory: (operand: MemOp, as: IRAssemblerBase) => void) {
        super(MemOpTypes.kImageFilter, factory);
    }

    public static MakeBlur(sigmaX: M.Scalar, sigmaY: M.Scalar, tileMode: TileMode,
                           input?: ImageFilter, crop?: M.Rect): ImageFilter {
        return new ImageFilter((op, as) => {
            let mInput = MEMOP_NULL, mCrop = MEMOP_NULL;
            if (input) {
                input.associateAssembler(as);
                mInput = input.getMemOperand();
            }
            if (crop) {
                mCrop = as.allocateLocalRect(crop);
            }
            as.getEmitter().emitHeapCreateBlurImageFilter(op, sigmaX, sigmaY, tileMode, mInput, mCrop);
            if (mCrop != MEMOP_NULL) {
                as.getEmitter().emitHeapFree(mCrop);
            }
        });
    }
}

export class PathEffect extends StatelessImmutableHeapObjectBase {
}

export class MaskFilter extends StatelessImmutableHeapObjectBase {
}
