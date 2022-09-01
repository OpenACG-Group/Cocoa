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

#include <sstream>

#include "fmt/format.h"

#include "Core/Exception.h"
#include "Glamor/Moe/MoeCodeDisassembler.h"
#include "Glamor/Moe/MoeOpcodeRenpyInterface.h"
GLAMOR_NAMESPACE_BEGIN

std::string MoeCodeDisassembler::Disassemble(Unique<MoeByteStreamReader> reader)
{
    std::ostringstream stream;
    int32_t cnt = 0;
    do
    {
        off_t offset = reader->GetReadOffsetInBuffer();
        int32_t buf = reader->GetBufferIndex();
        MoeByteStreamReader::Address bufPtr = reader->GetBufferPtr();

        auto verb = reader->ExtractNext<uint16_t>();
        if ((verb & 0xff) > opcode::g_opcode_vector_tbl_count)
            throw RuntimeException(__func__, "Illegal VM instruction for disassembling");
        const opcode::OpcodeVectorTbl *opv = &opcode::g_opcode_vector_tbl[(verb & 0xff) - 1];

        stream << fmt::format("  #{}:{:04d}<+{:04d}:{}>|    {} ", buf, cnt, offset, fmt::ptr(bufPtr), opv->name);

        const auto* opr = &opv->operands[0];
        while (opr->type_name)
        {
            if (opr != &opv->operands[0])
                stream << ", ";
            stream << opr->type_name << ' ';

            switch (static_cast<opcode::OperandTypes>(opr->type_id))
            {
            case opcode::OperandTypes::memop:
                stream << fmt::format("%0x{:x}", reader->ExtractNext<opcode::MemOp>());
                break;
            case opcode::OperandTypes::u8:
                stream << fmt::format("$0x{:x}", reader->ExtractNext<uint8_t>());
                break;
            case opcode::OperandTypes::i8:
                stream << fmt::format("$0x{:x}", reader->ExtractNext<int8_t>());
                break;
            case opcode::OperandTypes::u16:
                stream << fmt::format("$0x{:x}", reader->ExtractNext<uint16_t>());
                break;
            case opcode::OperandTypes::i16:
                stream << fmt::format("$0x{:x}", reader->ExtractNext<int16_t>());
                break;
            case opcode::OperandTypes::u32:
                stream << fmt::format("$0x{:x}", reader->ExtractNext<uint32_t>());
                break;
            case opcode::OperandTypes::i32:
                stream << fmt::format("$0x{:x}", reader->ExtractNext<int32_t>());
                break;
            case opcode::OperandTypes::u64:
                stream << fmt::format("$0x{:x}", reader->ExtractNext<uint64_t>());
                break;
            case opcode::OperandTypes::i64:
                stream << fmt::format("$0x{:x}", reader->ExtractNext<int64_t>());
                break;
            case opcode::OperandTypes::f32:
                stream << fmt::format("${}", reader->ExtractNext<float>());
                break;
            case opcode::OperandTypes::f64:
                stream << fmt::format("${}", reader->ExtractNext<double>());
                break;
            }
            opr++;
        }
        stream << '\n';

        if (verb == opcode::kSwitchNextBuffer)
            reader->MoveToNextBuffer();
        else if (verb == opcode::kCommandPoolEnd)
            break;
    } while (++cnt);

    return stream.str();
}

GLAMOR_NAMESPACE_END
