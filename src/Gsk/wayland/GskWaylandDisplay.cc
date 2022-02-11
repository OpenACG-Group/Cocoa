#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/memfd.h>
#include <syscall.h>

#include "Core/Filesystem.h"
#include "Core/Journal.h"
#include "Core/Errors.h"
#include "Core/EventLoop.h"
#include "Core/Exception.h"
#include "Core/Properties.h"

#include "Gsk/wayland/GskWaylandDisplay.h"
#include "Gsk/wayland/GskWaylandMonitor.h"
#include "Gsk/wayland/GskWaylandSeat.h"
#include "Gsk/wayland/GskWaylandKeymap.h"
#include "Gsk/GskEventQueue.h"
#include "Gsk/wayland/GskWaylandDevice.h"

GSK_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gsk.Wayland.Display)

namespace
{

void clientLogHandler(const char *format, va_list va)
{
    va_list copied;
    va_copy(copied, va);

    size_t size = std::vsnprintf(nullptr, 0, format, va);
    char *buffer = new char[size + 1];
    CHECK(buffer);
    std::vsnprintf(buffer, size + 1, format, copied);
    char *ptr = buffer + size;
    while (ptr >= buffer)
    {
        if (*ptr == '\n')
        {
            *ptr = '\0';
            break;
        }
        ptr--;
    }
    QLOG(LOG_INFO, "{}", buffer);
}

struct FutureGlobalsRequirement : UniquePersistent<FutureGlobalsRequirement>
{
    struct Req
    {
        std::vector<std::string> requires;
        uint32_t id;
        uint32_t version;
        std::function<void(GskWaylandDisplay*, const Req&)> callback;
    };
    std::list<Req> requests;

    static bool HasRequiredGlobal(const Req& req, GskWaylandDisplay *wd)
    {
        const auto& known = wd->getGlobalsSet()->known_globals;
        for (auto& iface : req.requires) // NOLINT
        {
            if (std::find(known.begin(), known.end(), iface) == known.end())
                return false;
        }
        return true;
    }

    void processRequests(GskWaylandDisplay *display)
    {
        for (auto itr = requests.begin(); itr != requests.end(); itr++)
        {
            if (HasRequiredGlobal(*itr, display))
            {
                itr->callback(display, *itr);
                itr = requests.erase(itr);
            }
            if (itr == requests.end())
                break;
        }
    }
};

std::string getFormatName(uint32_t format)
{
    if (format == 0)
        return "ARGB8888";
    else if (format == 1)
        return "XRGB8888";
    else
    {
        char buf[10];
        snprintf(buf, 10, "4cc %c%c%c%c", (char) (format & 0xff), (char) ((format >> 8) & 0xff),
                 (char) ((format >> 16) & 0xff), (char) ((format >> 24) & 0xff));
        return buf;
    }
    MARK_UNREACHABLE();
}

void shmListenerFormat(g_maybe_unused void *data, g_maybe_unused wl_shm *shm, uint32_t fmt)
{
    QLOG(LOG_INFO, "SHM supported pixel format {} (0x{:08x})", getFormatName(fmt), fmt);
}

const wl_shm_listener shm_listener_ = { shmListenerFormat };

void registryListenerGlobalAdd(void *data, ::wl_registry *registry, uint32_t name,
                               const char *interface, uint32_t version)
{
    QLOG(LOG_DEBUG, "Global %fg<hl>{}%reset [%fg<bl>id={}%reset, %fg<cy>version={}%reset]",
         interface, name, version);
    GskWaylandDisplay *display = GskWaylandDisplay::CastBare(data);
    CHECK(display);

    auto& set = display->getGlobalsSet();

#define iface_match(s) std::strcmp(interface, s) == 0
    if (iface_match("wl_compositor"))
    {
        set->compositor = (wl_compositor*) wl_registry_bind(registry, name, &wl_compositor_interface,
                                                            std::min(version, 4U));
        set->compositorVersion = std::min(version, 4U);
    }
    else if (iface_match("wl_shm"))
    {
        set->shm = (wl_shm *) wl_registry_bind(registry, name, &wl_shm_interface, 1);
        wl_shm_add_listener(set->shm, &shm_listener_, data);
    }
    else if (iface_match("xdg_wm_base"))
    {
        set->wmBaseId = name;
        set->wmBaseVersion = version;
    }
    else if (iface_match("zxdg_shell_v6"))
    {
        set->zxdgShellId = name;
    }
    else if (iface_match("wl_output"))
    {
        auto output = (wl_output *) wl_registry_bind(registry, name,
                                                     &wl_output_interface, std::min(version, 2U));
        display->addMonitor(std::make_shared<GskWaylandMonitor>(display->weak_from_this(),
                                                                name,
                                                                version,
                                                                output));
        display->insertServerBarrier([] {
            QLOG(LOG_DEBUG, "Output adding server barrier has reached");
        });
    }
    else if (iface_match("zxdg_output_manager_v1"))
    {
        set->outputManagerVersion = std::min(version, 3U);
        set->outputManager = (zxdg_output_manager_v1*) wl_registry_bind(registry,
                                                                        name,
                                                                        &zxdg_output_manager_v1_interface,
                                                                        set->outputManagerVersion);
        for (const Handle<GskMonitor>& pMonitor : display->getMonitor())
        {
            auto pCast = std::dynamic_pointer_cast<GskWaylandMonitor>(pMonitor);
            pCast->setupXDGOutputManager();
        }
        display->insertServerBarrier([] {
            QLOG(LOG_DEBUG, "XDG Output Manager server barrier has reached");
        });
    }
    else if (iface_match("wl_seat"))
    {
        FutureGlobalsRequirement::Req req{};
        req.id = name;
        req.version = version;
        req.requires = {"wl_compositor", "wl_data_device_manager"};
        req.callback = [registry](GskWaylandDisplay *display, const FutureGlobalsRequirement::Req& req) {
            auto& set = display->getGlobalsSet();
            set->seat = (wl_seat *) wl_registry_bind(registry, req.id, &wl_seat_interface,
                                                     std::min(req.version, 7U));
            display->addSeat(std::make_shared<GskWaylandSeat>(display->weak_from_this(),
                                                              set->seat,
                                                              req.id));
            display->insertServerBarrier([] {
                QLOG(LOG_DEBUG, "Seat adding server barrier has reached");
            });
        };
        FutureGlobalsRequirement::Ref().requests.push_back(req);
    }
    else if (iface_match("wl_data_device_manager"))
    {
        set->dataDeviceManagerVersion = std::min(version, 3U);
        set->dataDeviceManager =
                (wl_data_device_manager *) wl_registry_bind(registry, name,
                                                            &wl_data_device_manager_interface,
                                                            set->dataDeviceManagerVersion);
    }
    else if (iface_match("wl_subcompositor"))
    {
        set->subCompositor =
                (wl_subcompositor *) wl_registry_bind(registry, name, &wl_subcompositor_interface, 1);
    }
    else if (iface_match("zwp_pointer_gestures_v1"))
    {
        set->pointerGestures =
                (zwp_pointer_gestures_v1 *) wl_registry_bind(registry, name, &zwp_pointer_gestures_v1_interface,
                                                             std::min(version, 1U));
    }
    else if (iface_match("zwp_primary_selection_device_manager_v1"))
    {
        set->primarySelectionManager =
                (zwp_primary_selection_device_manager_v1 *)
                wl_registry_bind(registry, name, &zwp_primary_selection_device_manager_v1_interface, 1);
    }
    else if (iface_match("zwp_tablet_manager_v2"))
    {
        set->tabletManager =
                (zwp_tablet_manager_v2 *) wl_registry_bind(registry, name, &zwp_tablet_manager_v2_interface, 1);
    }

#undef iface_match
    set->known_globals.emplace_back(interface);
}

void registryListenerGlobalRemove(g_maybe_unused void *data, g_maybe_unused wl_registry *registry, uint32_t name)
{
    QLOG(LOG_DEBUG, "Remove interface: {}", name);

    GskWaylandDisplay::CastBare(data)->removeMaybeOutput(name);

    // FIXME(display-important-wayland): Leaking here,
    //                                   removing seat and output is necessary (gdkdisplay-wayland.c:508)
}

const wl_registry_listener registry_listener_ = {
    .global = registryListenerGlobalAdd,
    .global_remove = registryListenerGlobalRemove
};

void WMBasePingResponse(void *data, xdg_wm_base *wm, uint32_t serial)
{
    xdg_wm_base_pong(wm, serial);
}
const xdg_wm_base_listener wm_base_listener_ = { WMBasePingResponse };

void ZXDGShellPingResponse(void *data, zxdg_shell_v6 *shell, uint32_t serial)
{
    zxdg_shell_v6_pong(shell, serial);
}
const zxdg_shell_v6_listener zxdg_shell_listener_ = { ZXDGShellPingResponse };

} // namespace anonymous

GskWaylandDisplay::GskWaylandDisplay(int32_t fd)
    : GskDisplay()
    , PollSource(EventLoop::Instance(), fd)
    , PrepareSource(EventLoop::Instance())
    , fPods(std::make_unique<NativePODs>())
    , fGlobalsSet(std::make_unique<GlobalsSet>())
    , fShellVariant(ShellVariant::kPending)
    , fSerial(0)
    , fCursorThemeSize(0)
    , fXKBContext(xkb_context_new(XKB_CONTEXT_NO_FLAGS))
{
}

GskWaylandDisplay::~GskWaylandDisplay()
{
    CHECK(isClosed() && "Leaking: Display is not closed");
}

bool GskWaylandDisplay::prefersSSD()
{
    // TODO(extra-display-wayland): KWin-specified protocols. (gdkdisplay-wayland.c:333)
    return false;
}

namespace {

void prepare_cursor_themes(const Handle<GskWaylandDisplay>& display)
{
    FutureGlobalsRequirement::Req req{};
    req.requires = { "wl_shm" };
    req.callback = [](GskWaylandDisplay *display, const FutureGlobalsRequirement::Req& req) {
        auto& options = GskGlobalScope::Ref().getOptions();
        display->setCursorTheme(options.cursor_theme_name, options.cursor_theme_size);
    };
    FutureGlobalsRequirement::Ref().requests.push_back(req);
}

} // namespace anonymous

Handle<GskWaylandDisplay> GskWaylandDisplay::Make(const std::string& displayName)
{
    wl_log_set_handler_client(clientLogHandler);
    const char *name = displayName.empty() ? nullptr : displayName.c_str();
    wl_display *display_ = wl_display_connect(name);
    if (!display_)
    {
        QLOG(LOG_ERROR, "Unable to connect to Wayland compositor");
        return nullptr;
    }
    uint32_t version = wl_display_get_version(display_);
    QLOG(LOG_INFO, "Connected to Wayland compositor, version {}", version);

    auto display = std::make_shared<GskWaylandDisplay>(wl_display_get_fd(display_));
    CHECK(display);
    wl_display_set_user_data(display_, display.get());
    ScopeEpilogue closeDisplayScope([display] {
        display->cocoa::Gsk::GskDisplay::close();
    });

    display->fPods->display = display_;
    display->fPods->registry = wl_display_get_registry(display_);
    wl_registry *registry = display->fPods->registry;

    FutureGlobalsRequirement::New();
    ScopeEpilogue scope([] { FutureGlobalsRequirement::Delete(); });

    prepare_cursor_themes(display);

    wl_registry_add_listener(registry, &registry_listener_, display.get());
    if (wl_display_roundtrip(display_) < 0)
        return nullptr;
    FutureGlobalsRequirement::Ref().processRequests(display.get());

    if (!display->waitForAllServerBarriers())
        return nullptr;

    auto& set = display->getGlobalsSet();
    if (set->wmBaseId > 0)
    {
        display->fShellVariant = ShellVariant::kXDG;
        set->wmBaseVersion = std::min(set->wmBaseVersion, 3U);
        set->wmBase = (xdg_wm_base *) wl_registry_bind(registry, set->wmBaseId,
                                                       &xdg_wm_base_interface, set->wmBaseVersion);
        xdg_wm_base_add_listener(set->wmBase, &wm_base_listener_, nullptr);
    }
    else if (set->zxdgShellId > 0)
    {
        display->fShellVariant = ShellVariant::kZXDG;
        set->zxdgShell = (zxdg_shell_v6 *) wl_registry_bind(registry, set->zxdgShellId,
                                                            &zxdg_shell_v6_interface, 1);
        zxdg_shell_v6_add_listener(set->zxdgShell, &zxdg_shell_listener_, nullptr);
    }
    else
    {
        QLOG(LOG_ERROR, "Wayland compositor does not provide any supported shell interface");
        return nullptr;
    }

    QLOG(LOG_INFO, "Wayland compositor provides {} as shell interface",
         display->fShellVariant == ShellVariant::kXDG ? "XDG_WM_BASE" : "ZXDG_SHELL_V6");

    closeDisplayScope.abolish();
    display->initialize();
    display->startPoll(UV_READABLE | UV_DISCONNECT);
    display->startPrepare();
    return display;
}

KeepInLoop GskWaylandDisplay::prepareDispatch()
{
    auto queue = getUniqueEventQueue();
    if (queue->getPauseCount() > 0)
    {
        /* Event loop has been paused. We dispatch events immediately. */
        if (queue->getFirstNotFilledEventIterator() != queue->endIterator())
            dispatchEventsFromQueue();
        return KeepInLoop::kYes;
    }

    if (queue->getFirstNotFilledEventIterator() != queue->endIterator())
    {
        dispatchEventsFromQueue();
        return KeepInLoop::kYes;
    }

    if (queue->isReading())
        return KeepInLoop::kYes;

    if (wl_display_prepare_read(fPods->display) != 0)
        return KeepInLoop::kYes;
    for (wl_event_queue *wq : fPods->queues)
    {
        if (wl_display_prepare_read_queue(fPods->display, wq) != 0)
        {
            wl_display_cancel_read(fPods->display);
            return KeepInLoop::kYes;
        }
        wl_display_cancel_read(fPods->display);
    }
    queue->setReading(true);

    wl_display_flush(fPods->display);
    return KeepInLoop::kYes;
}

KeepInLoop GskWaylandDisplay::pollDispatch(int status, int events)
{
    auto queue = getUniqueEventQueue();
    if (queue->getPauseCount() > 0)
    {
        if (queue->isReading())
            wl_display_cancel_read(fPods->display);
        queue->setReading(false);
        if (queue->getFirstNotFilledEventIterator() != queue->endIterator())
            dispatchEventsFromQueue();
        return KeepInLoop::kYes;
    }
    if (queue->isReading())
    {
        if (events == UV_READABLE)
            wl_display_read_events(fPods->display);
        else
            wl_display_cancel_read(fPods->display);
        queue->setReading(false);
    }

    if (queue->getFirstNotFilledEventIterator() != queue->endIterator() || events == UV_READABLE)
        dispatchEventsFromQueue();
    return KeepInLoop::kYes;
}

void GskWaylandDisplay::dispatchEventsFromQueue()
{
    Handle<GskEvent> event = this->getEventFromQueue();
    if (event)
        GskEventQueue::Emit(event);
}

void GskWaylandDisplay::onEnqueueEvents(Unique<GskEventQueue>& queue)
{
    wl_display_dispatch_pending(fPods->display);

    for (wl_event_queue *wq : fPods->queues)
        wl_display_dispatch_queue_pending(fPods->display, wq);
}

void GskWaylandDisplay::onDispose()
{
    if (!isClosed())
    {
        PollSource::stopPoll();
        PrepareSource::stopPrepare();

        for (const auto& pair : fCursorSkSurfaceCache)
            delete pair.second;
        fCursorSkSurfaceCache.clear();

        for (ServerBarrier *sb : fServerBarriers)
        {
            wl_callback_destroy(sb->wlc);
            delete sb;
        }
        // TODO: Free globals?
        fTmpKeymap.reset();
        fGlobalsSet.reset();
        if (fPods->registry)
            wl_registry_destroy(fPods->registry);
        if (fPods->display)
            wl_display_disconnect(fPods->display);
        fPods.reset();
        xkb_context_unref(fXKBContext);
    }
}

void GskWaylandDisplay::insertServerBarrier(const std::function<void()>& func)
{
    static const wl_callback_listener listener = { ServerBarrierCallback };
    wl_callback *cb = wl_display_sync(fPods->display);
    auto *sb = new ServerBarrier{ this, func, cb };
    wl_callback_add_listener(cb, &listener, sb);
    fServerBarriers.push_back(sb);
}

bool GskWaylandDisplay::waitForAllServerBarriers()
{
    while (!fServerBarriers.empty())
    {
        if (wl_display_dispatch(fPods->display) < 0)
            return false;
    }
    return true;
}

void GskWaylandDisplay::ServerBarrierCallback(void *data, wl_callback *cb, g_maybe_unused uint32_t time)
{
    auto *barrier = reinterpret_cast<ServerBarrier*>(data);
    barrier->cb();
    barrier->display->fServerBarriers.remove(barrier);
    wl_callback_destroy(cb);
    delete barrier;
}

void GskWaylandDisplay::onSync()
{
    wl_display_roundtrip(fPods->display);
}

void GskWaylandDisplay::onFlush()
{
    if (!isClosed())
        wl_display_flush(fPods->display);
}

void GskWaylandDisplay::onMakeDefault()
{
    // TODO(extra-display-wayland): implement this (gdkdisplay-wayland.c)
    QLOG(LOG_DEBUG, "onMakeDefault: Not implemented yet");
}

bool GskWaylandDisplay::onHasPending()
{
    return false;
}

uint32_t GskWaylandDisplay::onGetNextSerial()
{
    static ulong serial = 0;
    return ++serial;
}

Handle<GskKeymap> GskWaylandDisplay::onGetKeymap()
{
    Handle<GskSeat> seat = getDefaultSeat();
    Handle<GskDevice> coreKeyboard;
    if (seat)
        coreKeyboard = seat->getKeyboard();
    if (coreKeyboard && fTmpKeymap)
        fTmpKeymap.reset();

    if (coreKeyboard)
    {
        auto dev = std::dynamic_pointer_cast<GskWaylandDevice>(coreKeyboard);
        return dev->getKeymap();
    }

    if (!fTmpKeymap)
    {
        auto sptr = std::dynamic_pointer_cast<GskWaylandDisplay>(shared_from_this());
        std::weak_ptr<GskWaylandDisplay> weak = sptr;
        fTmpKeymap = std::make_shared<GskWaylandKeymap>(weak);
    }
    return fTmpKeymap;
}

Handle<GskMonitor> GskWaylandDisplay::onGetMonitorAtSurface(const Handle<GskSurface>& surface)
{
    // TODO(display-wayland): implement this (gdkdisplay-wayland.c:903)
    return nullptr;
}

namespace {

wl_cursor_theme *try_load_theme(wl_shm *shm,
                                const std::string& dir,
                                bool dotdir,
                                const std::string& name,
                                int size)
{
    auto path = fmt::format("{}/{}/{}/cursors", dir, dotdir ? ".icons" : "icons", name);
    path = vfs::Realpath(path);

    if (vfs::Access(path, {vfs::AccessMode::kRegular}) != vfs::AccessResult::kOk)
        return nullptr;

    return wl_cursor_theme_create(path.c_str(), size, shm);
}

wl_cursor_theme *get_cursor_name(wl_shm *shm,
                                 const std::string& name,
                                 int size)
{
    using Data = PropertyDataNode;
    auto paths = prop::Get()->next("Runtime")->next("Paths");

    wl_cursor_theme *theme;

    theme = try_load_theme(shm,
                           paths->next("UserData")->as<Data>()->extract<std::string>(),
                           false,
                           name,
                           size);
    if (theme)
        return theme;

    theme = try_load_theme(shm,
                           paths->next("Home")->as<Data>()->extract<std::string>(),
                           true,
                           name,
                           size);
    if (theme)
        return theme;

    auto dataDirs = paths->next("SystemData")->as<PropertyArrayNode>();
    for (const auto& dir : *dataDirs)
    {
        theme = try_load_theme(shm, dir->as<Data>()->extract<std::string>(),
                               false, name, size);
        if (theme)
            return theme;
    }

    return wl_cursor_theme_create("/usr/share/icons/default/cursors", size, shm);
}

} // namespace anonymous

void GskWaylandDisplay::onSetCursorTheme(const std::string& name, int size)
{
    CHECK(fGlobalsSet->shm);
    if (fCursorThemeName == name && fCursorThemeSize == size)
        return;

    wl_cursor_theme *theme = get_cursor_name(fGlobalsSet->shm, name, size);

    if (theme == nullptr)
    {
        QLOG(LOG_WARNING, "Failed to load cursor theme {}", name);
        return;
    }

    if (fCursorTheme)
    {
        wl_cursor_theme_destroy(fCursorTheme);
        fCursorTheme = nullptr;
    }

    fCursorTheme = theme;
    fCursorThemeName = name;
    fCursorThemeSize = size;
}

namespace {

/* We use low-level coding style here. So goto is fine here. */

int open_posix_shared_memory()
{
    static bool force_shm_open = false;
    int ret = -1;

#if !defined(__NR_memfd_create)
    force_shm_open = true;
#endif

    do
    {
#if defined(__NR_memfd_create)
        if (!force_shm_open)
        {
            int options = MFD_CLOEXEC;
#if defined(MFD_ALLOW_SEALING)
            options |= MFD_ALLOW_SEALING;
#endif /* MFD_ALLOW_SEALING */
            ret = static_cast<int>(::syscall(__NR_memfd_create, "Cocoa:GskWayland", options));
            /* fall back to shm_open until Debian stops shipping 3.16 kernel */
            if (ret < 0 && errno == ENOSYS)
                force_shm_open = true;
#if defined(F_ADD_SEALS) && defined(F_SEAL_SHRINK)
            if (ret >= 0)
                fcntl(ret, F_ADD_SEALS, F_SEAL_SHRINK);
#endif /* F_ADD_SEALS && F_SEAL_SHRINK */
        }
#endif /* __NR_memfd_create */
        if (force_shm_open)
        {
            char name[NAME_MAX - 1] = "";
            std::sprintf(name, "/Cocoa::GskWayland:%x", static_cast<uint32_t>(random() & 0xffffffff));

            ret = ::shm_open(name, O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, 0600);

            if (ret >= 0)
                ::shm_unlink(name);
            else if (errno == EEXIST)
                continue;
        }
    } while (ret < 0 && errno == EINTR /* Syscall may be interrupted by signal */);

    if (ret < 0)
    {
        QLOG(LOG_ERROR, "Creating shared memory file (using {}) failed: {}",
             force_shm_open ? "shm_open" : "memfd_create", ::strerror(errno));
    }

    return ret;
}

wl_shm_pool *create_posix_shm_pool(wl_shm *shm, int size, size_t *buf_length, void **data_out)
{
    int fd;
    wl_shm_pool *pool;
    void *data;

    fd = open_posix_shared_memory();
    if (fd < 0)
        goto fail;

    if (::ftruncate(fd, size) < 0)
    {
        QLOG(LOG_ERROR, "Truncating shared memory file failed: {}", ::strerror(errno));
        ::close(fd);
        goto fail;
    }

    data = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED)
    {
        QLOG(LOG_ERROR, "Mapping shared memory file failed: {}", ::strerror(errno));
        ::close(fd);
        goto fail;
    }

    pool = wl_shm_create_pool(shm, fd, size);
    ::close(fd);

    *data_out = data;
    *buf_length = size;

    return pool;

fail:
    *data_out = nullptr;
    *buf_length = 0;
    return nullptr;
}

void raster_surface_destroy(g_maybe_unused void *pixels, void *context)
{
    auto *data = reinterpret_cast<WaylandSharedMemoryBuffer*>(context);

    if (data->buffer)
        wl_buffer_destroy(data->buffer);

    if (data->pool)
        wl_shm_pool_destroy(data->pool);

    ::munmap(data->data, data->length);
    delete data;
}

} // namespace anonymous

GskSkSurfaceWrapper *GskWaylandDisplay::createRasterShmSurface(const Vec2i& size, int scale)
{
    if (scale != 1)
    {
        // FIXME: Skia doesn't support surface-level scaling
        QLOG(LOG_WARNING, "fixme: Unsupported scale factor");
        return nullptr;
    }

    auto *data = new WaylandSharedMemoryBuffer;
    data->display = std::dynamic_pointer_cast<GskWaylandDisplay>(shared_from_this());
    data->buffer = nullptr;
    data->scale = scale;

    /* FIXME: I really don't known about the SkColorType (kBGRA or kARGB?) */
    SkImageInfo info = SkImageInfo::Make(SkISize::Make(size[0] * scale, size[1] * scale),
                                         SkColorType::kBGRA_8888_SkColorType,
                                         SkAlphaType::kUnpremul_SkAlphaType);

    data->pool = create_posix_shm_pool(fGlobalsSet->shm,
                                       static_cast<int32_t>(info.computeMinByteSize()),
                                       &data->length,
                                       &data->data);
    if (UNLIKELY(data->pool == nullptr))
    {
        QLOG(LOG_ERROR, "Unable to create shared memory pool");
        std::exit(1);
    }

    sk_sp<SkSurface> surface = SkSurface::MakeRasterDirectReleaseProc(info,
                                                                      data->data,
                                                                      info.minRowBytes(),
                                                                      raster_surface_destroy,
                                                                      data);

    data->buffer = wl_shm_pool_create_buffer(data->pool, 0, size[0] * scale, size[1] * scale,
                                             static_cast<int>(info.minRowBytes()),
                                             WL_SHM_FORMAT_ARGB8888);

    if (!surface)
    {
        QLOG(LOG_ERROR, "Unable to create Skia image surface");
    }
    return new GskSkSurfaceWrapper(surface, data);
}

void GskWaylandDisplay::removeCursorSurfaceCache(GskSkSurfaceWrapper *wrapped)
{
    for (auto itr = fCursorSkSurfaceCache.begin(); itr != fCursorSkSurfaceCache.end(); itr++)
    {
        if (itr->second == wrapped)
        {
            fCursorSkSurfaceCache.erase(itr);
            return;
        }
    }
}

void GskWaylandDisplay::updateScale()
{
    // TODO(surface-wayland): update scale for each toplevel surface (gdkdisplay-wayland.c:2242)

    for (const Handle<GskSeat>& seat : this->getSeats())
    {
        const auto& seat_ = std::dynamic_pointer_cast<GskWaylandSeat>(seat);
        seat_->updateCursorScale();
    }
}

void GskWaylandDisplay::removeMaybeOutput(uint32_t id)
{
    /* copy list because this->removeMonitor() changes the list and may
     * occur failure to the foreach loop. */
    std::list<Handle<GskMonitor>> monitors = this->getMonitor();
    for (const Handle<GskMonitor>& mon : monitors)
    {
        const auto& mon_ = std::dynamic_pointer_cast<GskWaylandMonitor>(mon);
        if (mon_->getId() == id)
        {
            this->removeMonitor(mon);
            mon_->invalidateFromDisplay();
            updateScale();
            break;
        }
    }
}

Handle<GskSurface>
GskWaylandDisplay::onCreateSurface(SurfaceType type,
                                   const Handle<GskSurface>& parent,
                                   Vec2i pos,
                                   Vec2i size)
{
    // TODO(surface): implement this (gdksurface-wayland.c:801)
    return nullptr;
}

GSK_NAMESPACE_END
