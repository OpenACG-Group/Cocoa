#ifndef COCOA_COBALT_COBALT_H
#define COCOA_COBALT_COBALT_H

#include <memory>
#include <vector>

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

#define g_async_api
#define g_sync_api

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
    g_nodiscard bool GetProfileRenderHostTransfer() const;

    void SetBackend(Backends backend);
    void SetSkiaJIT(bool allow);
    void SetProfileRenderHostTransfer(bool value = true);

private:
    Backends    fBackend;
    bool        fSkiaJIT;
    bool        fProfileRenderHostTransfer;
};

class GlobalScope : public UniquePersistent<GlobalScope>
{
public:
    struct ApplicationInfo
    {
        using VersionTriple = std::tuple<int32_t, int32_t, int32_t>;

        ApplicationInfo() = default;
        ApplicationInfo(const std::string_view& name, VersionTriple triple)
                : name(name), version_triple(std::move(triple)) {}
        ApplicationInfo(const ApplicationInfo&) = default;
        ApplicationInfo(ApplicationInfo&& rhs) noexcept
                : name(std::move(rhs.name))
                , version_triple(std::move(rhs.version_triple)) {}

        std::string         name;
        VersionTriple       version_triple;
    };

    GlobalScope(const ContextOptions& options, EventLoop *loop);
    ~GlobalScope();

    g_nodiscard ContextOptions& GetOptions();

    void Initialize(const ApplicationInfo& info);
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
#endif //COCOA_COBALT_COBALT_H
