#ifndef COCOA_GALLIUM_BINDINGS_CORE_EXPORTS_H
#define COCOA_GALLIUM_BINDINGS_CORE_EXPORTS_H

#include "Core/EventSource.h"
#include "Core/Properties.h"
#include "Gallium/bindings/Base.h"
#include "Gallium/Runtime.h"
#include "Gallium/binder/Class.h"

GALLIUM_BINDINGS_NS_BEGIN

class StreamWeakBuffer;
class StreamReadIterator;
class StreamWrap;
class PipeStreamWrap;
class ProcessWrap;
class FileWrap;
class PropertyWrap;
class Buffer;

class CoreBinding : public BindingBase
{
    GALLIUM_BINDING_OBJECT

public:
    CoreBinding();
    ~CoreBinding() override;
    void onSetInstanceProperties(v8::Local<v8::Object> instance) override;
    void onRegisterClasses(v8::Isolate *isolate) override;

    ClassExport<StreamWeakBuffer>       class_stream_weak_buffer_;
    ClassExport<StreamReadIterator>     class_stream_read_iterator_;
    ClassExport<StreamWrap>             class_stream_wrap_;
    ClassExport<PipeStreamWrap>         class_pipe_stream_wrap_;
    ClassExport<ProcessWrap>            class_process_wrap_;
    ClassExport<PropertyWrap>           class_property_wrap_;
    ClassExport<Buffer>                 class_buffer_;
    ClassExport<FileWrap>               class_file_wrap_;
};

// =========================================
// Stream-based I/O
// =========================================

class StreamWeakBuffer
{
public:
    StreamWeakBuffer(v8::Local<v8::Object> streamWrap, uv_buf_t *buf, size_t readBytes);
    ~StreamWeakBuffer();

    /* JSDecl: readonly expired: boolean */
    gal_nodiscard bool isExpired() const;

    /* JSDecl: function toStrongOwnership(): Buffer */
    gal_nodiscard v8::Local<v8::Value> toStrongOwnership();

    v8::Global<v8::Object>   stream_wrap_ref_;
    uv_buf_t                *weak_buf_;
    size_t                   read_bytes_;
};

class StreamReadIterator
{
public:
    StreamReadIterator(v8::Local<v8::Object> streamWrap, StreamWrap *pStream);
    ~StreamReadIterator();

    /**
     * JSDecl:
     * interface IteratorResult {
     *   done: boolean;
     *   value: StreamWeakBuffer;
     * }
     */

    /* JSDecl: function next(): Promise<IteratorResult> */
    gal_nodiscard v8::Local<v8::Value> next() const;

    /* JSDecl: function return(): Promise<IteratorResult */
    gal_nodiscard v8::Local<v8::Value> return_() const;

    /* JSDecl: function throw(): Promise<IteratorResult> */
    gal_nodiscard v8::Local<v8::Value> throw_();

    v8::Global<v8::Object>  stream_wrap_ref_;
    StreamWrap             *stream_;
};

/* JSDecl: #[[core::non-constructible]] class Stream */
class StreamWrap
{
public:
    static v8::Local<v8::Value> OpenTTYStdin();

    template<typename T>
    static T *Allocate()
    {
        return reinterpret_cast<T *>(::malloc(sizeof(T)));
    }

    explicit StreamWrap(uv_stream_t *handle);
    ~StreamWrap();

    /* JSDecl: readonly writable: boolean */
    gal_nodiscard bool isWritable() const;

    /* JSDecl: readonly readable: boolean */
    gal_nodiscard bool isReadable() const;

    /* JSDecl: function [Symbol.asyncIterator](): StreamReadIterator */
    gal_nodiscard v8::Local<v8::Value> asyncIterator();

    void clearIterationState();

    uv_stream_t             *stream_handle_;
    bool                     is_reading_;
    uv_buf_t                 owned_buf_;
    v8::Global<v8::Promise::Resolver> current_iterate_promise_;
};

/* JSDecl: class PipeStream extends Stream */
class PipeStreamWrap : public StreamWrap
{
};

// =========================================
// POSIX Process
// =========================================
void Print(const std::string& str);
void Dump(const std::string& what);
void DumpNativeHeapProfile();

/* JSDecl: function getEnviron(): string[] */
v8::Local<v8::Value> GetEnviron();

#define GAL_PROC_STREAM_INHERIT     1
#define GAL_PROC_STREAM_REDIRECT    2

/* JSDecl: #[[core::non-constructible]] class Process */
class ProcessWrap : public PreventGCObject
{
public:
    explicit ProcessWrap(uv_process_t *handle, v8::Local<v8::Object> streams[3]);
    ~ProcessWrap() override;

    /**
     * JSDecl:
     * interface ProcessOptions {
     *   file: string;
     *   args?: string[];
     *   env?: string[];
     *   cwd?: string;
     *   uid?: number;
     *   gid?: number;
     *   stdin?: number;
     *   stdout?: number;
     *   stderr?: number;
     * }
     */

    /* JSDecl: function Fork(options: ProcessOptions): Process */
    static v8::Local<v8::Value> Fork(v8::Local<v8::Object> options);

    /**
     * JSDecl:
     * interface ProcessExitStatus {
     *   status: number;
     *   signal: number;
     * }
     */

    /* JSDecl: function promiseOnExit(): Promise<ProcessExitStatus> */
    gal_nodiscard v8::Local<v8::Value> promiseOnExit();

    /* JSDecl: function kill(signum: number): void */
    void kill(int32_t signum) const;

    /* JSDecl: readonly pid: number */
    gal_nodiscard int32_t getPid() const;

    /* JSDecl: function getPipeStream(stream: number): PipeStream */
    gal_nodiscard v8::Local<v8::Object> getPipeStream(uint32_t stream);

    void detachHandle();

    v8::Isolate     *isolate_;
    uv_process_t    *process_handle_;
    bool             has_stopped_;
    v8::Global<v8::Object>            redirected_streams_[3];
    v8::Global<v8::Promise::Resolver> on_exit_promise_;
};

// =========================================
// POSIX Filesystem API - asynchronous
// =========================================
struct FsRequest;

/* JSDecl: #[[core::non-constructible]] class File */
class FileWrap
{
public:
    explicit FileWrap(uv_file fd);
    ~FileWrap();

    /* JSDecl: function close(): Promise<void> */
    gal_nodiscard v8::Local<v8::Value> close();

    /* JSDecl: function isClosed(): boolean */
    gal_nodiscard bool isClosed() const;

    /* JSDecl: function isClosing(): boolean */
    gal_nodiscard bool isClosing() const;

    /* JSDecl: function read(dst: Buffer, dstOffset: number, size: number, offset: number): Promise<number> */
    gal_nodiscard v8::Local<v8::Value> read(v8::Local<v8::Value> dst, int64_t dstOffset,
                                            size_t size, int64_t offset);

    /* JSDecl: function write(src: Buffer, srcOffset: number, size: number, offset: number): Promise<number> */
    gal_nodiscard v8::Local<v8::Value> write(v8::Local<v8::Value> src, int64_t srcOffset,
                                             size_t size, int64_t offset);

    /**
     * JSDecl:
     * interface Stat {
     *     dev: number;
     *     mode: number;
     *     nlink: number;
     *     uid: number;
     *     gid: number;
     *     rdev: number;
     *     blksize: number;
     *     ino: number;
     *     size: number;
     *     blocks: number;
     *     atimeMs: number;
     *     mtimeMs: number;
     *     ctimeMs: number;
     *     atime: Date;
     *     mtime: Date;
     *     ctime: Date;
     * }
     */

    /* JSDecl: function open(path: string, flags: number, mode: number): Promise<File> */
    static v8::Local<v8::Value> Open(const std::string& path, int32_t flags, int32_t mode);

    /* JSDecl: function fstat(): Promise<Stat> */
    gal_nodiscard v8::Local<v8::Value> fstat();

    /* JSDecl: function fsync(): Promise<void> */
    gal_nodiscard v8::Local<v8::Value> fsync();

    /* JSDecl: function fdatasync(): Promise<void> */
    gal_nodiscard v8::Local<v8::Value> fdatasync();

    /* JSDecl: function ftruncate(length: number): Promise<void> */
    gal_nodiscard v8::Local<v8::Value> ftruncate(off_t length);

    /* JSDecl: function fchmod(mode: number): Promise<void> */
    gal_nodiscard v8::Local<v8::Value> fchmod(int32_t mode);

    /* JSDecl: function futime(atime: number, mtime: number): Promise<void> */
    v8::Local<v8::Value> futime(double atime, double mtime);

    /* JSDecl: function fchown(uid: number, gid: number): Promise<void> */
    v8::Local<v8::Value> fchown(uv_uid_t uid, uv_gid_t gid);

    bool        closed_;
    bool        is_closing_;
    uv_file     fd_;
    std::list<FsRequest*> pending_requests_;
};

/* JSDecl: function unlink(path: string): Promise<void> */
v8::Local<v8::Value> Unlink(const std::string& path);

/* JSDecl: function mkdir(path: string, mode: number): Promise<void> */
v8::Local<v8::Value> Mkdir(const std::string& path, int32_t mode);

/* JSDecl: function mkdtemp(tpl: string): Promise<string> */
v8::Local<v8::Value> Mkdtemp(const std::string& tpl);

/**
 * JSDecl:
 * interface FileWithPath {
 *     file: File;
 *     path: string;
 * }
 */

/* JSDecl: function mkstemp(tpl: string): Promise<FileWithPath> */
v8::Local<v8::Value> Mkstemp(const std::string& tpl);

/* JSDecl: function rmdir(path: string): Promise<void> */
v8::Local<v8::Value> Rmdir(const std::string& path);

/* JSDecl: function stat(path: string): Promise<Stat> */
v8::Local<v8::Value> Stat(const std::string& path);

/* JSDecl: function lstat(path: string): Promise<Stat> */
v8::Local<v8::Value> LStat(const std::string& path);

/* JSDecl: function rename(path: string, newPath: string): Promise<void> */
v8::Local<v8::Value> Rename(const std::string& path, const std::string& newPath);

/* JSDecl: function access(path: string, mode: number): Promise<number> */
v8::Local<v8::Value> Access(const std::string& path, int32_t mode);

/* JSDecl: function chmod(path: string, mode: number): Promise<void> */
v8::Local<v8::Value> Chmod(const std::string& path, int32_t mode);

/* JSDecl: function utime(path: string, atime: number, mtime: number): Promise<void> */
v8::Local<v8::Value> UTime(const std::string& path, double atime, double mtime);

/* JSDecl: function lutime(path: string, atime: number, mtime: number): Promise<void> */
v8::Local<v8::Value> LUTime(const std::string& path, double atime, double mtime);

/* JSDecl: function link(path: string, newPath: string): Promise<void> */
v8::Local<v8::Value> Link(const std::string& path, const std::string& newPath);

/* JSDecl: function symlink(path: string, newPath: string, flags: number): Promise<void> */
v8::Local<v8::Value> Symlink(const std::string& path, const std::string& newPath, int32_t flags);

/* JSDecl: function readlink(path: string): Promise<string> */
v8::Local<v8::Value> Readlink(const std::string& path);

/* JSDecl: function realpath(path: string): Promise<string> */
v8::Local<v8::Value> Realpath(const std::string& path);

/* JSDecL: function chown(path: string, uid: number, gid: number): Promise<void> */
v8::Local<v8::Value> Chown(const std::string& path, uv_uid_t uid, uv_gid_t gid);

/* JSDecL: function lchown(path: string, uid: number, gid: number): Promise<void> */
v8::Local<v8::Value> LChown(const std::string& path, uv_uid_t uid, uv_gid_t gid);

/* JSDecl: #[[core::non-constructible]] class Property */
class PropertyWrap
{
public:
    /* JSDecl: static const PROT_*: number */
    enum class Prot : uint32_t
    {
        /* JS and C++ readable/writable */
        kPublic     = 0,

        /* JS readable, C++ readable/writable */
        kPrivate    = 1,

        kLast       = kPrivate
    };

    /* JSDecl: static const TYPE_*: number */
    enum class Type : uint32_t
    {
        kObject     = 0,
        kArray      = 1,
        kData       = 2,
        kLast       = kData
    };

    explicit PropertyWrap(const std::shared_ptr<PropertyNode>& node);
    ~PropertyWrap();

    static void InstallProperties();
    static v8::Local<v8::Object> GetWrap(v8::Isolate *isolate, const std::shared_ptr<PropertyNode>& node);

    gal_nodiscard std::shared_ptr<PropertyNode> lockNode() const {
        return fNode.lock();
    }

    /* JSDecl: const type: number */
    gal_nodiscard v8::Local<v8::Value> getType() const;

    /* JSDecl: const parent: Property */
    gal_nodiscard v8::Local<v8::Value> getParent() const;

    /* JSDecl: name: string */
    gal_nodiscard v8::Local<v8::Value> getName() const;
    void setName(v8::Local<v8::Value> name) const;

    /* JSDecl: const protection: number */
    gal_nodiscard v8::Local<v8::Value> getProtection() const;

    /* JSDecl: const numberOfChildren: number */
    gal_nodiscard v8::Local<v8::Value> getNumberOfChildren() const;

    /* JSDecl: function foreachChild(func: (child: Property) => void): void */
    void foreachChild(v8::Local<v8::Value> callback) const;

    /* JSDecl: function findChild(name: string): Property */
    gal_nodiscard v8::Local<v8::Value> findChild(const std::string& name) const;

    /* JSDecl: function insertChild(type: number, name: string): Property */
    gal_nodiscard v8::Local<v8::Value> insertChild(int32_t type, const std::string& name) const;

    /* JSDecl: function pushbackChild(type: number): Property */
    gal_nodiscard v8::Local<v8::Value> pushbackChild(int32_t type) const;

    /* JSDecl: function detachFromParent(): void */
    void detachFromParent();

    /* JSDecl: function extract(): any (maybe undefined) */
    gal_nodiscard v8::Local<v8::Value> extract() const;

    /* JSDecl: function resetData(value?: any): void */
    void resetData(const v8::FunctionCallbackInfo<v8::Value>& value) const;

    /* JSDecl: function hasData(): boolean */
    gal_nodiscard bool hasData() const;

    /* JSDecl: function dataValueRTTI(): string */
    gal_nodiscard std::string dataTypeinfo() const;

private:
    void checkNodeProtectionForJSWriting() const;

    std::weak_ptr<PropertyNode>   fNode;
};

class Buffer
{
public:
    enum class Encoding : uint32_t
    {
        kLatin1     = 1,
        kUtf8,
        kUcs2,
        kHex,

        kLast = kHex
    };

    explicit Buffer(const v8::FunctionCallbackInfo<v8::Value>& info);
    /**
     * Construct an empty buffer (with @p array_ empty)
     */
    Buffer() = default;
    ~Buffer();

    static v8::Local<v8::Object> MakeFromCopy(Buffer *other, off_t offset, ssize_t size = -1);
    static v8::Local<v8::Object> MakeFromSize(size_t size);
    static v8::Local<v8::Object> MakeFromPtrCopy(void *data, size_t size);
    static v8::Local<v8::Object> MakeFromPtrWithoutCopy(void *data, size_t size,
                                                        v8::BackingStore::DeleterCallback deleter, void *closure);

    size_t length();
    uint8_t byteAt(int64_t idx);
    v8::Local<v8::Value> copy(const v8::FunctionCallbackInfo<v8::Value>& args);
    v8::Local<v8::Value> toDataView(const v8::FunctionCallbackInfo<v8::Value>& args);
    v8::Local<v8::Value> toString(uint32_t coding, int32_t length);
    void memsetZero(uint32_t offset, uint32_t length);

    uint8_t *getWriteableDataPointerByte();

private:
    v8::Global<v8::Uint8Array>          array_;
    std::shared_ptr<v8::BackingStore>   backing_store_;
};

GALLIUM_BINDINGS_NS_END
#endif //COCOA_GALLIUM_BINDINGS_CORE_EXPORTS_H
