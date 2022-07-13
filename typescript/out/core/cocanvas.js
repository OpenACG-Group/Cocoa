/**
 * This module generates VGIR (Vector Graphics Intermediate Representation) bytecode
 * and submit it to native rendering engine, who executes the submitted bytecode and
 * generates drawing instructions. Those instructions will be performed by CPU or GPU
 * asynchronously depending on whether the hardware-acceleration is available.
 */
import * as std from 'core';
import * as render from 'cobalt';
import { LinkedList } from './linked_list';
import { Opcode, Constants, ProtoCodeEmitter } from "./generated/cocanvas_code_emitter";
const PROTO_BUFFER_UNIT_BUFFER_SIZE = 512;
const PROTO_BUFFER_MAX_UNUSED_ALLOCATION_CYCLES = 8;
const PROTO_OPCODE_BYTE_SIZE = 2;
let _features = new Map([
    ['ir-heap-profiling', false],
    ['ir-optimization-heapleak-elimination', false]
]);
export function setCanvasFeature(name, value) {
    if (!_features.has(name))
        throw Error('Invalid canvas feature \'' + name + '\'');
    _features.set(name, value);
}
class ProtoBufferCell {
    constructor(buffer, unusedAllocationCycles = 0) {
        this.buffer = buffer;
        this.unusedAllocationCycles = unusedAllocationCycles;
    }
    static EqualComparator(a, b) {
        return (a.buffer == b.buffer);
    }
}
class ProtoBufferPool {
    constructor() {
        this.freeBufferList = new LinkedList(ProtoBufferCell.EqualComparator);
        this.acquiredBufferList = new LinkedList(ProtoBufferCell.EqualComparator);
    }
    cleanBuffersReachedMaxUnusedAllocationCycle() {
        this.freeBufferList.removeIf((value) => {
            if (value.unusedAllocationCycles >= PROTO_BUFFER_MAX_UNUSED_ALLOCATION_CYCLES) {
                value.buffer = null;
                return true;
            }
            return false;
        });
    }
    acquireNewBuffer(size) {
        let bestFitCell = {
            sizeDiff: size,
            cell: null
        };
        for (let cell of this.freeBufferList) {
            let diff = cell.buffer.length - size;
            if (diff >= 0 && diff < bestFitCell.sizeDiff) {
                bestFitCell.sizeDiff = diff;
                bestFitCell.cell = cell;
            }
            cell.unusedAllocationCycles++;
        }
        let cell = bestFitCell.cell;
        if (cell == null) {
            cell = new ProtoBufferCell(new std.Buffer(size));
        }
        else {
            cell.unusedAllocationCycles = 0;
            this.freeBufferList.remove(cell);
        }
        this.acquiredBufferList.push(cell);
        return cell.buffer;
    }
    releaseAcquiredBuffer(buffer) {
        let foundCell = null;
        for (let cell of this.acquiredBufferList) {
            if (cell.buffer == buffer) {
                foundCell = cell;
                break;
            }
        }
        if (foundCell == null) {
            throw Error('Buffer is not acquired or not managed by current pool');
        }
        this.acquiredBufferList.remove(foundCell);
        this.freeBufferList.push(foundCell);
        this.cleanBuffersReachedMaxUnusedAllocationCycle();
    }
}
class ProtoBufferWriter {
    constructor(context) {
        this.context = context;
        let buf = context.getBufferPool().acquireNewBuffer(PROTO_BUFFER_UNIT_BUFFER_SIZE);
        this.buffersVector = [buf];
        this.currentBuffer = buf;
        this.currentBufferPos = 0;
        this.dataView = this.currentBuffer.toDataView();
    }
    dispose() {
        this.currentBuffer = null;
        this.currentBufferPos = 0;
        for (let buffer of this.buffersVector) {
            this.context.getBufferPool().releaseAcquiredBuffer(buffer);
        }
        this.buffersVector = [];
        this.dataView = null;
    }
    getBuffers() {
        return this.buffersVector;
    }
    switchToNextBuffer() {
        let buf = this.context.getBufferPool()
            .acquireNewBuffer(PROTO_BUFFER_UNIT_BUFFER_SIZE);
        this.buffersVector.push(buf);
        this.currentBuffer = buf;
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
        this.bufferPool = new ProtoBufferPool();
    }
    getBufferPool() {
        return this.bufferPool;
    }
}
export class Canvas {
    constructor(ctx, width, height) {
        this.hasFinished = false;
        this.finishedBuffersVector = null;
        this.writer = new ProtoBufferWriter(ctx);
        this.emitter = new ProtoCodeEmitter(this.writer);
        this.emitter.emitDrawBounds(width, height);
    }
    finish() {
        this.emitter.emitCommandPoolEnd();
        this.hasFinished = true;
        this.finishedBuffersVector = this.writer.getBuffers();
        this.writer.dispose();
        this.writer = null;
    }
    submit() {
        if (!this.hasFinished)
            throw Error('Canvas must be finished by finish() before submitting or disassembling');
        return render.GskIRCompiler.Compile(this.finishedBuffersVector, null, _features.get('ir-heap-profiling'));
    }
    disassemble() {
        if (!this.hasFinished)
            throw Error('Canvas must be finished by finish() before submitting or disassembling');
        return render.GskIRCompiler.Disassemble(this.finishedBuffersVector);
    }
    test(x, y, z) {
        // this.emitter.emitDrawImage(1, 0, 0, 0, 0);
        let R = 60.0, C = 128.0;
        this.emitter.emitHeapCreatePath(7);
        this.emitter.emitPathMoveTo(7, C + R, C);
        for (let i = 1; i < 15; i++) {
            let a = 0.44879895 * i;
            let r = R + R * (i % 2);
            this.emitter.emitPathLineTo(7, C + r * Math.cos(a), C + r * Math.sin(a));
        }
        this.emitter.emitHeapCreateVector2(2, 0, 0);
        this.emitter.emitHeapCreateVector2(3, 256, 256);
        this.emitter.emitHeapCreateU32Array(4, 2);
        this.emitter.emitHeapU32ArrayStore(4, 0, 0xff4285f4);
        this.emitter.emitHeapU32ArrayStore(4, 1, 0xff0f9d58);
        this.emitter.emitHeapCreateLinearGradientShader(5, 2, 3, 4, 0, Constants.TILEMODE_CLAMP);
        this.emitter.emitHeapCreatePaint(6);
        this.emitter.emitPaintSetShader(6, 5);
        this.emitter.emitPaintSetAntialias(6, 1);
        this.emitter.emitClear(0xffffffff);
        this.emitter.emitHeapCreateM44(8);
        this.emitter.emitM44SetTranslate(8, x, y, z);
        this.emitter.emitSetMatrix(8);
        this.emitter.emitDrawPath(7, 6);
    }
}
