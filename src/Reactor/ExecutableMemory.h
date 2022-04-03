#ifndef COCOA_REACTOR_EXECUTABLEMEMORY_H
#define COCOA_REACTOR_EXECUTABLEMEMORY_H

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "Reactor/Reactor.h"
REACTOR_NAMESPACE_BEGIN

enum class MemPermission : uint32_t
{
    kRead    = 1 << 1,
    kWrite   = 1 << 2,
    kExecute = 1 << 3
};

size_t MemoryPageSize();
void *AllocateMemoryPages(size_t bytes, Bitfield<MemPermission> perm, bool needExec);
void ProtectMemoryPages(void *memory, size_t bytes, Bitfield<MemPermission> perm);
void DeallocateMemoryPages(void *memory, size_t bytes);

template<typename P>
P unaligned_read(P *address)
{
    P value;
    std::memcpy(&value, address, sizeof(P));
    return value;
}

template<typename P, typename V>
void unaligned_write(P *address, V value)
{
    static_assert(sizeof(V) == sizeof(P), "value size must match pointer size");
    std::memcpy(address, &value, sizeof(P));
}

template<typename P>
class unaligned_ref
{
public:
    explicit unaligned_ref(void *ptr) : ptr((P *)ptr) {}

    template<typename V>
    P operator=(V value) {
        unaligned_write(ptr, value);
        return value;
    }

    explicit operator P() {
        return unaligned_read((P *)ptr);
    }

private:
    P *ptr;
};

template<typename P>
class unaligned_ptr
{
    template<typename S>
    friend class unaligned_ptr;

public:
    explicit unaligned_ptr(P *ptr) : ptr(ptr) {}

    unaligned_ref<P> operator*() {
        return unaligned_ref<P>(ptr);
    }

    template<typename S>
    explicit operator S() {
        return S(ptr);
    }

private:
    void *ptr;
};

REACTOR_NAMESPACE_END
#endif //COCOA_REACTOR_EXECUTABLEMEMORY_H
