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

#include "Core/Journal.h"

#include "Gallium/bindings/multimedia/ffwrappers/libavutil.h"
#include "Gallium/RuntimeBase.h"
#include "Gallium/bindings/multimedia/MediaInputOutput.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.bindings.multimedia.MediaInputOutput)

MediaIOContext::ContextOwner::~ContextOwner()
{
    if (!ctx)
        return;

    if (type == ContextType::kProtocol)
    {
        avio_closep(&ctx);
    }
    else if (type == ContextType::kDynamicMemory)
    {
        // Usually this type of context should not be released by destructor,
        // it only happens when the user didn't call `disposeMemStream()` manually.
        // In that case, we discard all the data and just destroy the buffer.
        uint8_t *buffer;
        // This function calls `avio_free_context()` internally.
        avio_close_dyn_buf(ctx, &buffer);
        ctx = nullptr;
    }
    else if (type == ContextType::kUserImpl)
    {
        // The buffer is allocated manually, and managed by ourself.
        // See `OpenFrom()` function below for more details.
        av_free(ctx->buffer);
        avio_context_free(&ctx);
    }
}

ffi::RetLocal<v8::Value> MediaIOContext::GetSupportedProtocols(bool output_proto)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    std::vector<v8::Local<v8::Value>> result;

    void *itr = nullptr;
    while (const char *name = avio_enum_protocols(&itr, output_proto))
        result.emplace_back(v8::String::NewFromUtf8(isolate, name).ToLocalChecked());

    return v8::Array::New(isolate, result.data(), result.size());
}

ffi::RetLocal<v8::Value> MediaIOContext::OpenURL(const std::string& url, int flags,
                                                 ffi::OptLocal<v8::Map> options,
                                                 ffi::OptLocal<v8::Function> interrupt_cb)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> jsctx = isolate->GetCurrentContext();

    std::unique_ptr<AVIOInterruptCB> av_intr_cb;
    AVDictionary *av_opt_dict = nullptr;

    auto owner = std::make_shared<ContextOwner>(isolate, ContextType::kProtocol);

    if (options)
    {
        v8::Local<v8::Map> map = *options;
        v8::Local<v8::Array> array = map->AsArray();
        CHECK(array->Length() == map->Size() * 2);

        size_t nb_entries = map->Size();
        for (size_t i = 0; i < nb_entries; i++)
        {
            v8::Local<v8::Value> key, value;
            if (!array->Get(jsctx, i * 2).ToLocal(&key) || !array->Get(jsctx, i * 2 + 1).ToLocal(&value))
                return ffi::FreePropagate();

            if (!key->IsString() || !value->IsString())
                return ffi::Fail(ffi::kErr, "keys and values of `options` must be string");

            // For `av_dict_set()`, NULL can be used as an empty dict.
            // It will create the initial dict automatically.
            av_dict_set(&av_opt_dict, *v8::String::Utf8Value(isolate, key),
                        *v8::String::Utf8Value(isolate, value), 0);
        }
    }

    if (interrupt_cb)
    {
        av_intr_cb = std::make_unique<AVIOInterruptCB>();
        av_intr_cb->opaque = owner.get();
        av_intr_cb->callback = +[](void *userdata) -> int {
            ContextOwner *context = static_cast<ContextOwner*>(userdata);
            CHECK(context);

            v8::Isolate *isolate = context->isolate;
            v8::HandleScope handle_scope(isolate);
            v8::Local<v8::Function> func = context->interrupt_cb.Get(isolate);
            v8::Local<v8::Context> jsctx = isolate->GetCurrentContext();

            v8::Local<v8::Value> result;
            v8::TryCatch try_catch(isolate);
            if (!func->Call(jsctx, v8::Null(isolate), 0, nullptr).ToLocal(&result))
                RuntimeBase::FromIsolate(isolate)->ReportUncaughtExceptionInCallback(try_catch);

            return result->ToBoolean(isolate)->Value();
        };
        owner->interrupt_cb.Reset(isolate, *interrupt_cb);
    }

    int err = avio_open2(&owner->ctx, url.c_str(), flags, av_intr_cb.get(), &av_opt_dict);
    if (av_opt_dict)
        av_dict_free(&av_opt_dict);
    if (err < 0)
        return ffi::Fail(ffi::kErr, fmt::format("failed to open: {}", av_err2str(err)));

    return ffi::JSObject::New<MediaIOContext>(isolate, std::move(owner));
}

ffi::RetLocal<v8::Value> MediaIOContext::OpenDynamicMemory()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto owner = std::make_shared<ContextOwner>(isolate, ContextType::kDynamicMemory);
    int err = avio_open_dyn_buf(&owner->ctx);
    if (err < 0)
        return ffi::Fail(ffi::kErr, fmt::format("failed to create dynamic memory stream: {}", av_err2str(err)));

    return ffi::JSObject::New<MediaIOContext>(isolate, std::move(owner));
}

namespace {

#define PREPARE_JS_ENV(func)                                        \
    CHECK(!owner->func.IsEmpty());                                  \
    v8::Isolate *isolate = owner->isolate;                          \
    v8::HandleScope handle_scope(isolate);                          \
    v8::Local<v8::Context> jsctx = isolate->GetCurrentContext();    \
    v8::Local<v8::Function> implfunc = owner->func.Get(isolate);    \
    v8::Local<v8::Object> thisobj = owner->user_impl_thisobj.Get(isolate);

#define CALL_READ_OR_WRITE_JS(jsfunc)                                                   \
    std::shared_ptr<v8::BackingStore> bs = v8::ArrayBuffer::NewBackingStore(            \
        const_cast<uint8_t*>(buf), buf_size, [](void*, size_t, void*) {}, nullptr);     \
    v8::Local<v8::Value> arrbuf = v8::ArrayBuffer::New(isolate, bs);                    \
    v8::Local<v8::Value> result;                                                        \
    v8::TryCatch try_catch(isolate);                                                    \
    bool error = !implfunc->Call(jsctx, thisobj, 1, &arrbuf).ToLocal(&result);          \
    arrbuf.As<v8::ArrayBuffer>()->Detach({}).Check();                                   \
    CHECK(bs.unique() && "buffer must not be shared with others");                      \
    if (error) {                                                                        \
        QLOG(LOG_ERROR, "user-implementation of `" #jsfunc "` thrown an exception");    \
        RuntimeBase::FromIsolate(isolate)->ReportUncaughtExceptionInCallback(try_catch);\
        return AVERROR_EXTERNAL;                                                        \
    }                                                                                   \
    if (!result->IsInt32()) {                                                           \
        QLOG(LOG_ERROR, "user-implementation of `" #jsfunc "` returned an invalid value"); \
        return AVERROR_EXTERNAL;                                                           \
    }

int on_read_packet_trampoline(void *opaque, uint8_t *buf, int buf_size)
{
    auto *owner = static_cast<MediaIOContext::ContextOwner*>(opaque);
    PREPARE_JS_ENV(read_packet_cb)
    CALL_READ_OR_WRITE_JS(onReadPacket)

    int32_t res_i32 = result->ToInt32(jsctx).ToLocalChecked()->Value();
    if (res_i32 < 0)
        return AVERROR_EXTERNAL;
    if (res_i32 == 0)
        return AVERROR_EOF;

    return res_i32;
}

int on_write_packet_trampoline(void *opaque, const uint8_t *buf, int buf_size)
{
    auto *owner = static_cast<MediaIOContext::ContextOwner*>(opaque);
    PREPARE_JS_ENV(write_packet_cb)
    CALL_READ_OR_WRITE_JS(onWritePacket)

    int32_t res_i32 = result->ToInt32(jsctx).ToLocalChecked()->Value();
    if (res_i32 < 0)
        return AVERROR_EXTERNAL;

    return res_i32;
}

int64_t on_seek_trampoline(void *opaque, int64_t offset, int whence)
{
    auto *owner = static_cast<MediaIOContext::ContextOwner*>(opaque);
    PREPARE_JS_ENV(seek_cb)

    v8::TryCatch try_catch(isolate);
    v8::Local<v8::Value> result;
    v8::Local<v8::Value> args[]{
        ffi::Cast<ffi::PreciseI64>::ToChecked(isolate, offset),
        v8::Int32::New(isolate, whence)
    };
    if (!implfunc->Call(jsctx, thisobj, 2, args).ToLocal(&result))
    {
        QLOG(LOG_ERROR, "user-implementation of `onSeek` thrown an exception");
        RuntimeBase::FromIsolate(isolate)->ReportUncaughtExceptionInCallback(try_catch);
        return AVERROR_EXTERNAL;
    }

    auto i64result = ffi::Cast<ffi::PreciseI64>::From(isolate, result);
    if (!i64result)
    {
        QLOG(LOG_ERROR, "user-implementation of `onSeek` returned an invalid value");
        return AVERROR_EXTERNAL;
    }

    return **i64result < 0 ? AVERROR_EXTERNAL : **i64result;
};

} // namespace anonymous

ffi::RetLocal<v8::Value> MediaIOContext::OpenFrom(const ffi::IFace<MediaIOBackend>& backend)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto owner = std::make_shared<ContextOwner>(isolate, ContextType::kUserImpl);
    bool writable = backend->writable && *backend->writable;
    if (writable && !backend->on_write_packet)
        return ffi::Fail(ffi::kErr, "`onWritePacket` must be provided for writable stream");

    if (!writable && !backend->on_read_packet)
        return ffi::Fail(ffi::kErr, "`onReadPacket` must be provided for readable stream");

    // A typical size, a cache page 4KB.
    constexpr size_t kDefaultBufSize = 4096 * 1024;
    size_t buffer_size = backend->buffer_size_hint ? *backend->buffer_size_hint : kDefaultBufSize;
    CHECK(buffer_size <= INT32_MAX);

    if (backend->on_read_packet)
        owner->read_packet_cb.Reset(isolate, *backend->on_read_packet);
    if (backend->on_write_packet)
        owner->write_packet_cb.Reset(isolate, *backend->on_write_packet);
    if (backend->on_seek)
        owner->seek_cb.Reset(isolate, *backend->on_seek);

    owner->user_impl_thisobj.Reset(isolate, backend.Object());

    uint8_t *buffer = static_cast<uint8_t*>(av_malloc(buffer_size));
    owner->ctx = avio_alloc_context(
            buffer,
            static_cast<int>(buffer_size),
            writable,
            owner.get(),
            backend->on_read_packet ? on_read_packet_trampoline : nullptr,
            backend->on_write_packet ? on_write_packet_trampoline : nullptr,
            backend->on_seek ? on_seek_trampoline : nullptr
    );
    if (!owner->ctx)
    {
        av_free(buffer);
        return ffi::Fail(ffi::kErr, "failed to allocate the context");
    }

    return ffi::JSObject::New<MediaIOContext>(isolate, std::move(owner));
}

ffi::Ret<bool> MediaIOContext::isLocked()
{
    return !context_.unique();
}

ffi::Ret<void> MediaIOContext::dispose()
{
    // Delete a reference to the context, but the context may still be owned
    // by other instances. In that case, the context keeps alive but is not
    // touchable in JavaScript anymore, as the `MediaIOContext` instance is
    // disposed.
    context_.reset();
    NotifyDisposeState(DisposeState::kDisposed);
    return {};
}

#define CHECK_LOCK_STATE \
    if (!context_.unique()) { return ffi::Fail(ffi::kErr, "operation not permitted: locked context"); }

ffi::RetLocal<v8::Value> MediaIOContext::disposeMemoryDynamic()
{
    CHECK_LOCK_STATE
    if (context_->type != ContextType::kDynamicMemory)
        return ffi::Fail(ffi::kErr, "this type of context does not support `disposeMemStream()`");

    uint8_t *address;
    size_t size = avio_get_dyn_buf(context_->ctx, &address);
    NotifyDisposeState(DisposeState::kDisposed);

    auto deleter = +[](void *data, size_t, void*) {
        av_free(data);
    };
    auto bs = v8::ArrayBuffer::NewBackingStore(address, size, deleter, nullptr);
    CHECK(bs);

    return v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), std::move(bs));
}

ffi::Ret<int32_t> MediaIOContext::read(const ffi::Mem<uint8_t>& dst)
{
    CHECK_LOCK_STATE
    if (context_->ctx->write_flag)
        return ffi::Fail(ffi::kErr, "context is not readable");
    int result = avio_read(context_->ctx, dst.Address(), static_cast<int>(dst.ByteSize()));
    if (result < 0)
        return ffi::Fail(ffi::kErr, fmt::format("failed to read: {}", av_err2str(result)));
    return result;
}

ffi::Ret<int32_t> MediaIOContext::readPartial(const ffi::Mem<uint8_t>& dst)
{
    CHECK_LOCK_STATE
    if (context_->ctx->write_flag)
        return ffi::Fail(ffi::kErr, "context is not readable");
    int result = avio_read_partial(context_->ctx, dst.Address(), static_cast<int>(dst.ByteSize()));
    if (result < 0)
        return ffi::Fail(ffi::kErr, fmt::format("failed to read: {}", av_err2str(result)));
    return result;
}

ffi::Ret<void> MediaIOContext::write(const ffi::Mem<uint8_t>& src)
{
    CHECK_LOCK_STATE
    if (!context_->ctx->write_flag)
        return ffi::Fail(ffi::kErr, "context is not writable");
    avio_write(context_->ctx, src.Address(), static_cast<int>(src.ByteSize()));
    return {};
}

ffi::Ret<ffi::PreciseI64> MediaIOContext::seek(ffi::PreciseI64 offset, ffi::Enum<SeekWhence> whence)
{
    CHECK_LOCK_STATE
    int64_t result = avio_seek(context_->ctx, *offset, whence.GetInteger());
    if (result < 0)
        return ffi::Fail(ffi::kErr, fmt::format("failed to seek: {}", av_err2str(result)));
    return result;
}

ffi::Ret<void> MediaIOContext::flush()
{
    CHECK_LOCK_STATE
    avio_flush(context_->ctx);
    return {};
}

ffi::Ret<ffi::PreciseU64> MediaIOContext::getSizeInBytes()
{
    CHECK_LOCK_STATE
    int64_t size = avio_size(context_->ctx);
    if (size < 0)
        return ffi::Fail(ffi::kErr, fmt::format("failed to measure size: {}", av_err2str(size)));
    return size;
}

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
