/* Native Interface [IO, Stream] */

const __IO_FLAG_READ     = 0x01;
const __IO_FLAG_WRITE    = 0x02;
const __IO_FLAG_CREATE   = 0x04;
const __IO_FLAG_TRUNC    = 0x08;
const __IO_MODE_IRUSR    = 0x001;
const __IO_MODE_IWUSR    = 0x002;
const __IO_MODE_IXUSR    = 0x004;
const __IO_MODE_IRGRP    = 0x008;
const __IO_MODE_IWGRP    = 0x010;
const __IO_MODE_IXGRP    = 0x020;
const __IO_MODE_IROTH    = 0x040;
const __IO_MODE_IWOTH    = 0x080;
const __IO_MODE_IXOTH    = 0x100;

const __IO_SEEK_CUR      = 0;
const __IO_SEEK_END      = 1;
const __IO_SEEK_BEGIN    = 2;

declare class __native_stream
{
    public isReadonly: boolean;

    public read(buffer: Uint8Array, size: number): number;
    public write(buffer: Uint8Array, size: number): number;
    public seekp(bytes: number, pos: number): void;
    public tellp(): number;
    public seekg(bytes: number, pos: number): void;
    public tellg(): number;
    public close(): void;

    // Experimental methods:
    public fcntl(): void;
    public ioctl(): void;
}
/**
 * @param file      A file to open.
 * @param flags     [bitfield] __IO_FLAG_*
 * @param mode      [bitfield] __IO_MODE_*
 * @return __native_stream
 *                  An object refers to the file.
 */
declare function __native_stream_open(file: string, flags: number, mode?: number): __native_stream;
declare function __native_stream_stdio(): __native_stream;
declare function __native_stream_stderr(): __native_stream;

/* User interface */

export class Stream
{
    public static IO_W = __IO_FLAG_WRITE;
    public static IO_R = __IO_FLAG_READ;
    public static IO_CREATE = __IO_FLAG_CREATE;
    public static IO_TRUNC = __IO_FLAG_TRUNC;
    public static MODE_IRUSR = __IO_MODE_IRUSR;
    public static MODE_IWUSR = __IO_MODE_IWUSR;
    public static MODE_IXUSR = __IO_MODE_IXUSR;
    public static MODE_IRGRP = __IO_MODE_IRGRP;
    public static MODE_IWGRP = __IO_MODE_IWGRP;
    public static MODE_IXGRP = __IO_MODE_IXGRP;
    public static MODE_IROTH = __IO_MODE_IROTH;
    public static MODE_IWOTH = __IO_MODE_IWOTH;
    public static MODE_IXOTH = __IO_MODE_IXOTH;
    public static SEEK_CUR   = __IO_SEEK_CUR;
    public static SEEK_END   = __IO_SEEK_END;
    public static SEEK_BEGIN = __IO_SEEK_BEGIN;

    private __native__: __native_stream;
    public readable: boolean;
    public writable: boolean;

    public static Open(file: string, flags: number = Stream.IO_R | Stream.IO_W,
                       mode: number = Stream.MODE_IRUSR | Stream.MODE_IWUSR |
                                      Stream.MODE_IRGRP | Stream.MODE_IWGRP |
                                      Stream.MODE_IROTH): Stream
    {
        let result = new Stream;
        result.__native__ = __native_stream_open(file, flags, mode);
        result.readable = (flags & Stream.IO_R) == Stream.IO_R;
        result.writable = (flags & Stream.IO_W) == Stream.IO_W;
        return result;
    }

    public read(buffer: Uint8Array, size: number): number
    {
        if (!this.readable)
            throw Error("Stream is unreadable");

        if (buffer.byteLength < size)
            throw Error("Buffer is too small");
        if (size == 0)
            return 0;
        return this.__native__.read(buffer, size);
    }

    public write(buffer: Uint8Array, size: number): number
    {
        if (!this.writable)
            throw Error("Stream is not writable");

        let realSize = size;
        if (buffer.byteLength < size)
            realSize = buffer.byteLength;
        return this.__native__.write(buffer, realSize);
    }
}
