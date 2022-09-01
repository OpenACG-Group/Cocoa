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
#include "Core/Exception.h"
#include "Glamor/Moe/MoeByteStreamReader.h"
GLAMOR_NAMESPACE_BEGIN

MoeByteStreamReader::MoeByteStreamReader(CodeHolderVector&& holders)
    : code_holder_array_(std::forward<CodeHolderVector>(holders))
    , current_code_holder_index_(0)
    , current_rdptr_(nullptr)
    , current_endptr_(nullptr)
{
    CHECK(!code_holder_array_.empty());

    std::unique_ptr<MoeCodeHolder>& firstHolder = code_holder_array_[0];
    current_rdptr_ = firstHolder->GetStartAddress();
    current_endptr_ = current_rdptr_ + firstHolder->GetLength();
}

MoeByteStreamReader::~MoeByteStreamReader() = default;

void MoeByteStreamReader::LookForward(size_t count)
{
    if (current_rdptr_ + count > current_endptr_)
    {
        throw RuntimeException(__func__,
            "Corrupted bytecode: reached buffer boundary without buffer switching");
    }

    current_rdptr_ += count;
}

void MoeByteStreamReader::MoveToNextBuffer()
{
    current_code_holder_index_++;
    if (current_code_holder_index_ >= code_holder_array_.size())
    {
        throw RuntimeException(__func__,
            "Failed in switching to next buffer: end of buffer chain");
    }

    std::unique_ptr<MoeCodeHolder>& holder = code_holder_array_[current_code_holder_index_];
    current_rdptr_ = holder->GetStartAddress();
    current_endptr_ = current_rdptr_ + holder->GetLength();
}

GLAMOR_NAMESPACE_END
