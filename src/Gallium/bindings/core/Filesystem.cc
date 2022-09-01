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

#include <cerrno>
#include <cstdlib>

#include "uv.h"

#include "Core/EventLoop.h"
#include "Core/Exception.h"
#include "Gallium/bindings/core/Exports.h"
GALLIUM_BINDINGS_NS_BEGIN

struct FsRequest
{
    explicit FsRequest(v8::Isolate *isolate, const char *syscall, void *closure)
        : req_{}
        , isolate_(isolate)
        , syscall_(syscall)
        , closure_(closure)
        , closure_collected_(false)
        , buffer_(nullptr) {
        v8::HandleScope scope(isolate);
        v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
        auto r = v8::Promise::Resolver::New(ctx).ToLocalChecked();
        resolver_.Reset(isolate, r);
        uv_req_set_data(reinterpret_cast<uv_req_t*>(&req_), this);
    }

    ~FsRequest() {
        uv_fs_req_cleanup(&req_);
        resolver_.Reset();
        buffer_ref_.Reset();
    }

    static FsRequest *Cast(uv_fs_t *req) {
        return reinterpret_cast<FsRequest*>(
                uv_req_get_data(reinterpret_cast<uv_req_t*>(req)));
    }

    v8::Local<v8::Promise::Resolver> GetResolver() {
        return resolver_.Get(isolate_);
    }

    void Resolve(v8::Local<v8::Value> value) {
        v8::Local<v8::Context> ctx = isolate_->GetCurrentContext();
        GetResolver()->Resolve(ctx, value).Check();
    }

    void Reject(v8::Local<v8::Value> value) {
        v8::Local<v8::Context> ctx = isolate_->GetCurrentContext();
        GetResolver()->Reject(ctx, value).Check();
    }

    uv_fs_t req_;
    v8::Global<v8::Promise::Resolver> resolver_;
    v8::Isolate *isolate_;

    const char *syscall_;

    void *closure_;
    /* The object referenced by @p closure_ was destructed by GC */
    bool closure_collected_;

    /* Keep a reference of the used Buffer object here to avoid being collected by GC */
    v8::Global<v8::Value> buffer_ref_;
    Buffer *buffer_;
};

void callback_reject_error_code(FsRequest *req, ssize_t err, const char *syscall_)
{
    v8::Isolate *i = req->isolate_;
    v8::Local<v8::Context> ctx = i->GetCurrentContext();
    v8::Local<v8::String> str = binder::to_v8(i, uv_strerror(static_cast<int>(err)));
    v8::Local<v8::Object> e = v8::Exception::Error(str).As<v8::Object>();
    e->Set(ctx, binder::to_v8(i, "errno"), binder::to_v8(i, err)).Check();
    e->Set(ctx, binder::to_v8(i, "syscall"), binder::to_v8(i, syscall_)).Check();
    req->Reject(e);
}

#define NEW_REQUEST(syscall_, self_)  auto *req = new FsRequest(i, syscall_, self_)
#define RET_PROMISE                   return req->GetResolver()->GetPromise()

#define CALLBACK_PROLOGUE \
    FsRequest *req = FsRequest::Cast(ptr); \
    v8::Isolate *i = req->isolate_;        \
    v8::HandleScope __scope(i);            \
    ScopeExitAutoInvoker __deleter([req] {        \
        delete req;                        \
    });

#define FILE_CALLBACK_PROLOGUE \
    CALLBACK_PROLOGUE                                                \
    ScopeExitAutoInvoker __pop([req] {                                      \
        if (!req->closure_collected_ && req->closure_) {             \
            auto *wrap = reinterpret_cast<FileWrap*>(req->closure_); \
            wrap->pending_requests_.remove(req);                     \
        }                                                            \
    });
    
    
#define API_PROLOGUE                                \
    v8::Isolate *i = v8::Isolate::GetCurrent();     \
    uv_loop_t *loop = EventLoop::Ref().handle();    \

void on_open_callback(uv_fs_t *ptr)
{
    CALLBACK_PROLOGUE

    if (req->req_.result < 0)
        callback_reject_error_code(req, req->req_.result, req->syscall_);
    else
    {
        v8::Local<v8::Object> file = binder::Class<FileWrap>::create_object(i, req->req_.result);
        req->Resolve(file);
    }
}

void on_undefined_promise_callback(uv_fs_t *ptr)
{
    CALLBACK_PROLOGUE
    if (req->req_.result < 0)
        callback_reject_error_code(req, req->req_.result, req->syscall_);
    else
        req->Resolve(v8::Undefined(i));
}

v8::Local<v8::Value> FileWrap::Open(const std::string& path, int32_t flags, int32_t mode)
{
    API_PROLOGUE
    NEW_REQUEST("open", nullptr);
    uv_fs_open(loop, &req->req_, path.c_str(), flags, mode, on_open_callback);
    RET_PROMISE;
}

v8::Local<v8::Value> Unlink(const std::string& path)
{
    API_PROLOGUE
    NEW_REQUEST("unlink", nullptr);
    uv_fs_unlink(loop, &req->req_, path.c_str(), on_undefined_promise_callback);
    RET_PROMISE;
}

v8::Local<v8::Value> Mkdir(const std::string& path, int32_t mode)
{
    API_PROLOGUE
    NEW_REQUEST("mkdir", nullptr);
    uv_fs_mkdir(loop, &req->req_, path.c_str(), mode, on_undefined_promise_callback);
    RET_PROMISE;
}

void on_mkdtemp_callback(uv_fs_t *ptr)
{
    CALLBACK_PROLOGUE
    if (req->req_.result < 0)
        callback_reject_error_code(req, req->req_.result, req->syscall_);
    else
        req->Resolve(binder::to_v8(i, req->req_.path));
}

v8::Local<v8::Value> Mkdtemp(const std::string& tpl)
{
    API_PROLOGUE
    NEW_REQUEST("mkdtemp", nullptr);
    uv_fs_mkdtemp(loop, &req->req_, tpl.c_str(), on_mkdtemp_callback);
    RET_PROMISE;
}

void on_mkstemp_callback(uv_fs_t *ptr)
{
    CALLBACK_PROLOGUE
    if (req->req_.result < 0)
        callback_reject_error_code(req, req->req_.result, req->syscall_);
    else
    {
        v8::Local<v8::Context> ctx = i->GetCurrentContext();
        v8::Local<v8::Object> w = v8::Object::New(i);
        w->Set(ctx, binder::to_v8(i, "path"), binder::to_v8(i, req->req_.path)).Check();
        v8::Local<v8::Object> file = binder::Class<FileWrap>::create_object(i, req->req_.result);
        w->Set(ctx, binder::to_v8(i, "file"), file).Check();
        req->Resolve(w);
    }
}

v8::Local<v8::Value> Mkstemp(const std::string& tpl)
{
    API_PROLOGUE
    NEW_REQUEST("mkstemp", nullptr);
    uv_fs_mkstemp(loop, &req->req_, tpl.c_str(), on_mkstemp_callback);
    RET_PROMISE;
}

v8::Local<v8::Value> Rmdir(const std::string& path)
{
    API_PROLOGUE
    NEW_REQUEST("rmdir", nullptr);
    uv_fs_rmdir(loop, &req->req_, path.c_str(), on_undefined_promise_callback);
    RET_PROMISE;
}

double calc_uv_timespec_milliseconds(const uv_timespec_t& tv)
{
    return (static_cast<double>(tv.tv_sec) * 1e3) +
           (static_cast<double>(tv.tv_nsec) / 1e6);
}

v8::Local<v8::Value> make_date_from_uv_timespec(const uv_timespec_t& tv, v8::Local<v8::Context> ctx)
{
    double ms = calc_uv_timespec_milliseconds(tv);
    return v8::Date::New(ctx, ms).ToLocalChecked();
}

v8::Local<v8::Object> make_stat_object(uv_stat_t *st, v8::Isolate *i)
{
    v8::Local<v8::Object> r = v8::Object::New(i);
    v8::Local<v8::Context> ctx = i->GetCurrentContext();
#define SET(k, v) r->Set(ctx, binder::to_v8(i, k), binder::to_v8(i, v)).Check()
#define SET_(k)   SET(#k, st->st_##k)

    SET_(dev);
    SET_(mode);
    SET_(nlink);
    SET_(uid);
    SET_(gid);
    SET_(rdev);
    SET_(blksize);
    SET_(ino);
    SET_(size);
    SET_(blocks);
    SET("atimeMs", calc_uv_timespec_milliseconds(st->st_atim));
    SET("mtimeMs", calc_uv_timespec_milliseconds(st->st_mtim));
    SET("ctimeMs", calc_uv_timespec_milliseconds(st->st_ctim));
    SET("atime", make_date_from_uv_timespec(st->st_atim, ctx));
    SET("mtime", make_date_from_uv_timespec(st->st_mtim, ctx));
    SET("ctime", make_date_from_uv_timespec(st->st_ctim, ctx));

#undef SET_
#undef SET
    return r;
}

void on_stat_or_lstat_callback(uv_fs_t *ptr)
{
    CALLBACK_PROLOGUE
    if (req->req_.result < 0)
        callback_reject_error_code(req, req->req_.result, req->syscall_);
    else
        req->Resolve(make_stat_object(&req->req_.statbuf, i));
}

v8::Local<v8::Value> Stat(const std::string& path)
{
    API_PROLOGUE
    NEW_REQUEST("stat", nullptr);
    uv_fs_stat(loop, &req->req_, path.c_str(), on_stat_or_lstat_callback);
    RET_PROMISE;
}

v8::Local<v8::Value> LStat(const std::string& path)
{
    API_PROLOGUE
    NEW_REQUEST("lstat", nullptr);
    uv_fs_lstat(loop, &req->req_, path.c_str(), on_stat_or_lstat_callback);
    RET_PROMISE;
}

v8::Local<v8::Value> Rename(const std::string& path, const std::string& newPath)
{
    API_PROLOGUE
    NEW_REQUEST("rename", nullptr);
    uv_fs_rename(loop, &req->req_, path.c_str(), newPath.c_str(), on_undefined_promise_callback);
    RET_PROMISE;
}

void on_access_callback(uv_fs_t *ptr)
{
    CALLBACK_PROLOGUE
    req->Resolve(binder::to_v8(i, ptr->result));
}

v8::Local<v8::Value> Access(const std::string& path, int32_t mode)
{
    API_PROLOGUE
    NEW_REQUEST("access", nullptr);
    uv_fs_access(loop, &req->req_, path.c_str(), mode, on_access_callback);
    RET_PROMISE;
}

v8::Local<v8::Value> Chmod(const std::string& path, int32_t mode)
{
    API_PROLOGUE
    NEW_REQUEST("chmod", nullptr);
    uv_fs_chmod(loop, &req->req_, path.c_str(), mode, on_undefined_promise_callback);
    RET_PROMISE;
}

v8::Local<v8::Value> UTime(const std::string& path, double atime, double mtime)
{
    API_PROLOGUE
    NEW_REQUEST("utime", nullptr);
    uv_fs_utime(loop, &req->req_, path.c_str(), atime, mtime, on_undefined_promise_callback);
    RET_PROMISE;
}

v8::Local<v8::Value> LUTime(const std::string& path, double atime, double mtime)
{
    API_PROLOGUE
    NEW_REQUEST("lutime", nullptr);
    uv_fs_lutime(loop, &req->req_, path.c_str(), atime, mtime, on_undefined_promise_callback);
    RET_PROMISE;
}

v8::Local<v8::Value> Symlink(const std::string& path, const std::string& newPath, int32_t flags)
{
    API_PROLOGUE
    NEW_REQUEST("symlink", nullptr);
    uv_fs_symlink(loop, &req->req_, path.c_str(), newPath.c_str(), flags, on_undefined_promise_callback);
    RET_PROMISE;
}

v8::Local<v8::Value> Link(const std::string& path, const std::string& newPath)
{
    API_PROLOGUE
    NEW_REQUEST("link", nullptr);
    uv_fs_link(loop, &req->req_, path.c_str(), newPath.c_str(), on_undefined_promise_callback);
    RET_PROMISE;
}

void on_resolve_ptr_promise_callback(uv_fs_t *ptr)
{
    CALLBACK_PROLOGUE
    if (ptr->result < 0)
        callback_reject_error_code(req, ptr->result, req->syscall_);
    else
        req->Resolve(binder::to_v8(req->isolate_, static_cast<const char*>(ptr->ptr)));
}

v8::Local<v8::Value> Readlink(const std::string& path)
{
    API_PROLOGUE
    NEW_REQUEST("readlink", nullptr);
    uv_fs_readlink(loop, &req->req_, path.c_str(), on_resolve_ptr_promise_callback);
    RET_PROMISE;
}

v8::Local<v8::Value> Realpath(const std::string& path)
{
    API_PROLOGUE
    NEW_REQUEST("realpath", nullptr);
    uv_fs_realpath(loop, &req->req_, path.c_str(), on_resolve_ptr_promise_callback);
    RET_PROMISE;
}

v8::Local<v8::Value> Chown(const std::string& path, uv_uid_t uid, uv_gid_t gid)
{
    API_PROLOGUE
    NEW_REQUEST("chown", nullptr);
    uv_fs_chown(loop, &req->req_, path.c_str(), uid, gid, on_undefined_promise_callback);
    RET_PROMISE;
}

v8::Local<v8::Value> LChown(const std::string& path, uv_uid_t uid, uv_gid_t gid)
{
    API_PROLOGUE
    NEW_REQUEST("lchown", nullptr);
    uv_fs_lchown(loop, &req->req_, path.c_str(), uid, gid, on_undefined_promise_callback);
    RET_PROMISE;
}

FileWrap::FileWrap(uv_file fd)
    : closed_(false)
    , is_closing_(false)
    , fd_(fd)
{
}

FileWrap::~FileWrap()
{
    if (!closed_ && !is_closing_)
    {
        uv_loop_t *loop = EventLoop::Ref().handle();
        uv_fs_t req{};
        uv_fs_close(loop, &req, fd_, nullptr);
    }

    for (FsRequest *r : pending_requests_)
    {
        r->closure_collected_ = true;
        r->closure_ = nullptr;
    }
}

void on_close_callback(uv_fs_t *ptr)
{
    FILE_CALLBACK_PROLOGUE

    if (req->req_.result < 0)
        callback_reject_error_code(req, req->req_.result, req->syscall_);
    else
        req->Resolve(v8::Undefined(i));

    if (!req->closure_collected_)
    {
        auto *wrap = reinterpret_cast<FileWrap*>(req->closure_);
        wrap->is_closing_ = false;
        wrap->closed_ = true;
    }
}

#define CHECK_CLOSED                             \
    do {                                         \
        if (closed_ || is_closing_) {            \
            binder::JSException::Throw(          \
                binder::ExceptT::kError,         \
                "File has already been closed or is closing"); \
        } \
    } while (false)

v8::Local<v8::Value> FileWrap::close()
{
    CHECK_CLOSED;
    API_PROLOGUE
    NEW_REQUEST("close", this);
    uv_fs_close(loop, &req->req_, fd_, on_close_callback);
    pending_requests_.push_back(req);
    is_closing_ = true;
    RET_PROMISE;
}

bool FileWrap::isClosed() const
{
    return closed_;
}

bool FileWrap::isClosing() const
{
    return is_closing_;
}

void on_read_callback(uv_fs_t *ptr)
{
    FILE_CALLBACK_PROLOGUE
    if (req->req_.result < 0)
        callback_reject_error_code(req, req->req_.result, req->syscall_);
    else
        req->Resolve(binder::to_v8(i, req->req_.result));
}

v8::Local<v8::Value> FileWrap::read(v8::Local<v8::Value> dst, int64_t dstOffset, size_t size, int64_t offset)
{
    CHECK_CLOSED;
    API_PROLOGUE

    Buffer *pBuffer = binder::Class<Buffer>::unwrap_object(i, dst);
    if (!pBuffer)
    {
        g_throw(TypeError, "Argument 'dst' must be a core.Buffer");
    }

    if (dstOffset + size - 1 > pBuffer->length())
    {
        g_throw(RangeError, "Invalid 'bufOffset' and 'size'");
    }

    NEW_REQUEST("preadv", this);
    req->buffer_ref_.Reset(i, dst);
    req->buffer_ = pBuffer;

    uv_buf_t buf{};
    buf.len = size;
    buf.base = reinterpret_cast<char*>(pBuffer->addressU8() + dstOffset);

    uv_fs_read(loop, &req->req_, fd_, &buf, 1, offset, on_read_callback);

    pending_requests_.push_back(req);
    RET_PROMISE;
}

void on_fstat_callback(uv_fs_t *ptr)
{
    FILE_CALLBACK_PROLOGUE
    if (req->req_.result < 0)
        callback_reject_error_code(req, req->req_.result, req->syscall_);
    else
        req->Resolve(make_stat_object(&req->req_.statbuf, i));
}

v8::Local<v8::Value> FileWrap::fstat()
{
    CHECK_CLOSED;
    API_PROLOGUE
    NEW_REQUEST("fstat", this);
    uv_fs_fstat(loop, &req->req_, fd_, on_fstat_callback);
    pending_requests_.push_back(req);
    RET_PROMISE;
}

void on_write_callback(uv_fs_t *ptr)
{
    FILE_CALLBACK_PROLOGUE
    if (req->req_.result < 0)
        callback_reject_error_code(req, req->req_.result, req->syscall_);
    else
        req->Resolve(binder::to_v8(i, req->req_.result));
}

v8::Local<v8::Value> FileWrap::write(v8::Local<v8::Value> src, int64_t srcOffset, size_t size, int64_t offset)
{
    CHECK_CLOSED;
    API_PROLOGUE

    Buffer *pBuffer = binder::Class<Buffer>::unwrap_object(i, src);
    if (!pBuffer)
    {
        g_throw(TypeError, "Argument 'src' must be a core.Buffer");
    }

    if (srcOffset + size - 1 > pBuffer->length())
    {
        g_throw(RangeError, "Invalid 'bufOffset' and 'size'");
    }

    NEW_REQUEST("write", this);
    req->buffer_ref_.Reset(i, src);
    req->buffer_ = pBuffer;

    uv_buf_t buf{};
    buf.len = size;
    buf.base = reinterpret_cast<char*>(pBuffer->addressU8() + srcOffset);
    uv_fs_write(loop, &req->req_, fd_, &buf, 1, offset, on_write_callback);

    pending_requests_.push_back(req);
    RET_PROMISE;
}

void on_file_undefined_promise_callback(uv_fs_t *ptr)
{
    FILE_CALLBACK_PROLOGUE
    if (req->req_.result < 0)
        callback_reject_error_code(req, req->req_.result, req->syscall_);
    else
        req->Resolve(v8::Undefined(i));
}

v8::Local<v8::Value> FileWrap::fsync()
{
    CHECK_CLOSED;
    API_PROLOGUE
    NEW_REQUEST("fsync", this);
    uv_fs_fsync(loop, &req->req_, fd_, on_file_undefined_promise_callback);
    pending_requests_.push_back(req);
    RET_PROMISE;
}

v8::Local<v8::Value> FileWrap::fdatasync()
{
    CHECK_CLOSED;
    API_PROLOGUE
    NEW_REQUEST("fdatasync", this);
    uv_fs_fdatasync(loop, &req->req_, fd_, on_file_undefined_promise_callback);
    pending_requests_.push_back(req);
    RET_PROMISE;
}

v8::Local<v8::Value> FileWrap::ftruncate(off_t length)
{
    CHECK_CLOSED;
    API_PROLOGUE
    NEW_REQUEST("ftruncate", this);
    uv_fs_ftruncate(loop, &req->req_, fd_, length, on_file_undefined_promise_callback);
    pending_requests_.push_back(req);
    RET_PROMISE;
}

v8::Local<v8::Value> FileWrap::fchmod(int32_t mode)
{
    CHECK_CLOSED;
    API_PROLOGUE
    NEW_REQUEST("fchmod", this);
    uv_fs_fchmod(loop, &req->req_, fd_, mode, on_file_undefined_promise_callback);
    pending_requests_.push_back(req);
    RET_PROMISE;
}

v8::Local<v8::Value> FileWrap::futime(double atime, double mtime)
{
    CHECK_CLOSED;
    API_PROLOGUE
    NEW_REQUEST("futime", this);
    uv_fs_futime(loop, &req->req_, fd_, atime, mtime, on_file_undefined_promise_callback);
    pending_requests_.push_back(req);
    RET_PROMISE;
}

v8::Local<v8::Value> FileWrap::fchown(uv_uid_t uid, uv_gid_t gid)
{
    CHECK_CLOSED;
    API_PROLOGUE
    NEW_REQUEST("fchown", this);
    uv_fs_fchown(loop, &req->req_, fd_, uid, gid, on_file_undefined_promise_callback);
    pending_requests_.push_back(req);
    RET_PROMISE;
}

GALLIUM_BINDINGS_NS_END
