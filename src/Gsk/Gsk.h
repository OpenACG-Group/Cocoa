/**
 * GSK (Graphic Scene Kit) is a module that provides graphics-related
 * functions in Cocoa. It is the infrastructure of rendering, representation
 * and interaction with input devices. GSK deals with the window system
 * like X11 server or Wayland compositor directly, providing a uniform
 * window system abstraction.
 */

#ifndef COCOA_GSK_H
#define COCOA_GSK_H

#define GSK_VERSION_MAJOR   1
#define GSK_VERSION_MINOR   0

#define GSK_NAMESPACE_BEGIN namespace cocoa::Gsk {
#define GSK_NAMESPACE_END   }

#ifndef SK_VULKAN
#error SK_VULKAN is required to enable Skia's Vulkan support
#endif

#include <memory>
#include <cstdint>
#include <vector>
#include <optional>
#include <cstring>

#include "include/core/SkRefCnt.h"

#include "Core/Errors.h"
#include "Core/UniquePersistent.h"

GSK_NAMESPACE_BEGIN

template<typename T>
using Handle = std::shared_ptr<T>;
template<typename T>
using Unique = std::unique_ptr<T>;
template<typename T>
using Weak = std::weak_ptr<T>;

#define g_private_api

#define g_nodiscard [[nodiscard]]
#define g_noreturn [[noreturn]]
#define g_maybe_unused [[maybe_unused]]
#define g_inline    inline

#define GSK_CURRENT_TIME    0

#define g_signal_signature(signature, name) \
    sigc::signal<signature> s_##name;

#define g_signal_fields(...)    \
    struct Signals {            \
        __VA_ARGS__             \
    } __signals__;

#define g_signal_getter(name)                                       \
    inline auto& signal##name() { return __signals__.s_##name; }

#define g_signal_emit(name, ...)                \
    do {                                        \
        __signals__.s_##name.emit(__VA_ARGS__); \
    } while (false)

#define g_slot

#define g_slot_signature(emitterClass, name) \
    sigc::connection s_##emitterClass##name;

#define g_slot_fields(...) \
    struct Connections {    \
        __VA_ARGS__         \
    } __connections__;

#define g_slot_connect(emitterClass, name, emitter, slot) \
    __connections__.s_##emitterClass##name = (emitter)->signal##name().connect(slot)

#define g_slot_disconnect(emitterClass, name) \
    __connections__.s_##emitterClass##name.disconnect()

class GskDisplayManager;

#define GSK_BACKEND_XCB     "xcb"
#define GSK_BACKEND_WAYLAND "wayland"

struct GskOptions
{
    /* Turn off this by debug purpose */
    bool skia_allow_jit = true;
    bool vk_hw_accel = true;
    std::vector<std::string> disallow_backends;

    std::string cursor_theme_name = "default";
    int cursor_theme_size = 32;
};

class GskGlobalScope : public UniquePersistent<GskGlobalScope>
{
public:
    explicit GskGlobalScope(const GskOptions& options);
    ~GskGlobalScope();

    g_nodiscard g_inline const GskOptions& getOptions() const {
        return fOptions;
    }

    g_nodiscard Handle<GskDisplayManager> getDisplayManager() const;

private:
    GskOptions                    fOptions;
    Handle<GskDisplayManager>     fDisplayManager;
};

template<typename T, std::size_t S>
struct Vec
{
    using value_type = T;
    const static size_t size = S;

    Vec(const Vec<T, S>& lhs) {
        std::memcpy(&array, &lhs.array, S * sizeof(T));
    }
    Vec() = default;
    template<typename...ArgsT>
    explicit Vec(T p0, ArgsT&&...args) : array{p0, std::forward<ArgsT>(args)...} {}

    g_nodiscard g_inline T& operator[](const std::size_t index) {
        return array[index];
    }

    g_nodiscard g_inline T operator[](const std::size_t index) const {
        return array[index];
    }

    template<typename R, typename std::enable_if<std::is_convertible_v<R, T>>::type* = nullptr>
    g_inline Vec<T, S>& operator=(const Vec<R, S>& other) {
        for (size_t i = 0; i < S; i++)
            array[i] = static_cast<R>(other[i]);
        return *this;
    }

    g_nodiscard g_inline bool operator==(const Vec<T, S>& other) const {
        for (size_t i = 0; i < S; i++)
            if (array[i] != other.array[i])
                return false;
        return true;
    }

    g_nodiscard g_inline bool operator!=(const Vec<T, S>& other) const {
        return !(this->operator==(other));
    }

    T array[S];
};

using Vec2i = Vec<int32_t, 2>;
using Vec2f = Vec<float, 2>;
using Vec2d = Vec<double, 2>;

template<typename T>
using Maybe = std::optional<T>;

template<typename T>
struct SkSpHashFunctor
{
    using value_type = T;
    using functor_arg_type = const sk_sp<T>&;

    g_inline uint64_t operator()(functor_arg_type sp) const {
        CHECK(sp && "Hash an empty sk_sp<>");
        return reinterpret_cast<uint64_t>(sp.get());
    }
};

#define sksp_extract_type(t) decltype(t)::element_type
#define sksp_hash(v)         SkSpHashFunctor<sksp_extract_type(v)>()(v)

GSK_NAMESPACE_END
#endif //COCOA_GSK_H
