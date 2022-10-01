type SizeT = number;
type OffsetT = number;

// =======================================================
// Basic Console IO and process API
// =======================================================

/**
 * An argument list specified by `--pass` and `--pass-delimiter`.
 * Stays empty if not specify explicitly (Never be null or undefined).
 */
export const args: string[];

/**
 * Prints `str` to standard output (without '\n').
 * @param str String content to print.
 */
export function print(str: string): void;

export function getEnviron(): Map<string, string>;

export function dumpNativeHeapProfile(): void;

export class Stream implements AsyncIterable<any> {
    readonly readable: boolean;
    readonly writable: boolean;

    [Symbol.asyncIterator](): AsyncIterator<any>;
}

export interface ProcessExitStatus {
    status: number;
    signal: number;
}

export interface ProcessOptions {
    file: string;
    args?: string[];
    env?: string[];
    cwd?: string;
    uid?: number;
    gid?: number;
    redirectStreams?: number;
}

export class Process {
    static readonly STREAM_STDIN: number;
    static readonly STREAM_STDOUT: number;
    static readonly STREAM_STDERR: number;
    static readonly SIGINT: number;
    static readonly SIGILL: number;
    static readonly SIGABRT: number;
    static readonly SIGFPE: number;
    static readonly SIGSEGV: number;
    static readonly SIGTERM: number;
    static readonly SIGHUP: number;
    static readonly SIGQUIT: number;
    static readonly SIGTRAP: number;
    static readonly SIGKILL: number;
    static readonly SIGPIPE: number;
    static readonly SIGALRM: number;
    static readonly SIGSTKFLT: number;
    static readonly SIGPWR: number;
    static readonly SIGBUS: number;
    static readonly SIGSYS: number;
    static readonly SIGURG: number;
    static readonly SIGSTOP: number;
    static readonly SIGTSTP: number;
    static readonly SIGCONT: number;
    static readonly SIGCHLD: number;
    static readonly SIGTTIN: number;
    static readonly SIGTTOU: number;
    static readonly SIGPOLL: number;
    static readonly SIGXFSZ: number;
    static readonly SIGXCPU: number;
    static readonly SIGVTALRM: number;
    static readonly SIGPROF: number;
    static readonly SIGUSR1: number;
    static readonly SIGUSR2: number;

    /**
     * Create a new child process and execute an executable file.
     * This functions returns when a child process has been created.
     *
     * @param options   Specify options. See `ProcessOptions` interface for more details.
     * @return          A `Process` object by which you can manipulate the child process.
     */
    static Fork(options: ProcessOptions): Process;

    readonly pid: number;

    promiseOnExit(): Promise<ProcessExitStatus>;
    kill(signum: number): void;
}

// =======================================================
// Property Tree manipulation API
// =======================================================

type PropertyProtection = number;
type PropertyType = number;

export class Property {
    static readonly PROT_PUBLIC: PropertyProtection;
    static readonly PROT_PRIVATE: PropertyProtection;
    static readonly TYPE_OBJECT: PropertyType;
    static readonly TYPE_ARRAY: PropertyType;
    static readonly TYPE_DATA: PropertyType;

    readonly type: PropertyType;

    /**
     * The parent node of this node. May be `null` for orphan node.
     */
    readonly parent: Property;

    /**
     * `protection` controls what operations on the node is allowed.
     * If `PROT_PUBLIC`, all the operations are available.
     * If `PROT_PRIVATE`, this node only can be accessed, but changing it is disallowed.
     * Exception will be thrown if you invoke disallowed operations.
     *
     * All the nodes created by `insertChild` or `pushbackChild` are `PROT_PUBLIC`,
     * and some nodes created by Cocoa natively is `PROT_PRIVATE`.
     */
    readonly protection: PropertyProtection;

    readonly numberOfChildren: number;

    name: string;

    /**
     * Access each child node and `func` callback will be called to receive
     * a property node.
     *
     * @param func  Callback function called for each child node.
     */
    foreachChild(func: (child: Property) => void): void;

    /**
     * Find and return a child node named `name`.
     * This method is only available when `type == TYPE_OBJECT`.
     *
     * @param name  Node name to find.
     * @return      (nullable) Found node, null if not found.
     * @throws      TypeError if `type != TYPE_OBJECT`
     */
    findChild(name: string): Property;

    /**
     * Construct and insert a child node typed `type`.
     * This method is only available when `type == TYPE_OBJECT`.
     *
     * @param type  Type of child node.
     * @param name  Name of child node.
     * @return      Constructed child node.
     * @throws      Error if `name` is invalid or conflicts with an existing node.
     *              Error if `protection == PROT_PRIVATE`.
     *              TypeError if `type != TYPE_OBJECT`.
     */
    insertChild(type: PropertyType, name: string): Property;

    /**
     * Construct and pushback a child node typed `type`.
     * This method is only available when `type == TYPE_ARRAY`.
     *
     * @param type  Type of child node.
     * @return      Constructed child node.
     * @throws      TypeError if `type != TYPE_ARRAY`.
     *              Error if `protection == PROT_PRIVATE`.
     */
    pushbackChild(type: PropertyType): Property;

    /**
     * Detach this node from its parent, which means that this removes the node itself
     * from parent node. Neither this node nor its children will be destructed, and all
     * the datas it holds are still available. This node will become an orphan node
     * after this method returns.
     *
     * @throws      Error if the node is an orphan (has no parent).
     *              Error if `protection == PROT_PRIVATE`.
     */
    detachFromParent(): void;

    /**
     * Extract typed data this node holds.
     * This is only available when `type == TYPE_DATA`.
     *
     * Data node actually holds a typed C++ native value or empty. Only can be extracted
     * primitive C++ types like numbers, booleans and strings (const char*). The only
     * exception is STL type `std::string`, which is treated as a primitive type of
     * string.
     *
     * @return (nullable) The extracted value, null if the value cannot be interpreted
     *                    as a JavaScript primitive type.
     */
    extract(): any;

    /**
     * Check whether a data node contains value.
     * This is only available when `type == TYPE_DATA`.
     *
     * @return `true` if a data node contains value, otherwise false.
     */
    hasData(): boolean;

    /**
     * Reset the value of this data node to `value`. If `value` is not provided,
     * the data node will become empty.
     * This is only available when `type == TYPE_DATA`.
     *
     * @param value     New value, should not be `null` or `undefined`.
     * @throws          Error if `protection == PROT_PRIVATE`.
     *                  TypeError if invalid argument.
     *                  TypeError if `value` cannot be interpreted as a native primitive
     *                  value.
     */
    resetData(value?: any): void;

    /**
     * Get a string which describes the underlying type of data node.
     * This is only available when `type == TYPE_DATA`.
     *
     * @return UNSTABLE API.
     */
    dataTypeinfo(): string;
}
export const property: Property;


// =======================================================
// Asynchronous filesystem IO API (POSIX)
// =======================================================

export interface IOError extends Error {
    syscall: string;    /* syscall name or the POSIX interface name */
    errno: number;      /* error number */
}

export interface Stat {
    dev: number;        /* ID of device containing file */
    mode: number;       /* File type and mode */
    nlink: number;      /* Number of hard links */
    uid: number;        /* User ID of owner */
    gid: number;        /* Group ID of owner */
    rdev: number;       /* Device ID (if special file) */
    blksize: number;    /* Block size for filesystem I/O */
    ino: number;        /* Inode number */
    size: number;       /* Total size, in bytes */
    blocks: number;     /* Number of 512B blocks allocated */
    atimeMs: number;    /* Time of last access in milliseconds */
    mtimeMs: number;    /* Time of last modification in milliseconds */
    ctimeMs: number;    /* Time of last status change in milliseconds */
    atime: Date;        /* Time of last access */
    mtime: Date;        /* Time of last modification */
    ctime: Date;        /* Time of last status change */
}

export class File {
    static readonly O_FILEMAP: number;
    static readonly O_RANDOM: number;
    static readonly O_SHORT_LIVED: number;
    static readonly O_SEQUENTIAL: number;
    static readonly O_TEMPORARY: number;
    static readonly O_APPEND: number;
    static readonly O_CREAT: number;
    static readonly O_DIRECT: number;
    static readonly O_DIRECTORY: number;
    static readonly O_DSYNC: number;
    static readonly O_EXCL: number;
    static readonly O_EXLOCK: number;
    static readonly O_NOATIME: number;
    static readonly O_NOCTTY: number;
    static readonly O_NOFOLLOW: number;
    static readonly O_NONBLOCK: number;
    static readonly O_RDONLY: number;
    static readonly O_RDWR: number;
    static readonly O_SYMLINK: number;
    static readonly O_SYNC: number;
    static readonly O_TRUNC: number;
    static readonly O_WRONLY: number;
    static readonly S_IRWXU: number;
    static readonly S_IRUSR: number;
    static readonly S_IWUSR: number;
    static readonly S_IXUSR: number;
    static readonly S_IRWXG: number;
    static readonly S_IRGRP: number;
    static readonly S_IWGRP: number;
    static readonly S_IXGRP: number;
    static readonly S_IRWXO: number;
    static readonly S_IROTH: number;
    static readonly S_IWOTH: number;
    static readonly S_IXOTH: number;
    static readonly S_IFMT: number;
    static readonly S_IFREG: number;
    static readonly S_IFDIR: number;
    static readonly S_IFCHR: number;
    static readonly S_IFBLK: number;
    static readonly S_IFIFO: number;
    static readonly S_IFLNK: number;
    static readonly S_IFSOCK: number;
    static readonly F_OK: number;
    static readonly R_OK: number;
    static readonly W_OK: number;
    static readonly X_OK: number;
    static readonly SYMLINK_DIR: number;
    static readonly SYMLINK_JUNCTION: number;

    static Open(path: string, flags: number, mode: number): Promise<File>;
    static WriteFileSync(path: string, buffer: Buffer): void;
    static ReadFileSync(path: string): Buffer;

    close(): Promise<void>;
    isClosed(): boolean;
    isClosing(): boolean;
    read(dst: Buffer, dstOffset: number, size: number, offset: number): Promise<number>;
    write(src: Buffer, srcOffset: number, size: number, offset: number): Promise<number>;
    fstat(): Promise<Stat>;
    fsync(): Promise<void>;
    fdatasync(): Promise<void>;
    ftruncate(): Promise<void>;
    fchmod(mode: number): Promise<void>;
    futime(atime: number, mtime: number): Promise<void>;
    fchown(uid: number, gid: number): Promise<void>;
}

interface FileWithPath {
    file: File;
    path: string;
}

export function unlink(path: string): Promise<void>;
export function mkdir(path: string, mode: number): Promise<void>;
export function mkdtemp(tpl: string): Promise<string>;
export function mkstemp(tpl: string): Promise<FileWithPath>;
export function rmdir(path: string): Promise<void>;
export function stat(path: string): Promise<Stat>;
export function lstat(path: string): Promise<Stat>;
export function rename(path: string, newPath: string): Promise<void>;
export function access(path: string, mode: number): Promise<number>;
export function chmod(path: string, mode: number): Promise<void>;
export function utime(path: string, atime: number, mtime: number): Promise<void>;
export function lutime(path: string, atime: number, mtime: number): Promise<void>;
export function link(path: string, newPath: string): Promise<void>;
export function symlink(path: string, newPath: string, flags: number): Promise<void>;
export function readlink(path: string): Promise<string>;
export function realpath(path: string): Promise<string>;
export function chown(path: string, uid: number, gid: number): Promise<void>;
export function lchown(path: string, uid: number, gid: number): Promise<void>;

// =======================================================
// Basic memory management: Buffer API
// =======================================================

type BufferEncoding = number;
export class Buffer {
    static readonly ENCODE_LATIN1: BufferEncoding;
    static readonly ENCODE_ASCII: BufferEncoding;
    static readonly ENCODE_UTF8: BufferEncoding;
    static readonly ENCODE_UCS2: BufferEncoding;
    static readonly ENCODE_HEX: BufferEncoding;

    static MakeFromSize(size: number): Buffer;
    static MakeFromString(string: string, encoding: BufferEncoding): Buffer;
    static MakeFromFile(path: string): Promise<Buffer>;
    static MakeFromPackageFile(package: string, path: string): Buffer;
    static MakeFromAdoptBuffer(array: Uint8Array): Buffer;

    readonly length: number;
    readonly byteArray: Uint8Array;

    byteAt(index: number): number;
    copy(offset?: OffsetT, length?: SizeT): Buffer;
    toDataView(offset?: OffsetT, length?: SizeT): DataView;
    toString(coding: number, length: SizeT): string;
    memsetZero(offset: OffsetT, length: SizeT): void;
}
