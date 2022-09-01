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

#ifndef COCOA_GLAMOR_MOEBYTESTREAMREADER_H
#define COCOA_GLAMOR_MOEBYTESTREAMREADER_H

#include "Core/Errors.h"
#include "Glamor/Glamor.h"
#include "Glamor/Moe/MoeCodeHolder.h"
GLAMOR_NAMESPACE_BEGIN

class MoeByteStreamReader
{
public:
    using CodeHolderVector = std::vector<std::unique_ptr<MoeCodeHolder>>;
    using Address = const uint8_t *;

    explicit MoeByteStreamReader(CodeHolderVector&& holders);
    ~MoeByteStreamReader();

    template<typename T>
    T ExtractNext();

    template<typename T>
    T PeekNext();

    template<typename T>
    void SwallowNext();

    void MoveToNextBuffer();

    g_nodiscard g_inline off_t GetReadOffsetInBuffer() const {
        return (current_rdptr_ - code_holder_array_[current_code_holder_index_]->GetStartAddress());
    }

    g_nodiscard g_inline int32_t GetBufferIndex() const {
        return current_code_holder_index_;
    }

    g_nodiscard g_inline Address GetBufferPtr() const {
        return current_rdptr_;
    }

    g_nodiscard g_inline size_t GetBuffersNum() const {
        return code_holder_array_.size();
    }

    g_nodiscard g_inline const CodeHolderVector::value_type& GetBufferByIndex(size_t idx) const {
        CHECK(idx < code_holder_array_.size());
        return code_holder_array_[idx];
    }

private:
    void LookForward(size_t count);

    CodeHolderVector    code_holder_array_;
    int32_t             current_code_holder_index_;
    Address             current_rdptr_;
    Address             current_endptr_;
};

template<typename T>
T MoeByteStreamReader::ExtractNext()
{
    LookForward(sizeof(T));
    return *reinterpret_cast<const T *>(current_rdptr_ - sizeof(T));
}

template<typename T>
T MoeByteStreamReader::PeekNext()
{
    LookForward(sizeof(T));
    current_rdptr_ -= sizeof(T);
    return *reinterpret_cast<const T *>(current_rdptr_);
}

template<typename T>
void MoeByteStreamReader::SwallowNext()
{
    LookForward(sizeof(T));
}

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_MOEBYTESTREAMREADER_H
