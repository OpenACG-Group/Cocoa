export interface IProtoBufferWriter {
    performPossibleBufferSwitching(requireSize: number): void;
    writeInt8Unsafe(x: number): void;
    writeUint8Unsafe(x: number): void;
    writeInt16Unsafe(x: number): void;
    writeUint16Unsafe(x: number): void;
    writeInt32Unsafe(x: number): void;
    writeUint32Unsafe(x: number): void;
    writeFloat32Unsafe(x: number): void;
    writeFloat64Unsafe(x: number): void;
    writeInt64Unsafe(x: bigint): void;
    writeUint64Unsafe(x: bigint): void;
}
