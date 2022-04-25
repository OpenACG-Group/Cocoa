#include <vector>
#include <queue>

#include "Core/EventLoop.h"
#include "Gallium/bindings/cobalt/Exports.h"
#include "Gallium/bindings/core/Exports.h"
#include "Gallium/binder/Class.h"

#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
GALLIUM_BINDINGS_COBALT_NS_BEGIN

#define VERB_OPCODE(x)      ((x) & 0xff)
#define VERB_N_ARGS(x)      ((x) >> 8)

namespace opcode {
enum {
    kSwitchNextBuffer       = 0x02,
    kCommandPoolEnd         = 0x03,
    kDrawBounds             = 0x04
};

using sbyte = int8_t;
using byte = uint8_t;
using word = int16_t;
using uword = uint16_t;
using dwrod = int32_t;
using udword = uint32_t;
using qword = int64_t;
using uqword = uint64_t;
using fdword = float;
}

class IRStreamReader
{
public:
    explicit IRStreamReader(std::vector<Buffer*>&& buffers)
        : buffers_(buffers)
        , buffer_index_(0)
        , buffer_rdptr_(nullptr)
        , buffer_endptr_(nullptr)
    {
        buffer_rdptr_ = buffers_[0]->getWriteableDataPointerByte();
        buffer_endptr_ = buffer_rdptr_ + buffers[0]->length();
    }

    template<typename T>
    T extractNext()
    {
        if (buffer_rdptr_ + sizeof(T) > buffer_endptr_)
            g_throw(Error, "Corrupted serialize IR buffer: reading out of buffer");
        buffer_rdptr_ += sizeof(T);
        return *reinterpret_cast<T*>(buffer_rdptr_ - sizeof(T));
    }

    // NOLINTNEXTLINE
    uint16_t fetchNextVerb()
    {
        if (buffer_rdptr_ >= buffer_endptr_)
            g_throw(Error, "Corrupted serialized IR buffer");

        auto verb = extractNext<uint16_t>();
        if (VERB_OPCODE(verb) == opcode::kSwitchNextBuffer)
        {
            switchNextBuffer();
            // TODO: Eliminate this recursive chain
            return fetchNextVerb();
        }
        return verb;
    }

private:
    void switchNextBuffer()
    {
        buffer_index_++;
        if (buffer_index_ >= buffers_.size())
            g_throw(Error, "[SwitchNextBuffer] direct us to an invalid buffer");
        buffer_rdptr_ = buffers_[buffer_index_]->getWriteableDataPointerByte();
        buffer_endptr_ = buffer_rdptr_ + buffers_[buffer_index_]->length();
    }

    std::vector<Buffer*>    buffers_;
    int32_t                 buffer_index_;
    uint8_t                *buffer_rdptr_;
    uint8_t                *buffer_endptr_;
};

v8::Local<v8::Value> VGIRCompilerWrap::Compile(v8::Local<v8::Value> array)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto values = binder::from_v8<std::vector<v8::Local<v8::Value>>>(isolate, array);
    std::vector<Buffer*> unwrapped(values.size());

    for (uint32_t i = 0; i < values.size(); i++)
    {
        Buffer *ptr = binder::Class<Buffer>::unwrap_object(isolate, values[i]);
        if (ptr == nullptr)
            g_throw(TypeError, "Expecting an array containing instances of core.Buffer type");
        unwrapped[i] = ptr;
    }

    IRStreamReader reader(std::move(unwrapped));

    uint16_t verb = reader.fetchNextVerb();
    if (VERB_OPCODE(verb) != opcode::kDrawBounds || VERB_N_ARGS(verb) != 2)
        g_throw(Error, "Requires a [DrawBounds] as the first command");

    SkRect drawBounds = SkRect::MakeWH(reader.extractNext<opcode::fdword>(),
                                       reader.extractNext<opcode::fdword>());
    if (drawBounds.width() <= 0 || drawBounds.height() <= 0)
        g_throw(Error, "Command [DrawBounds] requires 2 positive <fdword> numbers");

    SkPictureRecorder recorder;
    SkCanvas *canvas = recorder.beginRecording(drawBounds, SkRTreeFactory()());
    CHECK(canvas);

    while (true)
    {
        verb = reader.fetchNextVerb();
        if (VERB_OPCODE(verb) == opcode::kCommandPoolEnd)
            break;

        fmt::print("opcode: 0x{:02x}, args={}\n", VERB_OPCODE(verb), VERB_N_ARGS(verb));
    }

    sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();

    return v8::Undefined(isolate);
}

GALLIUM_BINDINGS_COBALT_NS_END
