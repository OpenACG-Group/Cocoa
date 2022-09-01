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

#include <optional>

#include "include/codec/SkCodec.h"

#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRegion.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkM44.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkBlender.h"
#include "include/core/SkShader.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPathEffect.h"

#include "include/effects/SkBlenders.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/effects/SkImageFilters.h"

#include "Core/Errors.h"
#include "Core/Exception.h"
#include "Glamor/Glamor.h"
#include "Glamor/Moe/MoeInterpreterEngine.h"
#include "Glamor/Moe/MoeOpcodeRenpyInterface.h"
GLAMOR_NAMESPACE_BEGIN

MoeInterpreterEngine::MoeInterpreterEngine(Unique<MoeByteStreamReader> reader)
    : stream_reader_(std::move(reader))
    , heap_()
{
}

MoeInterpreterEngine::~MoeInterpreterEngine() = default;

namespace {

#define VERB_OPCODE(x)      ((x) & 0xff)
#define VERB_N_ARGS(x)      ((x) >> 8)

struct ExecutionContext
{
    MoeHeap& heap;
    SkCanvas *canvas{nullptr};
};

void throw_error(const std::string_view& what)
{
    throw RuntimeException(__func__, fmt::format("Moe Interpreter: {}", what));
}

void ir_check_has_valid_canvas(SkCanvas *canvas)
{
    if (!canvas)
        throw_error("No valid canvas exists in current IR context");
}

#define IR_CHECK_CANVAS ir_check_has_valid_canvas(canvas)

} // namespace anonymous

#include <RenpyMoeOpcodeCodeGen.hpp>

namespace {
std::optional<SkRect> probe_whether_ir_requires_canvas(MoeByteStreamReader& reader)
{
    auto verb = reader.PeekNext<uint16_t>();
    if (VERB_OPCODE(verb) != opcode::kDrawBounds)
        return {};
    reader.SwallowNext<uint16_t>();

    auto width = reader.ExtractNext<float>();
    auto height = reader.ExtractNext<float>();

    if (width <= 0 || height <= 0)
        throw RuntimeException(__func__, "Annotation [DrawBounds] requires 2 positive f32 numbers");

    return SkRect::MakeWH(width, height);
}
} // namespace anonymous

sk_sp<SkPicture> MoeInterpreterEngine::PerformInterpret()
{
    SkPictureRecorder recorder;
    SkCanvas *canvas = nullptr;

    if (auto maybeRect = probe_whether_ir_requires_canvas(*stream_reader_))
    {
        canvas = recorder.beginRecording(*maybeRect);
        if (!canvas)
            throw_error("Failed in creating SkPicture recorder");
    }

    ExecutionContext context{
        .heap = heap_,
        .canvas = canvas
    };

    opcode::Dispatch(*stream_reader_, context);

    if (canvas)
        return recorder.finishRecordingAsPicture();
    return nullptr;
}

void MoeInterpreterEngine::GetLastHeapProfile(MoeHeap::Profile& out)
{
    heap_.ProfileResult(out);
}

GLAMOR_NAMESPACE_END
