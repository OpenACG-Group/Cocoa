/**
 * This module generates VGIR (Vector Graphics Intermediate Representation) bytecode
 * and submit it to native rendering engine, who executes the submitted bytecode and
 * generates drawing instructions. Those instructions will be performed by CPU or GPU
 * asynchronously depending on whether the hardware-acceleration is available.
 */

import * as std from 'core';
import * as render from 'cobalt';
import { LinkedList } from './linked_list';
import { IProtoBufferWriter } from "./cocanvas_iface";
import { Opcode, Constants, ProtoCodeEmitter } from "./generated/cocanvas_code_emitter";

const PROTO_BUFFER_UNIT_BUFFER_SIZE = 512;
const PROTO_BUFFER_MAX_UNUSED_ALLOCATION_CYCLES = 8;

const PROTO_OPCODE_BYTE_SIZE = 2;

class ProtoBufferCell {
    constructor(public buffer: std.Buffer, public unusedAllocationCycles: number = 0) {
    }

    static EqualComparator(a: ProtoBufferCell, b: ProtoBufferCell): boolean {
        return (a.buffer == b.buffer);
    }
}

class ProtoBufferPool {
    private readonly freeBufferList: LinkedList<ProtoBufferCell>;
    private readonly acquiredBufferList: LinkedList<ProtoBufferCell>;

    constructor() {
        this.freeBufferList = new LinkedList<ProtoBufferCell>(ProtoBufferCell.EqualComparator);
        this.acquiredBufferList = new LinkedList<ProtoBufferCell>(ProtoBufferCell.EqualComparator);
    }

    public cleanBuffersReachedMaxUnusedAllocationCycle(): void {
        this.freeBufferList.removeIf((value: ProtoBufferCell): boolean => {
            if (value.unusedAllocationCycles >= PROTO_BUFFER_MAX_UNUSED_ALLOCATION_CYCLES) {
                value.buffer = null;
                return true;
            }
            return false;
        });
    }

    public acquireNewBuffer(size: number): std.Buffer {
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

        let cell: ProtoBufferCell = bestFitCell.cell;
        if (cell == null) {
            cell = new ProtoBufferCell(new std.Buffer(size));
        } else {
            cell.unusedAllocationCycles = 0;
            this.freeBufferList.remove(cell);
        }

        this.acquiredBufferList.push(cell);
        return cell.buffer;
    }

    public releaseAcquiredBuffer(buffer: std.Buffer): void {
        let foundCell: ProtoBufferCell = null;
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

class ProtoBufferWriter implements IProtoBufferWriter {
    private context: DrawingContext;
    private buffersVector: std.Buffer[];
    private currentBuffer: std.Buffer;
    private currentBufferPos: number;
    private dataView: DataView;

    constructor(context: DrawingContext) {
        this.context = context;
        let buf = context.getBufferPool().acquireNewBuffer(PROTO_BUFFER_UNIT_BUFFER_SIZE);

        this.buffersVector = [buf];
        this.currentBuffer = buf;
        this.currentBufferPos = 0;
        this.dataView = this.currentBuffer.toDataView();
    }

    public dispose(): void {
        this.currentBuffer = null;
        this.currentBufferPos = 0;
        for (let buffer of this.buffersVector) {
            this.context.getBufferPool().releaseAcquiredBuffer(buffer);
        }
        this.buffersVector = [];
        this.dataView = null;
    }

    public getBuffers(): std.Buffer[] {
        return this.buffersVector;
    }

    private switchToNextBuffer(): void {
        let buf = this.context.getBufferPool()
                  .acquireNewBuffer(PROTO_BUFFER_UNIT_BUFFER_SIZE);
        this.buffersVector.push(buf);
        this.currentBuffer = buf;
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

export class DrawingContext {
    private readonly bufferPool: ProtoBufferPool;

    constructor() {
        this.bufferPool = new ProtoBufferPool();
    }

    public getBufferPool(): ProtoBufferPool {
        return this.bufferPool;
    }
}

export class Canvas {
    private writer: ProtoBufferWriter;
    private emitter: ProtoCodeEmitter;
    private hasFinished: boolean = false;
    private finishedBuffersVector: std.Buffer[] = null;

    constructor(ctx: DrawingContext, width: number, height: number) {
        this.writer = new ProtoBufferWriter(ctx);
        this.emitter = new ProtoCodeEmitter(this.writer);
        this.emitter.emitDrawBounds(width, height);
    }

    public finish(): void {
        this.emitter.emitCommandPoolEnd();
        this.hasFinished = true;
        this.finishedBuffersVector = this.writer.getBuffers();
        this.writer.dispose();
        this.writer = null;
    }

    public submit(): void {
        if (!this.hasFinished)
            throw Error('Canvas must be finished by finish() before submitting or disassembling');
        return render.VRIRCompiler.Compile(this.finishedBuffersVector);
    }

    public disassemble(): string {
        if (!this.hasFinished)
            throw Error('Canvas must be finished by finish() before submitting or disassembling');
        return render.VRIRCompiler.Disassemble(this.finishedBuffersVector);
    }

    public test(): void {
        this.emitter.emitHeapCreateVector2(1, 10, 10);
        this.emitter.emitHeapCreateVector2(2, 100, 100);
        this.emitter.emitHeapCreateU32Array(3, 4);
        this.emitter.emitHeapU32ArrayStore(3, 0, 0x0066cc00);
        this.emitter.emitHeapU32ArrayStore(3, 1, 0x0066cc80);
        this.emitter.emitHeapU32ArrayStore(3, 2, 0x0066cc70);
        this.emitter.emitHeapU32ArrayStore(3, 3, 0x0066ccff);
        this.emitter.emitHeapCreateLinearGradientShader(4, 1, 2, 3, Constants.TILEMODE_CLAMP);
        this.emitter.emitHeapFree(1);
        this.emitter.emitHeapFree(2);
        this.emitter.emitHeapFree(3);
        this.emitter.emitHeapCreatePaint(5);
        this.emitter.emitPaintSetShader(5, 4);
    }
}
