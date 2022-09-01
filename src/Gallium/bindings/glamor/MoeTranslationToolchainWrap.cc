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

#include <vector>
#include <variant>
#include <unordered_map>
#include <sstream>

#include "Core/Errors.h"
#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/core/Exports.h"
#include "Gallium/binder/Class.h"
#include "Gallium/binder/CallV8.h"

#include "Glamor/Moe/MoeInterpreterEngine.h"
#include "Glamor/Moe/MoeCodeDisassembler.h"
#include "Glamor/Moe/MoeByteStreamReader.h"
#include "Glamor/Moe/MoeCodeCompressor.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

class JSMoeCodeHolder : public glamor::MoeCodeHolder
{
public:
    explicit JSMoeCodeHolder(Buffer *buffer) : buffer_(buffer) {}
    ~JSMoeCodeHolder() override = default;

    const uint8_t * GetStartAddress() override {
        return buffer_->addressU8();
    }

    size_t GetLength() override {
        return buffer_->length();
    }

private:
    Buffer  *buffer_;
};

glamor::MoeByteStreamReader::CodeHolderVector get_ir_stream_reader(v8::Local<v8::Value> array)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto values = binder::from_v8<std::vector<v8::Local<v8::Value>>>(isolate, array);
    glamor::MoeByteStreamReader::CodeHolderVector unwrapped(values.size());

    for (uint32_t i = 0; i < values.size(); i++)
    {
        Buffer *ptr = binder::Class<Buffer>::unwrap_object(isolate, values[i]);
        if (ptr == nullptr)
            g_throw(TypeError, "Expecting an array containing instances of core.Buffer type");
        unwrapped[i] = std::make_unique<JSMoeCodeHolder>(ptr);
    }

    return unwrapped;
}

void heap_load_bound_objects(v8::Isolate *isolate,
                             glamor::MoeInterpreterEngine& engine,
                             MoeHeapObjectBinderWrap *binder)
{
    for (const auto& pair : binder->getBoundObjects())
    {
        v8::Local<v8::Value> obj = pair.second.second.Get(isolate);
        switch (pair.second.first)
        {
        case MoeHeapObjectBinderWrap::Type::kBitmap:
        {
            CkBitmapWrap *w = binder::Class<CkBitmapWrap>::unwrap_object(isolate, obj);
            CHECK(w);
            engine.LoadObjectToHeap(pair.first, w->getBitmap());
            break;
        }

        case MoeHeapObjectBinderWrap::Type::kImage:
        {
            CkImageWrap *w = binder::Class<CkImageWrap>::unwrap_object(isolate, obj);
            CHECK(w);
            engine.LoadObjectToHeap(pair.first, w->getImage());
            break;
        }

        case MoeHeapObjectBinderWrap::Type::kPicture:
        {
            CkPictureWrap *w = binder::Class<CkPictureWrap>::unwrap_object(isolate, obj);
            CHECK(w);
            engine.LoadObjectToHeap(pair.first, w->getPicture());
            break;
        }

        case MoeHeapObjectBinderWrap::Type::kString:
        {
            if (!obj->IsString())
                g_throw(TypeError, fmt::format("Failed to load bound heap object #{} as string", pair.first));
            v8::Local<v8::String> string = obj.As<v8::String>();
            v8::String::Utf8Value utf8Value(isolate, string);
            engine.LoadObjectToHeap(pair.first, SkString(*utf8Value, utf8Value.length()));
            break;
        }
        }
    }
}

v8::Local<v8::Value> MoeTranslationToolchainWrap::Interpreter(v8::Local<v8::Value> array,
                                                              v8::Local<v8::Value> binderObject,
                                                              bool heapProfiling)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    MoeHeapObjectBinderWrap *binder = nullptr;
    if (!binderObject->IsNullOrUndefined())
    {
        binder = binder::Class<MoeHeapObjectBinderWrap>::unwrap_object(isolate, binderObject);
        if (binder == nullptr)
            g_throw(TypeError, "'binder' must be an instance of MoeHeapObjectBinder");
    }

    glamor::MoeByteStreamReader::CodeHolderVector codeHolderVector = get_ir_stream_reader(array);
    glamor::MoeInterpreterEngine engine(
            std::make_unique<glamor::MoeByteStreamReader>(std::move(codeHolderVector)));

    if (binder)
        heap_load_bound_objects(isolate, engine, binder);

    sk_sp<SkPicture> picture = engine.PerformInterpret();

    std::map<std::string_view, v8::Local<v8::Value>> result;
    if (picture)
        result["artifact"] = binder::Class<CkPictureWrap>::create_object(isolate, picture);

    if (heapProfiling)
    {
        glamor::MoeHeap::Profile prof{};
        engine.GetLastHeapProfile(prof);

        std::map<std::string_view, uint32_t> profiling{
            { "heapSingleCellSize", prof.heap_cell_size },
            { "heapTotalSize", prof.heap_total_size },
            { "heapAllocationsCount", prof.allocation_count },
            { "heapExtractionsCount", prof.extraction_count },
            { "heapLeakedCellsCount", prof.leaked_cells }
        };

        result["heapProfiling"] = binder::to_v8(isolate, profiling);
    }

    return binder::to_v8(isolate, result);
}

v8::Local<v8::Value> MoeTranslationToolchainWrap::Disassemble(v8::Local<v8::Value> array)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    glamor::MoeByteStreamReader::CodeHolderVector codeHolderVector = get_ir_stream_reader(array);
    auto str = glamor::MoeCodeDisassembler::Disassemble(
            std::make_unique<glamor::MoeByteStreamReader>(std::move(codeHolderVector)));

    return binder::to_v8(isolate, str);
}

v8::Local<v8::Value> MoeTranslationToolchainWrap::Compress(v8::Local<v8::Value> array)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    glamor::MoeByteStreamReader::CodeHolderVector codeHolderVector = get_ir_stream_reader(array);
    auto reader = std::make_unique<glamor::MoeByteStreamReader>(std::move(codeHolderVector));

    glamor::MoeCodeCompressor::Compress(std::move(reader));

    // TODO: implement this.
    return v8::Undefined(isolate);
}

namespace {

using Binder = MoeHeapObjectBinderWrap;
template<typename T, Binder::Type S>
void binder_set_bound_object(Binder::ObjectMap& map, uint32_t key, v8::Local<v8::Value> object,
                             const std::string_view& classname)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (map.count(key) > 0)
        map.erase(key);
    if (!binder::Class<T>::unwrap_object(isolate, object))
    {
        g_throw(TypeError, fmt::format("'object' must be an instance of {}", classname));
    }
    map.try_emplace(key, S, v8::Global<v8::Value>(isolate, object));
}

} // namespace anonymous

void MoeHeapObjectBinderWrap::bindString(uint32_t key, v8::Local<v8::Value> string)
{
    if (bound_objects_.count(key) > 0)
        bound_objects_.erase(key);

    if (!string->IsString())
        g_throw(TypeError, "'string' must be a string");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    bound_objects_.try_emplace(key, Type::kString, v8::Global<v8::Value>(isolate, string));
}

void MoeHeapObjectBinderWrap::bindBitmap(uint32_t key, v8::Local<v8::Value> bitmap)
{
    binder_set_bound_object<CkBitmapWrap, Type::kBitmap>(bound_objects_, key,
                                                         bitmap, "CkBitmapWrap");
}

void MoeHeapObjectBinderWrap::bindImage(uint32_t key, v8::Local<v8::Value> image)
{
    binder_set_bound_object<CkImageWrap, Type::kImage>(bound_objects_, key,
                                                       image, "CkImage");
}

void MoeHeapObjectBinderWrap::bindPicture(uint32_t key, v8::Local<v8::Value> picture)
{
    binder_set_bound_object<CkPictureWrap, Type::kPicture>(bound_objects_, key,
                                                           picture, "CkPicture");
}

GALLIUM_BINDINGS_GLAMOR_NS_END
