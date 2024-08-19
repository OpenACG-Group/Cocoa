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

#ifndef COCOA_GALLIUM_BINDINGS_MULTIMEDIA_MEDIAINPUTOUTPUT_H
#define COCOA_GALLIUM_BINDINGS_MULTIMEDIA_MEDIAINPUTOUTPUT_H

#include "Gallium/bindings/multimedia/ffwrappers/libavformat-avio.h"

#include "Gallium/bindings/multimedia/Types.h"
#include "Gallium/ffi/DataTypes.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/ArrayBuffer.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/Interface.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

enum class SeekWhence
{
    kSet = SEEK_SET,
    kCur = SEEK_CUR,
    kEnd = SEEK_END
};

enum class MediaIOFlags
{
    kRead = AVIO_FLAG_READ,
    kWrite = AVIO_FLAG_WRITE,
    kReadWrite = AVIO_FLAG_READ_WRITE,
    kDirect = AVIO_FLAG_DIRECT,
    kNonBlock = AVIO_FLAG_NONBLOCK
};

//! TSDecl: @interface MediaIOBackend
struct MediaIOBackend
{
    //! @tsdocbegin
    //! Specify the type of the context. Creates a write stream if true;
    //! otherwise, creates a read stream.
    //! @tsdocend
    //! TSDecl: @property @optional writable: boolean
    ffi::Opt<bool> writable;

    //! TSDecl: @property @optional bufferSizeHint: i32
    ffi::Opt<size_t> buffer_size_hint;

    //! @tsdocbegin
    //! A callback to read some data to refill the given buffer `buffer`, may be absent
    //! if the `MediaIOContext` will not be used as a readable stream.
    //! Returns the number of bytes read, 0 if EOF, and any value < 0 indicates an error.
    //! Exceptions are swallowed.
    //!
    //! Note that `buffer` is ONLY available in the scope of your callback. Once your
    //! callback returns, `buffer` will be detached. Never pass it to other places.
    //! @tsdocend
    //! TSDecl: @property @optional onReadPacket: @fn(i32, buffer: @mem(raw))
    ffi::OptLocal<v8::Function> on_read_packet;

    //! @tsdocbegin
    //! A callback to write data in the given buffer `buffer` to destination, may be absent
    //! if the `writable` property is absent or false.
    //! Returns the number of bytes written, any value < 0 indicates an error.
    //! Exceptions are swallowed.
    //!
    //! Note that `buffer` is ONLY available in the scope of your callback. Once your
    //! callback returns, `buffer` will be detached. Never pass it to other places.
    //! @tsdocend
    //! TSDecl: @property @optional onWritePacket: @fn(i32, buffer: @mem(raw))
    ffi::OptLocal<v8::Function> on_write_packet;

    //! @tsdocbegin
    //! A callback to seek to specified byte position, may be absent if the underlying implementation
    //! is not seekable.
    //! Returns the position measured in bytes from the beginning of the stream, any value < 0
    //! indicates an error. Exceptions are swallowed.
    //! @tsdocend
    //! TSDecl: @property @optional onSeek: @fn(bigint, offset: bigint, whence: SeekWhence)
    ffi::OptLocal<v8::Function> on_seek;
};
//! TSDecl: @end

//! @tsdocbegin
//! `MediaIOContext` is a standard interface for other multimedia interfaces to
//! read and write binary data as bytestream. This API is synchronous.
//! The context is either readable or writable, never duplex.
//!
//! The context can be backed by the native implementation or user's implementation
//! in JavaScript. Once the context is occupied by a consumer (like `AVDecoder`) or
//! producer (like `AVEncoder`), it becomes locked, and any operation from JavaScript,
//! except `dispose()` and `isLocked()`, will throw an exception, until the consumer
//! or producer unlock it.
//! @tsdocend
//! TSDecl: @class @nonconstructible MediaIOContext
class MediaIOContext : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    enum ContextType
    {
        // context opened by `OpenURL()`
        kProtocol,
        // context opened by `OpenWritableMem()`
        kDynamicMemory,
        // context opened by `OpenFrom()`
        kUserImpl
    };

    struct ContextOwner
    {
        ContextOwner(v8::Isolate *isolate_, ContextType type_)
            : type(type_), isolate(isolate_), ctx(nullptr) {}
        ~ContextOwner();

        ContextType type;

        v8::Isolate *isolate;
        AVIOContext *ctx;

        // For protocol context only.
        v8::Global<v8::Function> interrupt_cb;
        // For user-implemented context only.
        v8::Global<v8::Object> user_impl_thisobj;
        v8::Global<v8::Function> read_packet_cb;
        v8::Global<v8::Function> write_packet_cb;
        v8::Global<v8::Function> seek_cb;
    };

    explicit MediaIOContext(std::shared_ptr<ContextOwner> ctx)
        : context_(std::move(ctx)) {}
    ~MediaIOContext() override = default;

    g_nodiscard std::shared_ptr<ContextOwner> GetAndLockContext() const {
        return context_;
    }

    g_nodiscard bool IsLocked() const {
        CHECK(context_ && "disposed context");
        return !context_.unique();
    }

    //! @tsdocbegin
    //! Returns all the supported protocols that can be used in `OpenURL()`.
    //!
    //! @param outputProto  Get output protocols if true; otherwise, get input protocols.
    //! @tsdocend
    //! TSDecl: @method @static GetSupportedProtocols(outputProto: boolean): @array(string)
    static ffi::RetLocal<v8::Value> GetSupportedProtocols(bool output_proto);

    //! @tsdocbegin
    //! Opens a context for accessing the resource indicated by the given URL.
    //! URL may be a local file, a network media, or something else supported.
    //! Note that if URL is opened in read+write mode, the context ONLY can be used to write data.
    //!
    //! @param url          URL of the resource
    //! @param flags        Flags to enable or disable some features
    //! @param options      A dictionary filled with protocol-private options.
    //!                     Invalid options are ignored.
    //! @param interruptCb  An interrupt callback, may be null, to be used at the protocols level.
    //!                     During blocking operations, if the callback returns true, the blocking
    //!                     operation will be aborted.
    //! @tsdocend
    //! TSDecl: @method @static OpenURL(url: string, flags: MediaIOFlags,
    //! TSDecl:                         options: @union(null, @generic(Map, string, string)),
    //! TSDecl:                         interruptCb: @union(null, @fn(boolean))): MediaIOContext
    static ffi::RetLocal<v8::Value> OpenURL(const std::string& url,
                                            int flags,
                                            ffi::OptLocal<v8::Map> options,
                                            ffi::OptLocal<v8::Function> interrupt_cb);

    //! @tsdocbegin
    //! Opens a context backed by a dynamic (resizable) memory stream, write-only.
    //! Written data will be stored in a memory buffer.
    //! Both `dispose()` and `disposeMemoryDynamic()` can be used to release the context,
    //! but the latter returns an `ArrayBuffer` that contains the written data.
    //! @tsdocend
    //! TSDecl: @method @static OpenDynamicMemory(): MediaIOContext
    static ffi::RetLocal<v8::Value> OpenDynamicMemory();

    //! @tsdocbegin
    //! Open a context backed by a user-implemented backend.
    //! See `MediaIOBackend` for more details.
    //! @tsdocend
    //! TSDecl: @method @static OpenFrom(backend: MediaIOBackend): MediaIOContext
    static ffi::RetLocal<v8::Value> OpenFrom(const ffi::IFace<MediaIOBackend>& backend);

    //! TSDecl: @method isLocked(): boolean
    ffi::Ret<bool> isLocked();

    //! TSDecl: @property @readonly writable: boolean
    ffi::Ret<bool> getWritable() {
        return context_->ctx->write_flag;
    }

    //! TSDecl: @property @readonly readable: boolean
    ffi::Ret<bool> getReadable() {
        return !context_->ctx->write_flag;
    }

    //! @tsdocbegin
    //! Free the IO context and all the resources associated with it.
    //! Once the context is disposed, any operation will throw an exception.
    //! If the context is locked, disposing of underlying resource will be delayed until
    //! the context is unlocked, but the `MediaIOContext` instance still behaves like a
    //! disposed instance.
    //! @tsdocend
    //! TSDecl: @method dispose(): void
    ffi::Ret<void> dispose();

    //! @tsdocbegin
    //! Like `dispose()`, free the IO context and all the resources associated with it.
    //! But it is for context opened by `OpenDynamicMemory()` only, throwing an exception
    //! when called on other type of contexts.
    //!
    //! Unlike `dispose()`, this must NOT be called on a locked context.
    //!
    //! @returns An `ArrayBuffer` that contains the written data.
    //! @tsdocend
    //! TSDecl: @method disposeMemoryDynamic(): @mem(raw)
    ffi::RetLocal<v8::Value> disposeMemoryDynamic();

    //! @tsdocbegin
    //! Write specified data into the stream.
    //! Throws an exception when the context is not writable, or an IO error occurred.
    //! Length of `src` must be not larger than INT32_MAX.
    //! @tsdocend
    //! TSDecl: @method write(src: @mem(u8)): void
    ffi::Ret<void> write(const ffi::Mem<uint8_t>& src);

    //! @tsdocbegin
    //! Read no more than `dst.byteLength` bytes into `dst` buffer.
    //! Always refill `dst` completely unless the stream reaches end (EOF).
    //! Throws an exception when the context is not readable, or an IO error occurred.
    //! Length of `dst` must be not larger than INT32_MAX.
    //!
    //! @returns Number of bytes read.
    //! @tsdocend
    //! TSDecl: @method read(dst: @mem(u8)): i32
    ffi::Ret<int32_t> read(const ffi::Mem<uint8_t>& dst);

    //! @tsdocbegin
    //! Read no more than `dst.byteLength` bytes into `dst` buffer.
    //! Unlike `read()`, this method allows reading fewer bytes than requested,
    //! even though the stream does not reach its end. The missing bytes can be read
    //! in the next call, and at least 1 byte is read in each call.
    //! Useful to reduce latency in certain cases.
    //! Length of `dst` must be not larger than INT32_MAX.
    //!
    //! @returns Number of bytes read.
    //! @tsdocend
    //! TSDecl: @method readPartial(dst: @mem(u8)): i32
    ffi::Ret<int32_t> readPartial(const ffi::Mem<uint8_t>& dst);

    //! @tsdocbegin
    //! Sets the current position of stream to the specified `offset`.
    //! Throws if an error occurs.
    //!
    //! @returns The new position from the beginning of stream, measured in bytes.
    //! @tsdocend
    //! TSDecl: @method seek(offset: bigint, whence: SeekWhence): bigint
    ffi::Ret<ffi::PreciseI64> seek(ffi::PreciseI64 offset, ffi::Enum<SeekWhence> whence);

    //! @tsdocbegin
    //! Force flushing of buffered data.
    //!
    //! For write streams, force the buffered data to be immediately written to the output,
    //! without waiting to fill the internal buffer.
    //!
    //! For read streams, discard all currently buffered data, and advance the
    //! reported file position to that of the underlying stream. This does not
    //! read new data, and does not perform any seeks.
    //! @tsdocend
    //! TSDecl: @method flush(): void
    ffi::Ret<void> flush();

    //! @tsdocbegin
    //! Gets the size of stream in bytes. For write streams, size is updated each
    //! time a successful writeout ends up further position-wise.
    //! Throws an exception if the size is unmeasurable.
    //! @tsdocend
    //! TSDecl: @property @readonly sizeInBytes: bigint
    ffi::Ret<ffi::PreciseU64> getSizeInBytes();

private:
    std::shared_ptr<ContextOwner> context_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
#endif //COCOA_GALLIUM_BINDINGS_MULTIMEDIA_MEDIAINPUTOUTPUT_H
