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

#include "Core/Errors.h"
#include "Core/Data.h"
#include "Core/HuffmanCodec.h"

#include "Glamor/Moe/MoeCodeCompressor.h"
#include "Glamor/Moe/MoeOpcodeRenpyInterface.h"
GLAMOR_NAMESPACE_BEGIN

namespace {

int32_t get_inst_size_by_opcode(uint8_t opcode)
{
    if (opcode > opcode::g_opcode_vector_tbl_count)
        return -1;
    return opcode::g_opcode_vector_tbl[opcode - 1].fixed_size;
}

struct BufferCopyRange
{
    MoeByteStreamReader::Address start_addr;
    off_t length;
};

Shared<Data> copy_instruction_buffers(const Unique<MoeByteStreamReader>& reader)
{
    std::vector<BufferCopyRange> copy_ranges;
    size_t total_size_bytes = 0;

    // First pass, compute the final size of target buffer
    int32_t next_verb_offset = 0;
    while (true)
    {
        if (next_verb_offset == reader->GetReadOffsetInBuffer())
        {
            auto v = reader->PeekNext<uint16_t>();

            if (v == opcode::kSwitchNextBuffer || v == opcode::kCommandPoolEnd)
            {
                copy_ranges.push_back(BufferCopyRange {
                    .start_addr = reader->GetBufferByIndex(reader->GetBufferIndex())->GetStartAddress(),
                    .length = reader->GetReadOffsetInBuffer()
                });
                total_size_bytes += reader->GetReadOffsetInBuffer();
            }

            if (v == opcode::kSwitchNextBuffer)
            {
                reader->MoveToNextBuffer();
                next_verb_offset = 0;
                continue;
            }
            else if (v == opcode::kCommandPoolEnd)
            {
                break;
            }
            int32_t fixed_size = get_inst_size_by_opcode(v);
            if (fixed_size < 0)
                return nullptr;

            next_verb_offset += fixed_size;
        }
        reader->SwallowNext<uint8_t>();
    }

    // Second pass, copy data
    auto data = Data::MakeFromSize(total_size_bytes);
    for (const auto& range : copy_ranges)
    {
        data->write(range.start_addr, range.length);
    }

    return data;
}

} // namespace anonymous

Shared<Data> MoeCodeCompressor::Compress(Unique<MoeByteStreamReader> reader)
{
    Shared<Data> instructions_data = copy_instruction_buffers(reader);
    return HuffmanEncode(instructions_data);
}

GLAMOR_NAMESPACE_END
