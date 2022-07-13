#ifndef COCOA_GLAMOR_MOEBYTESTREAMREADER_H
#define COCOA_GLAMOR_MOEBYTESTREAMREADER_H

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
