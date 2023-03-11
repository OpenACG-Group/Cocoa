/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef COCOA_GALLIUM_BINDINGS_CORE_EXPORTS_H
#define COCOA_GALLIUM_BINDINGS_CORE_EXPORTS_H

#include "Core/EventSource.h"
#include "Gallium/bindings/Base.h"
#include "Gallium/Runtime.h"
#include "Gallium/binder/Class.h"

GALLIUM_BINDINGS_NS_BEGIN

class StreamWeakBuffer;
class StreamAsyncIterator;
class StreamWrap;
class PipeStreamWrap;
class ProcessWrap;
class FileWrap;
class Buffer;

void CoreSetInstanceProperties(v8::Local<v8::Object> instance);

// =========================================
// Stream-based I/O
// =========================================

class StreamAsyncIterator
{
public:
    explicit StreamAsyncIterator(StreamWrap *stream);
    ~StreamAsyncIterator();

    /**
     * TSDecl:
     * interface IteratorResult {
     *   done: boolean;
     *   value: {
     *      length: number;
     *      buffer: Buffer;
     *   };
     * }
     */

    //! TSDecl: function next(): Promise<IteratorResult>
    v8::Local<v8::Value> next();

    //! TSDecl: function return(value?: any): Promise<IteratorResult>
    void return_(v8::FunctionCallbackInfo<v8::Value> info);

    //! TSDecl: function throw(exception?: any): Promise<IteratorResult>
    void throw_(v8::FunctionCallbackInfo<v8::Value> info);

    void Dispose();

    g_nodiscard g_inline bool IsPendingState() const {
        return pending_;
    }

    v8::Local<v8::Promise> EnterPendingState();
    void FinishPendingState();

    g_inline void SetCurrentBuffer(v8::Isolate *isolate, v8::Local<v8::Object> obj)
    {
        current_buffer_.Reset(isolate, obj);
    }

    g_nodiscard g_inline v8::Local<v8::Object> GetCurrentBuffer(v8::Isolate *isolate)
    {
        CHECK(!current_buffer_.IsEmpty());
        return current_buffer_.Get(isolate);
    }

    g_nodiscard g_inline
    v8::Local<v8::Promise::Resolver> GetCurrentResolver(v8::Isolate *isolate)
    {
        CHECK(!current_resolver_.IsEmpty());
        return current_resolver_.Get(isolate);
    }

private:
    bool                        disposed_;
    StreamWrap                 *stream_;
    bool                                pending_;
    v8::Global<v8::Promise::Resolver>   current_resolver_;
    v8::Global<v8::Object>              current_buffer_;
};

//! TSDecl: class Stream
class StreamWrap
{
    friend class StreamAsyncIterator;

public:
    explicit StreamWrap(uv_stream_t *handle);
    ~StreamWrap();

    //! TSDecl: readonly writable: boolean
    g_nodiscard bool isWritable() const;

    //! TSDecl: readonly readable: boolean
    g_nodiscard bool isReadable() const;

    //! TSDecl: function [Symbol.asyncIterator](): StreamAsyncIterator
    v8::Local<v8::Value> asyncIterator();

    //! TSDecl: function write(buffers: Array<Buffer>): Promise<void>
    v8::Local<v8::Value> write(v8::Local<v8::Value> buffers);

protected:
    void Dispose();

private:
    static void OnAllocateCallback(uv_handle_t *hnd, size_t suggested, uv_buf_t *result);
    static void OnReadCallback(uv_stream_t *st, ssize_t nread, const uv_buf_t *buf);

    bool                     disposed_;
    uv_stream_t             *stream_handle_;
    v8::Global<v8::Object>   async_iterator_obj_;
    StreamAsyncIterator     *async_iterator_;
};

//! TSDecl: class TTYStream extends Stream
class TTYStreamWrap : public StreamWrap
{
public:
    explicit TTYStreamWrap(uv_tty_t *handle)
        : StreamWrap(reinterpret_cast<uv_stream_t*>(handle))
        , closed_(false)
        , handle_(handle) {}
    ~TTYStreamWrap();

    //! TSDecl: function OpenStdin(): TTYStreamWrap
    static v8::Local<v8::Value> OpenStdin();

    //! TSDecl: function OpenStdout(): TTYStreamWrap
    static v8::Local<v8::Value> OpenStdout();

    //! TSDecl: function OpenStderr(): TTYStreamWrap
    static v8::Local<v8::Value> OpenStderr();

    //! TSDecl: function close(): void
    void close();

private:
    static v8::Local<v8::Value> OpenFromFd(int fd);

    bool             closed_;
    uv_tty_t        *handle_;
};

// =========================================
// POSIX Process
// =========================================
void Print(const std::string& str);
void Dump(const std::string& what);
void DumpNativeHeapProfile();

//! TSDecl: function getEnviron(): string[]
v8::Local<v8::Value> GetEnviron();

#define GAL_PROC_STREAM_INHERIT     1
#define GAL_PROC_STREAM_REDIRECT    2

//! TSDecl: #[[core::non-constructible]] class Process
class ProcessWrap : public PreventGCObject
{
public:
    explicit ProcessWrap(uv_process_t *handle, v8::Local<v8::Object> streams[3]);
    ~ProcessWrap() override;

    /**
     * TSDecl:
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

    //! TSDecl: function Fork(options: ProcessOptions): Process
    static v8::Local<v8::Value> Fork(v8::Local<v8::Object> options);

    /**
     * TSDecl:
     * interface ProcessExitStatus {
     *   status: number;
     *   signal: number;
     * }
     */

    //! TSDecl: function promiseOnExit(): Promise<ProcessExitStatus>
    g_nodiscard v8::Local<v8::Value> promiseOnExit();

    //! TSDecl: function kill(signum: number): void
    void kill(int32_t signum) const;

    //! TSDecl: readonly pid: number
    g_nodiscard int32_t getPid() const;

    //! TSDecl: function getPipeStream(stream: number): PipeStream
    g_nodiscard v8::Local<v8::Object> getPipeStream(uint32_t stream);

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

//! TSDecl: #[[core::non-constructible]] class File
class FileWrap
{
public:
    explicit FileWrap(uv_file fd);
    ~FileWrap();

    //! TSDecl: function WriteFileSync(path: string, buffer: core:Buffer): void
    static v8::Local<v8::Value> WriteFileSync(const std::string& path,
                                              v8::Local<v8::Value> buffer);

    //! TSDecl: function ReadFileSync(path: string): core:Buffer
    static v8::Local<v8::Value> ReadFileSync(const std::string& path);

    //! TSDecl: function Open(path: string, flags: number, mode: number): Promise<File>
    static v8::Local<v8::Value> Open(const std::string& path, int32_t flags, int32_t mode);

    //! TSDecl: function close(): Promise<void>
    g_nodiscard v8::Local<v8::Value> close();

    //! TSDecl: function isClosed(): boolean
    g_nodiscard bool isClosed() const;

    //! TSDecl: function isClosing(): boolean
    g_nodiscard bool isClosing() const;

    //! TSDecl: function read(dst: Buffer, dstOffset: number, size: number, offset: number): Promise<number>
    g_nodiscard v8::Local<v8::Value> read(v8::Local<v8::Value> dst, int64_t dstOffset,
                                            size_t size, int64_t offset);

    //! TSDecl: function write(src: Buffer, srcOffset: number, size: number, offset: number): Promise<number>
    g_nodiscard v8::Local<v8::Value> write(v8::Local<v8::Value> src, int64_t srcOffset,
                                             size_t size, int64_t offset);

    /**
     * TSDecl:
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

    //! TSDecl: function fstat(): Promise<Stat>
    g_nodiscard v8::Local<v8::Value> fstat();

    //! TSDecl: function fsync(): Promise<void>
    g_nodiscard v8::Local<v8::Value> fsync();

    //! TSDecl: function fdatasync(): Promise<void>
    g_nodiscard v8::Local<v8::Value> fdatasync();

    //! TSDecl: function ftruncate(length: number): Promise<void>
    g_nodiscard v8::Local<v8::Value> ftruncate(off_t length);

    //! TSDecl: function fchmod(mode: number): Promise<void>
    g_nodiscard v8::Local<v8::Value> fchmod(int32_t mode);

    //! TSDecl: function futime(atime: number, mtime: number): Promise<void>
    v8::Local<v8::Value> futime(double atime, double mtime);

    //! TSDecl: function fchown(uid: number, gid: number): Promise<void>
    v8::Local<v8::Value> fchown(uv_uid_t uid, uv_gid_t gid);

    bool        closed_;
    bool        is_closing_;
    uv_file     fd_;
    std::list<FsRequest*> pending_requests_;
};

//! TSDecl: function unlink(path: string): Promise<void>
v8::Local<v8::Value> Unlink(const std::string& path);

//! TSDecl: function mkdir(path: string, mode: number): Promise<void>
v8::Local<v8::Value> Mkdir(const std::string& path, int32_t mode);

//! TSDecl: function mkdtemp(tpl: string): Promise<string>
v8::Local<v8::Value> Mkdtemp(const std::string& tpl);

/**
 * TSDecl:
 * interface FileWithPath {
 *     file: File;
 *     path: string;
 * }
 */

//! TSDecl: function mkstemp(tpl: string): Promise<FileWithPath>
v8::Local<v8::Value> Mkstemp(const std::string& tpl);

//! TSDecl: function rmdir(path: string): Promise<void>
v8::Local<v8::Value> Rmdir(const std::string& path);

//! TSDecl: function stat(path: string): Promise<Stat>
v8::Local<v8::Value> Stat(const std::string& path);

//! TSDecl: function lstat(path: string): Promise<Stat>
v8::Local<v8::Value> LStat(const std::string& path);

//! TSDecl: function rename(path: string, newPath: string): Promise<void>
v8::Local<v8::Value> Rename(const std::string& path, const std::string& newPath);

//! TSDecl: function access(path: string, mode: number): Promise<number>
v8::Local<v8::Value> Access(const std::string& path, int32_t mode);

//! TSDecl: function chmod(path: string, mode: number): Promise<void>
v8::Local<v8::Value> Chmod(const std::string& path, int32_t mode);

//! TSDecl: function utime(path: string, atime: number, mtime: number): Promise<void>
v8::Local<v8::Value> UTime(const std::string& path, double atime, double mtime);

//! TSDecl: function lutime(path: string, atime: number, mtime: number): Promise<void>
v8::Local<v8::Value> LUTime(const std::string& path, double atime, double mtime);

//! TSDecl: function link(path: string, newPath: string): Promise<void>
v8::Local<v8::Value> Link(const std::string& path, const std::string& newPath);

//! TSDecl: function symlink(path: string, newPath: string, flags: number): Promise<void>
v8::Local<v8::Value> Symlink(const std::string& path, const std::string& newPath, int32_t flags);

//! TSDecl: function readlink(path: string): Promise<string>
v8::Local<v8::Value> Readlink(const std::string& path);

//! TSDecl: function realpath(path: string): Promise<string>
v8::Local<v8::Value> Realpath(const std::string& path);

/* JSDecL: function chown(path: string, uid: number, gid: number): Promise<void> */
v8::Local<v8::Value> Chown(const std::string& path, uv_uid_t uid, uv_gid_t gid);

/* JSDecL: function lchown(path: string, uid: number, gid: number): Promise<void> */
v8::Local<v8::Value> LChown(const std::string& path, uv_uid_t uid, uv_gid_t gid);

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

    /**
     * Construct an empty buffer (with @p array_ empty)
     */
    Buffer();
    ~Buffer();

    static v8::Local<v8::Object> MakeFromCopy(Buffer *other, off_t offset, ssize_t size = -1);

    //! TSDecl: function MakeFromSize(size: number): Buffer
    static v8::Local<v8::Object> MakeFromSize(size_t size);

    //! TSDecl: function MakeFromString(string: string, encoding: string): Buffer
    static v8::Local<v8::Object> MakeFromString(v8::Local<v8::String> string, uint32_t encoding);

    //! TSDecl: function MakeFromFile(path: string): Promise<Buffer>
    static v8::Local<v8::Promise> MakeFromFile(const std::string& path);

    //! TSDecl: function MakeFromAdoptBuffer(array: Uint8Array): Buffer
    static v8::Local<v8::Object> MakeFromAdoptBuffer(v8::Local<v8::Object> array);

    //! TSDecl: function MakeFromBase64(base64: string): Buffer
    static v8::Local<v8::Object> MakeFromBase64(v8::Local<v8::String> base64);

    /**
     * Similar to `MakeFromPtrWithoutCopy`, but the caller can use lambda expression
     * as the deleter to capture the ownership of external resource.
     * `closure_captured_external` will be called when the buffer is destructed.
     */
    static v8::Local<v8::Object> MakeFromExternal(void *data,
                                                  size_t size,
                                                  std::function<void()> closure_captured_external);

    static v8::Local<v8::Object> MakeFromPtrCopy(const void *data, size_t size);
    static v8::Local<v8::Object> MakeFromPtrWithoutCopy(void *data,
                                                        size_t size,
                                                        v8::BackingStore::DeleterCallback deleter,
                                                        void *closure);

    v8::Local<v8::Value> getByteArray();
    size_t length();
    uint8_t byteAt(int64_t idx);
    v8::Local<v8::Value> copy(const v8::FunctionCallbackInfo<v8::Value>& args);
    v8::Local<v8::Value> toDataView(const v8::FunctionCallbackInfo<v8::Value>& args);
    v8::Local<v8::Value> toString(uint32_t coding, int32_t length);
    void memsetZero(uint32_t offset, uint32_t length);

    g_private_api uint8_t *addressU8();

private:
    size_t                              alloc_size_hint_;
    v8::Global<v8::Uint8Array>          array_;
    std::shared_ptr<v8::BackingStore>   backing_store_;
};

GALLIUM_BINDINGS_NS_END
#endif //COCOA_GALLIUM_BINDINGS_CORE_EXPORTS_H
