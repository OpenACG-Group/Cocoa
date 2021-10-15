#ifndef COCOA_RENDERER_H
#define COCOA_RENDERER_H

#include <future>
#include <functional>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <thread>
#include <atomic>

#include "Vanilla/Base.h"
#include "Vanilla/RenderKit/LayerFactories.h"
#include "Vanilla/RenderKit/RenderMicroTask.h"
VANILLA_NS_BEGIN

class DrawContext;
class ContentAggregator;
class Layer;

class Renderer
{
public:
    struct Command
    {
        using LayerOperationCb = std::function<bool(const Handle<Layer>&)>;

        struct LayerOperationGroup
        {
            LayerOperationCb op;
            uint32_t id;
            // private, filled by scheduler
            Handle<Layer> _layer = nullptr;
        };

        enum class Verb
        {
            kUnknown,
            kActivatePictureLayer,
            kPresent,
            kOperateLayer,
            kOperateLayersConcurrently,
            kSchedEnterDeferring,
            kSchedLeaveDeferring,
            /* Wait until the renderer thread becomes idle.
             * Then force scheduler to enter deferring mode. */
            kSchedSpinWaitIdleAndDeferring
        };

        class Result
        {
        public:
            enum class Type
            {
                kVoid,
                kBoolean
            };

            enum class Status
            {
                kRefused,
                kFinished,
                kBadCommand,
                kDropped
            };

            explicit Result(Status status)
                : fStatus(status), fType(Type::kVoid), fValueBoolean(false) {}
            Result(Status status, bool value)
                : fStatus(status), fType(Type::kBoolean), fValueBoolean(value) {}
            ~Result() = default;

            va_nodiscard inline Type getType() const {
                return fType;
            }

            va_nodiscard inline Status getStatus() const {
                return fStatus;
            }

            va_nodiscard inline bool toBoolean() const {
                return fValueBoolean;
            }

        private:
            Status      fStatus;
            Type        fType;
            bool        fValueBoolean;
        };

        Command();
        Command(const Command&) = delete;

        Command(Command&& rhs) noexcept
            : verb(rhs.verb)
            , layerId(rhs.layerId)
            , resolvedLayer(std::move(rhs.resolvedLayer))
            , op(std::move(rhs.op))
            , promise(std::move(rhs.promise))
            , sequence(rhs.sequence)
            , presentRegion(rhs.presentRegion)
            , layerOperationGroup(std::move(rhs.layerOperationGroup))
            , extend{0, 0, 0, 0} {}

        ~Command() = default;

        Verb                    verb;
        uint32_t                layerId;            // Filled by caller
        Handle<Layer>           resolvedLayer;      // Filled by scheduler
        LayerOperationCb        op;
        std::promise<Result>    promise;
        uint32_t                sequence;
        SkRect                  presentRegion;
        std::optional<std::vector<LayerOperationGroup>>
                                layerOperationGroup;
        uint8_t                 extend[4];
    };

    class CommandQueue
    {
    public:
        CommandQueue() : fThreadStop(false) {}
        ~CommandQueue() = default;

        va_nodiscard size_t size() {
            std::unique_lock<std::mutex> lock(fMutex);
            return fQueue.size();
        }

        va_nodiscard bool isEmpty() {
            return fQueue.empty();
        }

        void stop() {
            {
                std::unique_lock<std::mutex> lock(fMutex);
                fThreadStop = true;
            }
            fCond.notify_one();
        }

        void push(Command&& cmd) {
            {
                std::unique_lock<std::mutex> lock(fMutex);
                fQueue.emplace(std::forward<Command>(cmd));
            }
            fCond.notify_one();
        }

        va_nodiscard inline std::optional<Command> waitPop() {
            std::unique_lock<std::mutex> lock(fMutex);
            fCond.wait(lock, [this] { return fThreadStop || !fQueue.empty(); });
            if (fThreadStop)
                return {};
            Command cmd(std::move(fQueue.front()));
            fQueue.pop();
            return std::make_optional<Command>(std::move(cmd));
        }

    private:
        std::queue<Command>         fQueue;
        std::condition_variable     fCond;
        std::mutex                  fMutex;
        bool                        fThreadStop;
    };

    enum class SchedulerStatus
    {
        kDeferring,
        kNormal,
        kNotRunning
    };

    using FutureResult = std::future<Command::Result>;

    explicit Renderer(const Handle<DrawContext>& drawContext);
    ~Renderer();

    va_nodiscard inline Handle<ContentAggregator> getAggregator() {
        return fAggregator;
    }

    va_nodiscard inline SchedulerStatus getSchedulerStatus() {
        return fSchedStatus.load();
    }

    void dispose();

    uint32_t pushLayer(const LayerFactory& factory);
    uint32_t insertLayer(uint32_t before, const LayerFactory& factory);
    void removeLayer(uint32_t id);

    FutureResult cmdActivatePictureLayer(uint32_t id);
    FutureResult cmdPresent(const SkRect& region);
    FutureResult cmdOperateLayer(const Command::LayerOperationCb& op, uint32_t id);
    FutureResult cmdOperateLayersConcurrently(const std::vector<Command::LayerOperationGroup>& groups);
    FutureResult cmdSchedSpinWaitIdleAndDeferring();
    FutureResult cmdSchedEnterDeferring();
    FutureResult cmdSchedLeaveDeferring(bool dropCmds);

private:
    void schedulerWaitForRenderIdle();
    void schedulerHandleCommand(Command& cmd, std::queue<Command>& deferredCmdQueue);
    void schedulerHandleDeferredQueue(Command& cmd, std::queue<Command>& deferredCmdQueue);
    void schedulerOperateLayersConcurrently(Command& cmd);
    void schedulerThread();
    void renderThread();

    Handle<ContentAggregator>   fAggregator;
    bool                        fDisposed;
    CommandQueue                fRendererCommandQueue;
    CommandQueue                fSchedulerCommandQueue;
    std::thread                 fRendererThread;
    std::thread                 fSchedulerThread;
    std::atomic<uint32_t>       fRendererCurrentCmdSeq;
    std::atomic<SchedulerStatus> fSchedStatus;
    RenderMicroTask             fMicroTasks;
};

VANILLA_NS_END
#endif //COCOA_RENDERER_H
