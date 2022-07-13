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
