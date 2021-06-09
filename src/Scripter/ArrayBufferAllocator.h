#ifndef COCOA_ARRAYBUFFERALLOCATOR_H
#define COCOA_ARRAYBUFFERALLOCATOR_H

#include "include/v8.h"
#include "Scripter/ScripterBase.h"
SCRIPTER_NS_BEGIN

class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator
{
public:
    ArrayBufferAllocator();
    ~ArrayBufferAllocator() override;

    void *Allocate(size_t length) override;
    void *AllocateUninitialized(size_t length) override;
    void *Reallocate(void* data, size_t old_length, size_t new_length) override;
    void Free(void* data, size_t length) override;
};

SCRIPTER_NS_END
#endif //COCOA_ARRAYBUFFERALLOCATOR_H
