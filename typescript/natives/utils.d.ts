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
export declare enum TextCodec {
    Latin1,
    UTF8,
    UCS2,
    Hex,
}
export interface EncodeTextIntoResult {
    readChars: u32;
    writtenBytes: u32;
}
export function computeTextByteSize(text: string, codec: TextCodec): u64;
export function encodeText(text: string, codec: TextCodec): Uint8Array;
export function encodeTextInto(text: string, codec: TextCodec, dst: Uint8Array): EncodeTextIntoResult;
export function decodeText(buffer: Uint8Array, codec: TextCodec): string;
