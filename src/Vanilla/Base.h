#ifndef COCOA_VANILLA_BASE_H
#define COCOA_VANILLA_BASE_H

#include <cstdint>
#include <memory>
#include <ostream>
#include <source_location>
#include <sigc++/sigc++.h>
#include "include/core/SkImageInfo.h"
#include "Core/Exception.h"

#define VANILLA_NS_BEGIN        namespace cocoa::vanilla {
#define VANILLA_NS_END          }
VANILLA_NS_BEGIN

#define VANILLA_MAJOR_VERSION   1
#define VANILLA_MINOR_VERSION   0
#define VANILLA_VERSION         "1.0"

/* Other definitions */
#define va_nodiscard    [[nodiscard]]
#define va_maybe_unused [[maybe_unused]]

class VanillaException : public RuntimeException
{
public:
    VanillaException(const std::string& who, const std::string& what)
        : RuntimeException(who, what) {}
    ~VanillaException() override = default;
};

template<typename T> using Handle = std::shared_ptr<T>;
template<typename T> using UniqueHandle = std::unique_ptr<T>;
template<typename T> using WeakHandle = std::weak_ptr<T>;

namespace vec
{
typedef int16_t __attribute__((vector_size(8))) short4;
typedef int32_t __attribute__((vector_size(8))) int2;
typedef float __attribute__((vector_size(8))) float2;
typedef int8_t __attribute__((vector_size(8))) char8;
typedef uint16_t __attribute__((vector_size(8))) ushort4;
typedef uint32_t __attribute__((vector_size(8))) uint2;
typedef uint8_t __attribute__((vector_size(8))) uchar8;

typedef int16_t __attribute__((vector_size(16))) short8;
typedef int32_t __attribute__((vector_size(16))) int4;
typedef float __attribute__((vector_size(16))) float4;
typedef int8_t __attribute__((vector_size(16))) char16;
typedef uint16_t __attribute__((vector_size(16))) ushort8;
typedef uint32_t __attribute__((vector_size(16))) uint4;
typedef uint8_t __attribute__((vector_size(16))) uchar16;

typedef int16_t __attribute__((vector_size(32))) short16;
typedef int32_t __attribute__((vector_size(32))) int8;
typedef int8_t __attribute__((vector_size(32))) char32;
typedef uint16_t __attribute__((vector_size(32))) ushort16;
typedef uint32_t __attribute__((vector_size(32))) uint8;
typedef uint8_t __attribute__((vector_size(32))) uchar32;
} // namespace vec

using VaScalar = float;

/* Signals */
#define VA_SIG_SIGNATURE(signature, name) \
    sigc::signal<signature> s_##name;

#define VA_SIG_FIELDS(...)  \
    struct Signals {        \
        __VA_ARGS__         \
    } __signals__;

#define VA_SIG_GETTER(name) \
    inline auto& signal##name() { return __signals__.s_##name; }

#define va_slot
#define VA_SLOT_SIGNATURE(emitterClass, name) \
    sigc::connection s_##emitterClass##name;

#define VA_SLOT_FIELDS(...) \
    struct Connections {    \
        __VA_ARGS__         \
    } __connections__;

#define va_slot_connect(emitterClass, name, emitter, slot) \
    __connections__.s_##emitterClass##name = emitter->signal##name().connect(slot)

#define va_slot_disconnect(emitterClass, name) \
    __connections__.s_##emitterClass##name.disconnect()

VANILLA_NS_END
#endif //COCOA_VANILLA_BASE_H
