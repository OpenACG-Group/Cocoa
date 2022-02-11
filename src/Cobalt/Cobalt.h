#ifndef COCOA_COBALT_H
#define COCOA_COBALT_H

#include <memory>
#include <vector>
#include <sigc++/sigc++.h>

#include "Core/UniquePersistent.h"
#include "Core/EventLoop.h"

#define COBALT_NAMESPACE_BEGIN  namespace cocoa::cobalt {
#define COBALT_NAMESPACE_END    }

COBALT_NAMESPACE_BEGIN

template<typename T>
using co_sp = std::shared_ptr<T>;

template<typename T>
using co_unique = std::unique_ptr<T>;

template<typename T>
using co_weak = std::weak_ptr<T>;

#define g_private_api
#define g_nodiscard     [[nodiscard]]
#define g_noreturn      [[noreturn]]
#define g_inline        inline
#define g_maybe_unused  [[maybe_unused]]

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

#define COBALT_BACKEND_WAYLAND      "wayland"

#define COBALT_SKIA_JIT_DEFAULT     true

enum class Backends
{
    kWayland,
    kDefault = kWayland
};

class RenderHost;
class RenderClient;

class ContextOptions
{
public:
    ContextOptions();
    ContextOptions(const ContextOptions&) = default;
    ~ContextOptions() = default;

    g_nodiscard Backends GetBackend() const;
    g_nodiscard bool GetSkiaJIT() const;

    void SetBackend(Backends backend);
    void SetSkiaJIT(bool allow);

private:
    Backends    fBackend;
    bool        fSkiaJIT;
};

class GlobalScope : public UniquePersistent<GlobalScope>
{
public:
    GlobalScope(const ContextOptions& options, EventLoop *loop);
    ~GlobalScope();

    g_nodiscard ContextOptions& GetOptions();

    void Initialize();
    void Dispose();

    g_nodiscard g_inline RenderHost *GetRenderHost() {
        return render_host_;
    }

    g_nodiscard g_inline RenderClient *GetRenderClient() {
        return render_client_;
    }

private:
    ContextOptions  fOptions;
    EventLoop      *event_loop_;
    RenderHost     *render_host_;
    RenderClient   *render_client_;
};

COBALT_NAMESPACE_END
#endif //COCOA_COBALT_H
