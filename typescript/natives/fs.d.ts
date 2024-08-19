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
export declare enum Mode {
    S_IRWXU,
    S_IRUSR,
    S_IWUSR,
    S_IXUSR,
    S_IRWXG,
    S_IRGRP,
    S_IWGRP,
    S_IXGRP,
    S_IRWXO,
    S_IROTH,
    S_IWOTH,
    S_IXOTH,
    S_ISUID,
    S_ISGID,
    S_ISVTX,
}
export interface ReadFileResult {
    buffer: Uint8Array;
    readSize: u64;
}
export function ReadFile(path: string, offset: i64, dst: (null | Uint8Array)): ReadFileResult;
export function WriteFile(path: string, content: Uint8Array, mode: u32): void;
export function realpath(path: string): string;
