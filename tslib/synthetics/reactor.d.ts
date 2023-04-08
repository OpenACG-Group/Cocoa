export class Value {
}

export class BasicBlock {
    newByte(constant?: number): Value;
    newSByte(constant?: number): Value;
    newShort(constant?: number): Value;
    newUShort(constant?: number): Value;
    newInt(constant?: number): Value;
    newUInt(constant?: number): Value;
    newLong(constant?: number): Value;
    newULong(constant?: number): Value;
    newFloat(constant?: number): Value;

    newByte2(): Value;
    newByte2(x: number, y: number): Value;
    newByte4(): Value;
    newByte4(x: number, y: number, z: number, w: number): Value;
    newSByte2(): Value;
    newSByte2(x: number, y: number): Value;
    newSByte4(): Value;
    newSByte4(x: number, y: number, z: number, w: number): Value;
    newShort2(): Value;
    newShort2(x: number, y: number): Value;
    newShort4(): Value;
    newShort4(x: number, y: number, z: number, w: number): Value;
    newUShort2(): Value;
    newUShort2(x: number, y: number): Value;
    newUShort4(): Value;
    newUShort4(x: number, y: number, z: number, w: number): Value;
    newInt2(): Value;
    newInt2(x: number, y: number): Value;
    newInt4(): Value;
    newInt4(x: number, y: number, z: number, w: number): Value;
    newUInt2(): Value;
    newUInt2(x: number, y: number): Value;
    newUInt4(): Value;
    newUInt4(x: number, y: number, z: number, w: number): Value;
    newLong2(): Value;
    newLong2(x: number, y: number): Value;
    newLong4(): Value;
    newLong4(x: number, y: number, z: number, w: number): Value;
    newULong2(): Value;
    newULong2(x: number, y: number): Value;
    newULong4(): Value;
    newULong4(x: number, y: number, z: number, w: number): Value;
    newFloat2(): Value;
    newFloat2(x: number, y: number): Value;
    newFloat4(): Value;
    newFloat4(x: number, y: number, z: number, w: number): Value;

    createIf(cond: Value): BranchInst;

    createReturnVoid(): void;
    createReturn(value: Value): void;
}

export type CodeGenFunc = (codegen: BasicBlock) => void;

export class BranchInst {
    createThen(codegen: CodeGenFunc): BranchInst;
    createElse(codegen: CodeGenFunc): BranchInst;
}

export class GShaderBuilder {
    constructor(name: string, mainSignature: string);

    userMainEntrypoint(codegen: CodeGenFunc): void;
}
