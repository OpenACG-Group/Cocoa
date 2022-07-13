/**
 * This module generates GSKIR (GSK Intermediate Representation) bytecode
 * and submit it to native rendering engine, who executes the submitted bytecode and
 * generates drawing instructions. Those instructions will be performed by CPU or GPU
 * asynchronously depending on whether the hardware-acceleration is available.
 */
import * as std from 'core';
import * as Glamor from 'glamor';
import { Opcode, Constants, ProtoCodeEmitter } from "./generated/code_emitter";
import * as M from './comath';
const PROTO_BUFFER_UNIT_BUFFER_SIZE = 4096;
const PROTO_OPCODE_BYTE_SIZE = 2;
const MEMOP_NULL = 0;
export var Features;
(function (Features) {
    Features.CODEGEN_HEAP_PROFILING = 'codegen-heap-profiling';
    Features.CODEGEN_REDUNDANT_ALLOCATION_ANALYSIS = 'codegen-redundant-allocation-analysis';
})(Features || (Features = {}));
let _features = new Map([
    [Features.CODEGEN_HEAP_PROFILING, false],
    [Features.CODEGEN_REDUNDANT_ALLOCATION_ANALYSIS, false]
]);
/**
 * Enable or disable features of CanvasKit.
 * @param name     Name of feature. See also namespace `Features`.
 * @param value    `true` to enable the feature, while `false` to disable it.
 */
export function setCanvasFeature(name, value) {
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
export function U32ColorFromUnorm(a, r, g, b) {
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
export function U32ColorFromNorm(a, r, g, b) {
    let u8a = M.linearInterpolate(0, 0xff, a);
    let u8r = M.linearInterpolate(0, 0xff, r);
    let u8g = M.linearInterpolate(0, 0xff, g);
    let u8b = M.linearInterpolate(0, 0xff, b);
    return U32ColorFromUnorm(u8a, u8r, u8g, u8b);
}
/**
 * A convenient function of `U32ColorFromNorm` with `v = (A, R, G, B)`.
 */
export function U32ColorFromVector4Norm(v) {
    return U32ColorFromNorm(v.x, v.y, v.z, v.w);
}
/**
 * A convenient function of `U32ColorFromNorm` with `v = (R, G, B)`.
 */
export function U32ColorFromVector3Norm(v, alpha = 1.0) {
    return U32ColorFromNorm(alpha, v.x, v.y, v.z);
}
export class HeapNullReferenceError extends Error {
    constructor(typename) {
        super(`Reference to heap object '${typename}' should not be NULL`);
        this.typename = typename;
    }
}
export class AssemblerError extends Error {
    constructor(what) { super(what); }
}
export class HeapObjectOwnershipError extends Error {
    constructor(what) { super(what); }
}
function BoolNumber(v) {
    return v ? 1 : 0;
}
class ProtoBufferWriter {
    constructor(context) {
        this.context = context;
        let bufIndex = context.acquireBuffer();
        this.buffersVector = [{ index: bufIndex, buffer: context.getBufferFromIndex(bufIndex) }];
        this.currentBuffer = this.buffersVector[0].buffer;
        this.currentBufferPos = 0;
        this.dataView = this.currentBuffer.toDataView();
    }
    dispose() {
        this.currentBuffer = null;
        this.currentBufferPos = 0;
        for (let buffer of this.buffersVector) {
            this.context.releaseBuffer(buffer.index);
        }
        this.buffersVector = [];
        this.dataView = null;
    }
    getBuffers() {
        let result = [];
        for (let obj of this.buffersVector) {
            result.push(obj.buffer);
        }
        return result;
    }
    switchToNextBuffer() {
        let bufIndex = this.context.acquireBuffer();
        this.buffersVector.push({
            index: bufIndex,
            buffer: this.context.getBufferFromIndex(bufIndex)
        });
        this.currentBuffer = this.context.getBufferFromIndex(bufIndex);
        this.currentBufferPos = 0;
        this.dataView = this.currentBuffer.toDataView();
    }
    performPossibleBufferSwitching(requireSize) {
        let size = requireSize + PROTO_OPCODE_BYTE_SIZE;
        let remaining = this.currentBuffer.length - this.currentBufferPos;
        if (size >= remaining) {
            this.writeUint16Unsafe(Opcode.kSwitchNextBuffer | 0x0);
            this.switchToNextBuffer();
        }
    }
    writeInt8Unsafe(x) {
        this.dataView.setInt8(this.currentBufferPos, x | 0);
        this.currentBufferPos += 1;
    }
    writeUint8Unsafe(x) {
        this.dataView.setUint8(this.currentBufferPos, x | 0);
        this.currentBufferPos += 1;
    }
    writeInt16Unsafe(x) {
        this.dataView.setInt16(this.currentBufferPos, x | 0, true);
        this.currentBufferPos += 2;
    }
    writeUint16Unsafe(x) {
        this.dataView.setUint16(this.currentBufferPos, x | 0, true);
        this.currentBufferPos += 2;
    }
    writeInt32Unsafe(x) {
        this.dataView.setInt32(this.currentBufferPos, x | 0, true);
        this.currentBufferPos += 4;
    }
    writeUint32Unsafe(x) {
        this.dataView.setUint32(this.currentBufferPos, x | 0, true);
        this.currentBufferPos += 4;
    }
    writeFloat32Unsafe(x) {
        this.dataView.setFloat32(this.currentBufferPos, x, true);
        this.currentBufferPos += 4;
    }
    writeFloat64Unsafe(x) {
        this.dataView.setFloat64(this.currentBufferPos, x, true);
        this.currentBufferPos += 8;
    }
    writeInt64Unsafe(x) {
        this.dataView.setBigInt64(this.currentBufferPos, x, true);
        this.currentBufferPos += 8;
    }
    writeUint64Unsafe(x) {
        this.dataView.setBigUint64(this.currentBufferPos, x, true);
        this.currentBufferPos += 8;
    }
}
export class MemoryResourceGroup {
    constructor() {
        this.pool = new Array();
        this.statusBitmap = new Uint8Array(1024);
        this.singleBufferSize = PROTO_BUFFER_UNIT_BUFFER_SIZE;
        // TODO: adjust `singleBufferSize` based on history allocations at runtime
        //       to reduce memory pressure.
    }
    acquireBuffer() {
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
        let idx = group << 3; // idx = group * 8;
        while (bits & 1) {
            bits >>>= 1; // bits *= 2
            idx++;
        }
        this.statusBitmap[group] |= 1 << (idx & 7);
        // std.print(`# Allocate index ${idx}, group ${group}\n`);
        if (idx > this.pool.length) {
            throw new Error('Corrupted memory resource group');
        }
        else if (idx == this.pool.length) {
            this.pool.push(new std.Buffer(this.singleBufferSize));
        }
        return idx;
    }
    releaseBuffer(idx) {
        if (!Number.isInteger(idx) || idx >= this.pool.length) {
            throw new TypeError('Invalid index of buffer in pool');
        }
        this.statusBitmap[idx >> 3] &= ~(1 << (idx & 7));
    }
    getBufferFromIndex(idx) {
        if (!Number.isInteger(idx) || idx >= this.pool.length) {
            throw new TypeError('Invalid index of buffer in pool');
        }
        return this.pool[idx];
    }
}
var MemOpTypes;
(function (MemOpTypes) {
    MemOpTypes["kString"] = "string";
    MemOpTypes["kU32Array"] = "u32array";
    MemOpTypes["kF32Array"] = "f32array";
    MemOpTypes["kMatrix3x3"] = "mat3x3";
    MemOpTypes["kMatrix4x4"] = "mat4x4";
    MemOpTypes["kVector2"] = "vec2";
    MemOpTypes["kVector3"] = "vec3";
    MemOpTypes["kVector4"] = "vec4";
    MemOpTypes["kRect"] = "rect";
    MemOpTypes["kRRect"] = "rrect";
    MemOpTypes["kRegion"] = "region";
    MemOpTypes["kPaint"] = "paint";
    MemOpTypes["kPath"] = "path";
    MemOpTypes["kSamplingOptions"] = "samplingoptions";
    MemOpTypes["kShader"] = "shader";
    MemOpTypes["kBlender"] = "blender";
    MemOpTypes["kColorFilter"] = "colorfilter";
    MemOpTypes["kImageFilter"] = "imagefilter";
    MemOpTypes["kMaskFilter"] = "maskfilter";
    MemOpTypes["kPathEffect"] = "patheffect";
    MemOpTypes["kImage"] = "image";
    MemOpTypes["kBitmap"] = "bitmap";
    MemOpTypes["kPicture"] = "picture";
})(MemOpTypes || (MemOpTypes = {}));
class IRAssemblerBase {
    constructor(emitter) {
        this.emitter = emitter;
        this.memOpIdCounter = 0;
        this.memOpRefCounts = new Map();
    }
    getEmitter() {
        return this.emitter;
    }
    refMemOp(op) {
        if (op.memOp == MEMOP_NULL) {
            throw new HeapNullReferenceError(op.type);
        }
        let v = this.memOpRefCounts.has(op.memOp) ? this.memOpRefCounts.get(op.memOp) : 0;
        this.memOpRefCounts.set(op.memOp, v + 1);
    }
    unrefMemOp(op) {
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
        }
        else {
            this.memOpRefCounts.set(op.memOp, v);
        }
    }
    allocateMemOpWithRef(factoryFunc) {
        let op = ++this.memOpIdCounter;
        this.refMemOp({ type: factoryFunc(op, this.emitter), memOp: op });
        return op;
    }
    allocateLocalMemOpWithoutRef(factoryFunc) {
        let op = ++this.memOpIdCounter;
        factoryFunc(op, this.emitter);
        return op;
    }
    allocateLocalRect(rect) {
        return this.allocateLocalMemOpWithoutRef((op, e) => {
            this.emitter.emitHeapCreateRect(op, rect.x(), rect.y(), rect.width(), rect.height());
            return MemOpTypes.kRect;
        });
    }
    allocateLocalVector2(v) {
        return this.allocateLocalMemOpWithoutRef((op, e) => {
            this.emitter.emitHeapCreateVector2(op, v.x, v.y);
            return MemOpTypes.kVector2;
        });
    }
    allocateLocalVector3(v) {
        return this.allocateLocalMemOpWithoutRef((op, e) => {
            this.emitter.emitHeapCreateVector3(op, v.x, v.y, v.z);
            return MemOpTypes.kVector3;
        });
    }
    allocateLocalVector4(v) {
        return this.allocateLocalMemOpWithoutRef((op, e) => {
            this.emitter.emitHeapCreateVector4(op, v.x, v.y, v.z, v.w);
            return MemOpTypes.kVector4;
        });
    }
    checkAssemblerOwnership(heapObject) {
        if (heapObject.getAssembler() != this) {
            throw new HeapObjectOwnershipError('Heap object is not owned by current assembler');
        }
    }
}
class HeapObjectWithAssemblerBase {
    constructor(assembler, type, factory) {
        this.assembler = assembler;
        let resultOp = this.assembler.allocateMemOpWithRef((op) => {
            factory(op, this.assembler);
            return type;
        });
        this.heapMemOp = { type: type, memOp: resultOp };
    }
    getEmitter() {
        return this.assembler.getEmitter();
    }
    getMemOperand() {
        return this.heapMemOp.memOp;
    }
    getAssembler() {
        return this.assembler;
    }
    ref() {
        this.assembler.refMemOp(this.heapMemOp);
    }
    unref() {
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
class StatelessImmutableHeapObjectBase {
    constructor(type, factory) {
        this.type = type;
        this.factory = factory;
        this.associatedAssembler = null;
        this.heapMemOp = null;
    }
    associateAssembler(assembler) {
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
    ref() {
        if (!this.heapMemOp) {
            throw new HeapNullReferenceError('ref an unassociated immutable heap object');
        }
        this.associatedAssembler.refMemOp(this.heapMemOp);
    }
    unref() {
        if (!this.heapMemOp) {
            throw new HeapNullReferenceError('unref an unassociated immutable heap object');
        }
        this.associatedAssembler.unrefMemOp(this.heapMemOp);
    }
    isAssociated() {
        return (this.associatedAssembler != null);
    }
    resetAssociation() {
        this.associatedAssembler = null;
        this.heapMemOp = null;
    }
    getMemOperand() {
        if (!this.heapMemOp) {
            throw new HeapNullReferenceError('Unassociated immutable heap object');
        }
        return this.heapMemOp.memOp;
    }
}
/* Paint parameters */
export var PaintCap;
(function (PaintCap) {
    PaintCap[PaintCap["kButt"] = Constants.PAINTCAP_BUTT] = "kButt";
    PaintCap[PaintCap["kRound"] = Constants.PAINTCAP_ROUND] = "kRound";
    PaintCap[PaintCap["kSquare"] = Constants.PAINTCAP_SQUARE] = "kSquare";
})(PaintCap || (PaintCap = {}));
export var PaintJoin;
(function (PaintJoin) {
    PaintJoin[PaintJoin["kMiter"] = Constants.PAINTJOIN_MITER] = "kMiter";
    PaintJoin[PaintJoin["kRound"] = Constants.PAINTJOIN_ROUND] = "kRound";
    PaintJoin[PaintJoin["kBevel"] = Constants.PAINTJOIN_BEVEL] = "kBevel";
})(PaintJoin || (PaintJoin = {}));
export var BlendMode;
(function (BlendMode) {
    BlendMode[BlendMode["kClear"] = Constants.BLENDMODE_CLEAR] = "kClear";
    BlendMode[BlendMode["kSrc"] = Constants.BLENDMODE_SRC] = "kSrc";
    BlendMode[BlendMode["kDst"] = Constants.BLENDMODE_DST] = "kDst";
    BlendMode[BlendMode["kDstOver"] = Constants.BLENDMODE_DST_OVER] = "kDstOver";
    BlendMode[BlendMode["kSrcOver"] = Constants.BLENDMODE_SRC_OVER] = "kSrcOver";
    BlendMode[BlendMode["kSrcIn"] = Constants.BLENDMODE_SRC_IN] = "kSrcIn";
    BlendMode[BlendMode["kDstIn"] = Constants.BLENDMODE_DST_IN] = "kDstIn";
    BlendMode[BlendMode["kSrcOut"] = Constants.BLENDMODE_SRC_OUT] = "kSrcOut";
    BlendMode[BlendMode["kDstOut"] = Constants.BLENDMODE_DST_OUT] = "kDstOut";
    BlendMode[BlendMode["kSrcATop"] = Constants.BLENDMODE_SRC_ATOP] = "kSrcATop";
    BlendMode[BlendMode["kDstATop"] = Constants.BLENDMODE_DST_ATOP] = "kDstATop";
    BlendMode[BlendMode["kXor"] = Constants.BLENDMODE_XOR] = "kXor";
    BlendMode[BlendMode["kPlus"] = Constants.BLENDMODE_PLUS] = "kPlus";
    BlendMode[BlendMode["kModulate"] = Constants.BLENDMODE_MODULATE] = "kModulate";
    BlendMode[BlendMode["kScreen"] = Constants.BLENDMODE_SCREEN] = "kScreen";
    BlendMode[BlendMode["kOverlay"] = Constants.BLENDMODE_OVERLAY] = "kOverlay";
    BlendMode[BlendMode["kLighten"] = Constants.BLENDMODE_LIGHTEN] = "kLighten";
    BlendMode[BlendMode["kColorDodge"] = Constants.BLENDMODE_COLOR_DODGE] = "kColorDodge";
    BlendMode[BlendMode["kColorBurn"] = Constants.BLENDMODE_COLOR_BURN] = "kColorBurn";
    BlendMode[BlendMode["kHardLight"] = Constants.BLENDMODE_HARD_LIGHT] = "kHardLight";
    BlendMode[BlendMode["kSoftLight"] = Constants.BLENDMODE_SOFT_LIGHT] = "kSoftLight";
    BlendMode[BlendMode["kDifference"] = Constants.BLENDMODE_DIFFERENCE] = "kDifference";
    BlendMode[BlendMode["kExclusion"] = Constants.BLENDMODE_EXCLUSION] = "kExclusion";
    BlendMode[BlendMode["kMultiply"] = Constants.BLENDMODE_MULTIPLY] = "kMultiply";
    BlendMode[BlendMode["kHue"] = Constants.BLENDMODE_HUE] = "kHue";
    BlendMode[BlendMode["kSaturation"] = Constants.BLENDMODE_SATURATION] = "kSaturation";
    BlendMode[BlendMode["kColor"] = Constants.BLENDMODE_COLOR] = "kColor";
    BlendMode[BlendMode["kLuminosity"] = Constants.BLENDMODE_LUMINOSITY] = "kLuminosity";
})(BlendMode || (BlendMode = {}));
export var TileMode;
(function (TileMode) {
    TileMode[TileMode["kClamp"] = Constants.TILEMODE_CLAMP] = "kClamp";
    TileMode[TileMode["kRepeat"] = Constants.TILEMODE_REPEAT] = "kRepeat";
    TileMode[TileMode["kMirror"] = Constants.TILEMODE_MIRROR] = "kMirror";
    TileMode[TileMode["kDecal"] = Constants.TILEMODE_DECAL] = "kDecal";
})(TileMode || (TileMode = {}));
export var RegionOp;
(function (RegionOp) {
    RegionOp[RegionOp["kDifference"] = Constants.REGIONOP_DIFFERENCE] = "kDifference";
    RegionOp[RegionOp["kIntersect"] = Constants.REGIONOP_INTERSECT] = "kIntersect";
    RegionOp[RegionOp["kUnion"] = Constants.REGIONOP_UNION] = "kUnion";
    RegionOp[RegionOp["kXor"] = Constants.REGIONOP_XOR] = "kXor";
    RegionOp[RegionOp["kReverseDifference"] = Constants.REGIONOP_REVERSE_DIFFERENCE] = "kReverseDifference";
    RegionOp[RegionOp["kReplace"] = Constants.REGIONOP_REPLACE] = "kReplace";
})(RegionOp || (RegionOp = {}));
export var ColorChannel;
(function (ColorChannel) {
    ColorChannel[ColorChannel["kA"] = Constants.COLORCHANNEL_A] = "kA";
    ColorChannel[ColorChannel["kR"] = Constants.COLORCHANNEL_G] = "kR";
    ColorChannel[ColorChannel["kG"] = Constants.COLORCHANNEL_B] = "kG";
    ColorChannel[ColorChannel["kB"] = Constants.COLORCHANNEL_B] = "kB";
})(ColorChannel || (ColorChannel = {}));
export var FilterMode;
(function (FilterMode) {
    FilterMode[FilterMode["kNearest"] = Constants.FILTERMODE_NEAREST] = "kNearest";
    FilterMode[FilterMode["kLinear"] = Constants.FILTERMODE_LINEAR] = "kLinear";
})(FilterMode || (FilterMode = {}));
export var MipmapMode;
(function (MipmapMode) {
    MipmapMode[MipmapMode["kNone"] = Constants.MIPMAPMODE_NONE] = "kNone";
    MipmapMode[MipmapMode["kNearest"] = Constants.MIPMAPMODE_NEAREST] = "kNearest";
    MipmapMode[MipmapMode["kLinear"] = Constants.MIPMAPMODE_LINEAR] = "kLinear";
})(MipmapMode || (MipmapMode = {}));
export var PathFillType;
(function (PathFillType) {
    PathFillType[PathFillType["kWinding"] = Constants.PATHFILLTYPE_WINDING] = "kWinding";
    PathFillType[PathFillType["kEvenOdd"] = Constants.PATHFILLTYPE_EVEN_ODD] = "kEvenOdd";
    PathFillType[PathFillType["kInverseWinding"] = Constants.PATHFILLTYPE_INVERSE_WINDING] = "kInverseWinding";
    PathFillType[PathFillType["kInverseEvenOdd"] = Constants.PATHFILLTYPE_INVERSE_EVEN_ODD] = "kInverseEvenOdd";
})(PathFillType || (PathFillType = {}));
export var ClipOp;
(function (ClipOp) {
    ClipOp[ClipOp["kDifference"] = Constants.CLIPOP_DIFFERENCE] = "kDifference";
    ClipOp[ClipOp["kIntersect"] = Constants.CLIPOP_INTERSECT] = "kIntersect";
})(ClipOp || (ClipOp = {}));
export var CanvasStatus;
(function (CanvasStatus) {
    CanvasStatus[CanvasStatus["kRecording"] = 0] = "kRecording";
    CanvasStatus[CanvasStatus["kFinished"] = 1] = "kFinished";
    CanvasStatus[CanvasStatus["kDisposed"] = 2] = "kDisposed";
})(CanvasStatus || (CanvasStatus = {}));
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
    constructor(ctx, width, height) {
        let writer = new ProtoBufferWriter(ctx);
        super(new ProtoCodeEmitter(writer));
        this.breakpointIdCounter = 0;
        this.writer = writer;
        this.status = CanvasStatus.kRecording;
        this.heapBinder = new Glamor.MoeHeapObjectBinder();
        this.breakpointsMap = new Map();
        this.getEmitter().emitDrawBounds(width, height);
    }
    checkStatus(expects) {
        if (this.status != expects) {
            throw new Error(`Canvas is in a wrong state`);
        }
    }
    annotateDebugPrint(content) {
        let contentOp = this.allocateLocalMemOpWithoutRef((op, e) => {
            this.heapBinder.bindString(op, content);
            return MemOpTypes.kString;
        });
        this.emitter.emitMarkBoundMemOpRequirement(contentOp);
        this.emitter.emitDebugPrint(contentOp);
        this.emitter.emitHeapFree(contentOp);
    }
    save() {
        this.emitter.emitSave();
    }
    restore() {
        this.emitter.emitRestore();
    }
    translate(dx, dy) {
        this.emitter.emitTranslate(dx, dy);
    }
    scale(dx, dy) {
        this.emitter.emitScale(dx, dy);
    }
    rotate(degrees, px, py) {
        if (px != undefined && py != undefined) {
            this.emitter.emitRotate2(degrees, px, py);
        }
        else {
            this.emitter.emitRotate(degrees);
        }
    }
    skew(sx, sy) {
        this.emitter.emitSkew(sx, sy);
    }
    concat(matrix) {
        this.checkAssemblerOwnership(matrix);
        this.emitter.emitConcat(matrix.getMemOperand());
    }
    resetMatrix() {
        this.emitter.emitResetMatrix();
    }
    setMatrix(matrix) {
        this.checkAssemblerOwnership(matrix);
        this.emitter.emitSetMatrix(matrix.getMemOperand());
    }
    clipRect(rect, op, doAntialias) {
        let mr = this.allocateLocalRect(rect);
        this.emitter.emitClipRect(mr, op, BoolNumber(doAntialias));
        this.emitter.emitHeapFree(mr);
    }
    clipRRect(rrect, op, doAntialias) {
        this.checkAssemblerOwnership(rrect);
        this.emitter.emitClipRRect(rrect.getMemOperand(), op, BoolNumber(doAntialias));
    }
    clipPath(path, op, doAntialias) {
        this.checkAssemblerOwnership(path);
        this.emitter.emitClipPath(path.getMemOperand(), op, BoolNumber(doAntialias));
    }
    // TODO: clipRegion
    drawColor(color, mode) {
        this.emitter.emitDrawColor(color, mode);
    }
    clear(color) {
        if (!Number.isInteger(color)) {
            throw new Error('Argument \'color\ should be an integer');
        }
        this.emitter.emitClear((color | 0) & 0xffffffff);
    }
    drawPaint(paint) {
        this.checkAssemblerOwnership(paint);
        this.emitter.emitDrawPaint(paint.getMemOperand());
    }
    drawLine(x0, y0, x1, y1, paint) {
        this.checkAssemblerOwnership(paint);
        this.emitter.emitDrawLine(x0, y0, x1, y1, paint.getMemOperand());
    }
    drawRect(rect, paint) {
        this.checkAssemblerOwnership(paint);
        let mr = this.allocateLocalRect(rect);
        this.emitter.emitDrawRect(mr, paint.getMemOperand());
        this.emitter.emitHeapFree(mr);
    }
    // TODO: drawRegion
    drawOval(oval, paint) {
        this.checkAssemblerOwnership(paint);
        let mr = this.allocateLocalRect(oval);
        this.emitter.emitDrawOval(mr, paint.getMemOperand());
        this.emitter.emitHeapFree(mr);
    }
    drawRRect(rrect, paint) {
        this.checkAssemblerOwnership(rrect);
        this.emitter.emitDrawRRect(rrect.getMemOperand(), paint.getMemOperand());
    }
    drawDRRect(outer, inner, paint) {
        this.checkAssemblerOwnership(outer);
        this.checkAssemblerOwnership(inner);
        this.checkAssemblerOwnership(paint);
        this.emitter.emitDrawDRRect(outer.getMemOperand(), inner.getMemOperand(), paint.getMemOperand());
    }
    drawCircle(cx, cy, radius, paint) {
        this.checkAssemblerOwnership(paint);
        this.emitter.emitDrawCircle(cx, cy, radius, paint.getMemOperand());
    }
    drawCirclev(c, radius, paint) {
        this.drawCircle(c.x, c.y, radius, paint);
    }
    drawArc(oval, startAngle, sweepAngle, useCenter, paint) {
        let mr = this.allocateLocalRect(oval);
        this.emitter.emitDrawArc(mr, startAngle, sweepAngle, BoolNumber(useCenter), paint.getMemOperand());
        this.emitter.emitHeapFree(mr);
    }
    drawPath(path, paint) {
        this.checkAssemblerOwnership(path);
        this.checkAssemblerOwnership(paint);
        this.emitter.emitDrawPath(path.getMemOperand(), paint.getMemOperand());
    }
    drawPicture(picture, matrix, paint) {
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
    drawImage(image, left, top, sampling, paint) {
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
    drawImageRect(image, dst, sampling, paint) {
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
    insertDebugBreakpoint(func) {
        this.breakpointIdCounter++;
        this.emitter.emitDebugBreakpoint(this.breakpointIdCounter);
        this.breakpointsMap.set(this.breakpointIdCounter, func);
    }
    insertProfilingBreakpoint(func) {
        this.breakpointIdCounter++;
        this.emitter.emitProfilingBreakpoint(this.breakpointIdCounter);
        this.breakpointsMap.set(this.breakpointIdCounter, func);
    }
    /**
     * Recording of drawing operations is finished and destroy used
     * resources. This method must be called before `submit` or `toString()`.
     * Note that the IR buffers will NOT be released util `dispose` is called.
     */
    finish() {
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
    dispose() {
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
    submit() {
        this.checkStatus(CanvasStatus.kFinished);
        let heapProfiling = _features.get(Features.CODEGEN_HEAP_PROFILING);
        let breakpointHandlers = {
            debugCallback: (id) => {
                // TODO: implement this.
            },
            profilingCallback: (id) => {
                // TODO: implement this.
            }
        };
        return Glamor.MoeTranslationToolchain.Interpreter(this.writer.getBuffers(), this.heapBinder, breakpointHandlers, heapProfiling);
    }
    /**
     * Get a string representation of generated IR code.
     *
     * @throws      Error if canvas is disposed or has not been finished.
     */
    toString() {
        this.checkStatus(CanvasStatus.kFinished);
        return Glamor.MoeTranslationToolchain.Disassemble(this.writer.getBuffers());
    }
}
export class Matrix4x4 extends HeapObjectWithAssemblerBase {
    constructor(canvas) {
        super(canvas, MemOpTypes.kMatrix4x4, (op, as) => {
            as.getEmitter().emitHeapCreateM44(op);
        });
    }
    setRows(r0, r1, r2, r3) {
        let mr0 = this.getAssembler().allocateLocalVector4(r0), mr1 = this.getAssembler().allocateLocalVector4(r1), mr2 = this.getAssembler().allocateLocalVector4(r2), mr3 = this.getAssembler().allocateLocalVector4(r3);
        this.getEmitter().emitM44SetRows(this.getMemOperand(), mr0, mr1, mr2, mr3);
        this.getEmitter().emitHeapFree(mr0);
        this.getEmitter().emitHeapFree(mr1);
        this.getEmitter().emitHeapFree(mr2);
        this.getEmitter().emitHeapFree(mr3);
    }
    setCols(c0, c1, c2, c3) {
        let mc0 = this.getAssembler().allocateLocalVector4(c0), mc1 = this.getAssembler().allocateLocalVector4(c1), mc2 = this.getAssembler().allocateLocalVector4(c2), mc3 = this.getAssembler().allocateLocalVector4(c3);
        this.getEmitter().emitM44SetCols(this.getMemOperand(), mc0, mc1, mc2, mc3);
        this.getEmitter().emitHeapFree(mc0);
        this.getEmitter().emitHeapFree(mc1);
        this.getEmitter().emitHeapFree(mc2);
        this.getEmitter().emitHeapFree(mc3);
    }
    setTranslate(x, y, z) {
        this.getEmitter().emitM44SetTranslate(this.getMemOperand(), x, y, z);
    }
    setScale(x, y, z) {
        this.getEmitter().emitM44SetScale(this.getMemOperand(), x, y, z);
    }
    setRectToRect(src, dst) {
        let mSrc = this.getAssembler().allocateLocalRect(src), mDst = this.getAssembler().allocateLocalRect(dst);
        this.getEmitter().emitM44SetRectToRect(this.getMemOperand(), mSrc, mDst);
        this.getEmitter().emitHeapFree(mSrc);
        this.getEmitter().emitHeapFree(mDst);
    }
    setLookAt(eye, center, up) {
        let mEye = this.getAssembler().allocateLocalVector3(eye), mCenter = this.getAssembler().allocateLocalVector3(center), mUp = this.getAssembler().allocateLocalVector3(up);
        this.getEmitter().emitM44SetLookAt(this.getMemOperand(), mEye, mCenter, mUp);
        this.getEmitter().emitHeapFree(mEye);
        this.getEmitter().emitHeapFree(mCenter);
        this.getEmitter().emitHeapFree(mUp);
    }
    setPerspective(near, far, angle) {
        this.getEmitter().emitM44SetPerspective(this.getMemOperand(), near, far, angle);
    }
    setIdentity() {
        this.getEmitter().emitM44SetIdentity(this.getMemOperand());
    }
    concat(other) {
        this.getAssembler().checkAssemblerOwnership(other);
        this.getEmitter().emitM44Concat(this.getMemOperand(), other.getMemOperand());
    }
}
export class SamplingOptions extends StatelessImmutableHeapObjectBase {
    constructor(factory) {
        super(MemOpTypes.kSamplingOptions, factory);
    }
    static Make(filterMode, mipmapMode) {
        return new SamplingOptions((op, as) => {
            as.getEmitter().emitHeapCreateSamplingOptions(op, filterMode, mipmapMode);
        });
    }
    static MakeCubicMitchell() {
        return new SamplingOptions((op, as) => {
            as.getEmitter().emitHeapCreateSamplingOptionsCubicMitchell(op);
        });
    }
    static MakeCubicCatmullRom() {
        return new SamplingOptions((op, as) => {
            as.getEmitter().emitHeapCreateSamplingOptionsCubicCatmullRom(op);
        });
    }
}
export class RRect extends HeapObjectWithAssemblerBase {
    constructor(canvas) {
        super(canvas, MemOpTypes.kRRect, (op, as) => {
            as.getEmitter().emitHeapCreateRRectEmpty(op);
        });
    }
    setOval(oval) {
        let mr = this.getAssembler().allocateLocalRect(oval);
        this.getEmitter().emitRRectSetOval(this.getMemOperand(), mr);
        this.getEmitter().emitHeapFree(mr);
    }
    setNinePatch(rect, leftRad, topRad, rightRad, bottomRad) {
        let mr = this.getAssembler().allocateLocalRect(rect);
        this.getEmitter().emitRRectSetNinePatch(this.getMemOperand(), mr, leftRad, topRad, rightRad, bottomRad);
        this.getEmitter().emitHeapFree(mr);
    }
    setRectXY(rect, xRad, yRad) {
        let mr = this.getAssembler().allocateLocalRect(rect);
        this.getEmitter().emitRRectSetRectXY(this.getMemOperand(), mr, xRad, yRad);
        this.getEmitter().emitHeapFree(mr);
    }
    inset(dx, dy) {
        this.getEmitter().emitRRectInset(this.getMemOperand(), dx, dy);
    }
    outset(dx, dy) {
        this.getEmitter().emitRRectOutset(this.getMemOperand(), dx, dy);
    }
    offset(dx, dy) {
        this.getEmitter().emitRRectOffset(this.getMemOperand(), dx, dy);
    }
}
export class Path extends HeapObjectWithAssemblerBase {
    constructor(canvas) {
        super(canvas, MemOpTypes.kPath, (op, assembler) => {
            assembler.getEmitter().emitHeapCreatePath(op);
        });
    }
    setPathFillType(type) {
        this.getEmitter().emitPathSetPathFillType(this.getMemOperand(), type);
    }
    toggleInverseFillType() {
        this.getEmitter().emitPathToggleInverseFillType(this.getMemOperand());
    }
    reset() {
        this.getEmitter().emitPathReset(this.getMemOperand());
    }
    moveTo(x, y) {
        this.getEmitter().emitPathMoveTo(this.getMemOperand(), x, y);
    }
    moveTov(vec) {
        this.getEmitter().emitPathMoveTo(this.getMemOperand(), vec.x, vec.y);
    }
    rMoveTo(x, y) {
        this.getEmitter().emitPathRMoveTo(this.getMemOperand(), x, y);
    }
    rMoveTov(vec) {
        this.getEmitter().emitPathRMoveTo(this.getMemOperand(), vec.x, vec.y);
    }
    lineTo(x, y) {
        this.getEmitter().emitPathLineTo(this.getMemOperand(), x, y);
    }
    lineTov(vec) {
        this.getEmitter().emitPathLineTo(this.getMemOperand(), vec.x, vec.y);
    }
    rLineTo(x, y) {
        this.getEmitter().emitPathRLineTo(this.getMemOperand(), x, y);
    }
    rLineTov(vec) {
        this.getEmitter().emitPathRLineTo(this.getMemOperand(), vec.x, vec.y);
    }
    quadTo(x1, y1, x2, y2) {
        this.getEmitter().emitPathQuadTo(this.getMemOperand(), x1, y1, x2, y2);
    }
    quadTov(v1, v2) {
        this.getEmitter().emitPathQuadTo(this.getMemOperand(), v1.x, v1.y, v2.x, v2.y);
    }
    rQuadTo(x1, y1, x2, y2) {
        this.getEmitter().emitPathRQuadTo(this.getMemOperand(), x1, y1, x2, y2);
    }
    rQuadTov(v1, v2) {
        this.getEmitter().emitPathRQuadTo(this.getMemOperand(), v1.x, v1.y, v2.x, v2.y);
    }
    conicTo(x1, y1, x2, y2, w) {
        this.getEmitter().emitPathConicTo(this.getMemOperand(), x1, y1, x2, y2, w);
    }
    conicTov(v1, v2, w) {
        this.getEmitter().emitPathConicTo(this.getMemOperand(), v1.x, v1.y, v2.x, v2.y, w);
    }
    rConicTo(x1, y1, x2, y2, w) {
        this.getEmitter().emitPathRConicTo(this.getMemOperand(), x1, y1, x2, y2, w);
    }
    rConicTov(v1, v2, w) {
        this.getEmitter().emitPathRConicTo(this.getMemOperand(), v1.x, v1.y, v2.x, v2.y, w);
    }
    cubicTo(x1, y1, x2, y2, x3, y3) {
        this.getEmitter().emitPathCubicTo(this.getMemOperand(), x1, y1, x2, y2, x3, y3);
    }
    cubicTov(v1, v2, v3) {
        this.getEmitter().emitPathCubicTo(this.getMemOperand(), v1.x, v1.y, v2.x, v2.y, v3.x, v3.y);
    }
    rCubicTo(x1, y1, x2, y2, x3, y3) {
        this.getEmitter().emitPathRCubicTo(this.getMemOperand(), x1, y1, x2, y2, x3, y3);
    }
    rCubicTov(v1, v2, v3) {
        this.getEmitter().emitPathRCubicTo(this.getMemOperand(), v1.x, v1.y, v2.x, v2.y, v3.x, v3.y);
    }
    rectArcTo(oval, startAngle, sweepAngle, forceMoveTo) {
        let ovalMemOp = this.getAssembler().allocateLocalRect(oval);
        this.getEmitter().emitPathRectArcTo(this.getMemOperand(), ovalMemOp, startAngle, sweepAngle, BoolNumber(forceMoveTo));
        this.getEmitter().emitHeapFree(ovalMemOp);
    }
    tangentArcTo(x1, y1, x2, y2, radius) {
        this.getEmitter().emitPathTangentArcTo(this.getMemOperand(), x1, y1, x2, y2, radius);
    }
    tangentArcTov(v1, v2, radius) {
        this.getEmitter().emitPathTangentArcTo(this.getMemOperand(), v1.x, v1.y, v2.x, v2.y, radius);
    }
    rotateArcTo(rx, ry, xAxisRotate, largeArc, ccwSweep, x, y) {
        this.getEmitter().emitPathRotateArcTo(this.getMemOperand(), rx, ry, xAxisRotate, BoolNumber(largeArc), BoolNumber(ccwSweep), x, y);
    }
    rotateArcTov(rv, xAxisRotate, largeArc, ccwSweep, pos) {
        this.getEmitter().emitPathRotateArcTo(this.getMemOperand(), rv.x, rv.y, xAxisRotate, BoolNumber(largeArc), BoolNumber(ccwSweep), pos.x, pos.y);
    }
    rRotateArcTo(rx, ry, xAxisRotate, largeArc, ccwSweep, x, y) {
        this.getEmitter().emitPathRRotateArcTo(this.getMemOperand(), rx, ry, xAxisRotate, BoolNumber(largeArc), BoolNumber(ccwSweep), x, y);
    }
    rRotateArcTov(rv, xAxisRotate, largeArc, ccwSweep, pos) {
        this.getEmitter().emitPathRRotateArcTo(this.getMemOperand(), rv.x, rv.y, xAxisRotate, BoolNumber(largeArc), BoolNumber(ccwSweep), pos.x, pos.y);
    }
    close() {
        this.getEmitter().emitPathClose(this.getMemOperand());
    }
    addRect(rect, ccw = false) {
        let rectMemOp = this.getAssembler().allocateLocalRect(rect);
        this.getEmitter().emitPathAddRect(this.getMemOperand(), rectMemOp, BoolNumber(ccw));
        this.getEmitter().emitHeapFree(rectMemOp);
    }
    addOval(oval, ccw = false) {
        let ovalMemOp = this.getAssembler().allocateLocalRect(oval);
        this.getEmitter().emitPathAddOval(this.getMemOperand(), ovalMemOp, BoolNumber(ccw));
        this.getEmitter().emitHeapFree(ovalMemOp);
    }
    addCircle(x, y, radius, ccw = false) {
        this.getEmitter().emitPathAddCircle(this.getMemOperand(), x, y, radius, BoolNumber(ccw));
    }
    addCirclev(v, radius, ccw = false) {
        this.getEmitter().emitPathAddCircle(this.getMemOperand(), v.x, v.y, radius, BoolNumber(ccw));
    }
    addArc(oval, startAngle, sweepAngle) {
        let ovalMemOp = this.getAssembler().allocateLocalRect(oval);
        this.getEmitter().emitPathAddArc(this.getMemOperand(), ovalMemOp, startAngle, sweepAngle);
        this.getEmitter().emitHeapFree(ovalMemOp);
    }
    addUniformRoundRect(rect, rx, ry, ccw = false) {
        let rectMemOp = this.getAssembler().allocateLocalRect(rect);
        this.getEmitter().emitPathAddUniformRoundRect(this.getMemOperand(), rectMemOp, rx, ry, BoolNumber(ccw));
        this.getEmitter().emitHeapFree(rectMemOp);
    }
    addUniformRoundRectv(rect, rv, ccw = false) {
        this.addUniformRoundRect(rect, rv.x, rv.y, ccw);
    }
    addRoundRect(rect, radii, ccw = false) {
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
    addPoly(vertXY, close) {
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
    addPolyv(vert, close) {
        let extractedArray = Array(vert.length * 2);
        for (let i = 0; i < vert.length; i++) {
            extractedArray[i * 2] = vert[i].x;
            extractedArray[i * 2 + 1] = vert[i].y;
        }
        this.addPoly(extractedArray, close);
    }
    transform(mat, dst, applyPerspectiveClip) {
        this.getAssembler().checkAssemblerOwnership(mat);
        this.getAssembler().checkAssemblerOwnership(dst);
        this.getEmitter().emitPathTransform(this.getMemOperand(), mat.getMemOperand(), dst.getMemOperand(), BoolNumber(applyPerspectiveClip));
    }
}
export class Paint extends HeapObjectWithAssemblerBase {
    constructor(canvas) {
        super(canvas, MemOpTypes.kPaint, (op, as) => {
            as.getEmitter().emitHeapCreatePaint(op);
        });
    }
    reset() {
        this.getEmitter().emitPaintReset(this.getMemOperand());
    }
    setAntialias(aa) {
        this.getEmitter().emitPaintSetAntialias(this.getMemOperand(), BoolNumber(aa));
    }
    setDither(dither) {
        this.getEmitter().emitPaintSetDither(this.getMemOperand(), BoolNumber(dither));
    }
    setStyleStroke(stroke) {
        this.getEmitter().emitPaintSetStyleStroke(this.getMemOperand(), BoolNumber(stroke));
    }
    setColor(color) {
        if (!Number.isInteger(color)) {
            throw new Error('Argument \'color\' must be an integer');
        }
        this.getEmitter().emitPaintSetColor(this.getMemOperand(), (color | 0) & 0xffffffff);
    }
    setAlphaf(alpha) {
        alpha = Math.min(Math.max(0, alpha));
        this.getEmitter().emitPaintSetAlphaf(this.getMemOperand(), alpha);
    }
    setStrokeWidth(width) {
        this.getEmitter().emitPaintSetStrokeWidth(this.getMemOperand(), width);
    }
    setStrokeMiter(miter) {
        this.getEmitter().emitPaintSetStrokeMiter(this.getMemOperand(), miter);
    }
    setStrokeCap(cap) {
        this.getEmitter().emitPaintSetStrokeCap(this.getMemOperand(), cap);
    }
    setStrokeJoin(join) {
        this.getEmitter().emitPaintSetStrokeJoin(this.getMemOperand(), join);
    }
    setBlendMode(mode) {
        this.getEmitter().emitPaintSetBlendMode(this.getMemOperand(), mode);
    }
    setShader(shader) {
        shader.associateAssembler(this.assembler);
        this.getEmitter().emitPaintSetShader(this.getMemOperand(), shader.getMemOperand());
    }
    setBlender(blender) {
        blender.associateAssembler(this.assembler);
        this.getEmitter().emitPaintSetBlender(this.getMemOperand(), blender.getMemOperand());
    }
    setColorFilter(filter) {
        filter.associateAssembler(this.assembler);
        this.getEmitter().emitPaintSetColorFilter(this.getMemOperand(), filter.getMemOperand());
    }
    setImageFilter(filter) {
        filter.associateAssembler(this.assembler);
        this.getEmitter().emitPaintSetImageFilter(this.getMemOperand(), filter.getMemOperand());
    }
    setPathEffect(effect) {
        effect.associateAssembler(this.assembler);
        this.getEmitter().emitPaintSetPathEffect(this.getMemOperand(), effect.getMemOperand());
    }
    setMaskFilter(filter) {
        filter.associateAssembler(this.assembler);
        this.getEmitter().emitPaintSetMaskFilter(this.getMemOperand(), filter.getMemOperand());
    }
}
export class Shader extends StatelessImmutableHeapObjectBase {
    constructor(creator) {
        super(MemOpTypes.kShader, (op, as) => {
            creator(op, as);
        });
    }
    static MakeEmpty() {
        return new Shader((op, as) => {
            return as.getEmitter().emitHeapCreateEmptyShader(op);
        });
    }
    static MakeColor(color) {
        return new Shader((op, as) => {
            return as.getEmitter().emitHeapCreateColorShader(op, color);
        });
    }
    static MakeBlend(blender, dst, src) {
        return new Shader((op, as) => {
            blender.associateAssembler(as);
            dst.associateAssembler(as);
            src.associateAssembler(as);
            return as.getEmitter().emitHeapCreateBlendShader(op, blender.getMemOperand(), dst.getMemOperand(), src.getMemOperand());
        });
    }
}
export class Blender extends StatelessImmutableHeapObjectBase {
}
export class ColorFilter extends StatelessImmutableHeapObjectBase {
}
export class ImageFilter extends StatelessImmutableHeapObjectBase {
    constructor(factory) {
        super(MemOpTypes.kImageFilter, factory);
    }
    static MakeBlur(sigmaX, sigmaY, tileMode, input, crop) {
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
