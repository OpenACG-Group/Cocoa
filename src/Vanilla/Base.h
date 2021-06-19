#ifndef COCOA_VANILLA_BASE_H
#define COCOA_VANILLA_BASE_H

#include <memory>
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

using VaScalar = double;
class VaVec2f
{
public:
    VaVec2f(VaScalar x, VaScalar y) : fX(x), fY(y) {}
    ~VaVec2f() = default;
    inline VaScalar x() const { return fX; }
    inline VaScalar y() const { return fY; }
private:
    VaScalar fX;
    VaScalar fY;
};

/* Other definitions */
#define va_nodiscard    [[nodiscard]]

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
