#include "Core/Errors.h"

#include "Vanilla/RenderKit/Renderer.h"
#include "Vanilla/RenderKit/ContentAggregator.h"
#include "Vanilla/RenderKit/Layer.h"
#include "Vanilla/RenderKit/PictureLayer.h"
#include "Core/Journal.h"
VANILLA_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Vanilla.RenderKit)

namespace {

uint32_t gCmdSequenceCounter = 1;
#define CMD_SEQ_IMPOSSIBLE_VALUE 0

} // namespace anonymous

Renderer::Command::Command()
    : verb(Command::Verb::kUnknown)
    , layerId(0)
    , sequence(gCmdSequenceCounter++)
    , presentRegion(SkRect::MakeEmpty())
    , extend{0, 0, 0, 0}
{
}

Renderer::Renderer(const Handle<DrawContext>& drawContext)
    : fAggregator(ContentAggregator::Make(drawContext))
    , fDisposed(false)
    , fRendererCurrentCmdSeq(CMD_SEQ_IMPOSSIBLE_VALUE)
    , fSchedStatus(SchedulerStatus::kNotRunning)
    , fMicroTasks(4)
{
    CHECK(fAggregator);

    fRendererThread = std::thread(&Renderer::renderThread, this);
    fSchedulerThread = std::thread(&Renderer::schedulerThread, this);
}

Renderer::~Renderer()
{
    this->dispose();
}

void Renderer::dispose()
{
    if (!fDisposed)
    {
        fDisposed = true;
        /* Scheduler MUST be cancelled before Renderer
         * otherwise Scheduler may block in schedulerWaitForRenderIdle */
        if (fSchedulerThread.joinable())
        {
            fSchedulerCommandQueue.stop();
            fSchedulerThread.join();
        }
        if (fRendererThread.joinable())
        {
            fRendererCommandQueue.stop();
            fRendererThread.join();
        }
        fMicroTasks.dispose();
        fAggregator->dispose();
    }
}

void Renderer::renderThread()
{
#ifdef __linux__
    ::pthread_setname_np(::pthread_self(),
                         "Renderer");
#endif /* __linux__ */

    while (true)
    {
        std::optional<Command> maybeCmd(fRendererCommandQueue.waitPop());
        if (!maybeCmd)
            break;
        Command cmd(std::move(maybeCmd.value()));
        fRendererCurrentCmdSeq.store(cmd.sequence);

        switch (cmd.verb)
        {
        default:
            QLOG(LOG_WARNING, "Invalid command verb");
            cmd.promise.set_value(Command::Result(Command::Result::Status::kRefused));
            break;

        case Command::Verb::kActivatePictureLayer:
            PictureLayer::Cast(cmd.resolvedLayer)->activate();
            cmd.promise.set_value(Command::Result(Command::Result::Status::kFinished));
            break;

        case Command::Verb::kPresent:
            fAggregator->update(cmd.presentRegion);
            cmd.promise.set_value(Command::Result(Command::Result::Status::kFinished));
            break;

        case Command::Verb::kOperateLayer:
            bool ret = cmd.op(cmd.resolvedLayer);
            cmd.promise.set_value(Command::Result(Command::Result::Status::kFinished, ret));
            break;
        }
        fRendererCurrentCmdSeq.store(CMD_SEQ_IMPOSSIBLE_VALUE);
    }
}

void Renderer::schedulerThread()
{
#ifdef __linux__
    ::pthread_setname_np(::pthread_self(),
                         "RenderScheduler");
#endif /* __linux__ */

    fSchedStatus.store(SchedulerStatus::kNormal);
    std::queue<Command> deferredCmdQueue;
    while (true)
    {
        std::optional<Command> maybeCmd(fSchedulerCommandQueue.waitPop());
        if (!maybeCmd)
            break;
        Command cmd(std::move(maybeCmd.value()));

        if (fSchedStatus.load() == SchedulerStatus::kDeferring)
        {
            if (cmd.verb == Command::Verb::kSchedEnterDeferring
                || cmd.verb == Command::Verb::kSchedSpinWaitIdleAndDeferring)
            {
                /* Scheduler has already been in deferring mode */
                cmd.promise.set_value(Command::Result(Command::Result::Status::kRefused));
                continue;
            }
            if (cmd.verb != Command::Verb::kSchedLeaveDeferring)
            {
                deferredCmdQueue.emplace(std::move(cmd));
                continue;
            }
        }
        schedulerHandleCommand(cmd, deferredCmdQueue);
    }
    fSchedStatus.store(SchedulerStatus::kNotRunning);
}

void Renderer::schedulerHandleCommand(Command& cmd, std::queue<Command>& deferredCmdQueue)
{
    bool passToRenderer = false;
    switch (cmd.verb)
    {
    case Command::Verb::kUnknown:
        QLOG(LOG_WARNING, "Invalid command verb (kUnknown)");
        break;

    case Command::Verb::kActivatePictureLayer:
    case Command::Verb::kOperateLayer:
    {
        Handle<Layer> layer = fAggregator->getLayerById(cmd.layerId);
        if (!layer)
        {
            cmd.promise.set_value(Command::Result(Command::Result::Status::kBadCommand));
            break;
        }
        if (cmd.verb == Command::Verb::kActivatePictureLayer && !PictureLayer::Cast(layer))
        {
            cmd.promise.set_value(Command::Result(Command::Result::Status::kBadCommand));
            break;
        }
        cmd.resolvedLayer = layer;
        passToRenderer = true;
        break;
    }

    case Command::Verb::kOperateLayersConcurrently:
        schedulerOperateLayersConcurrently(cmd);
        break;

    case Command::Verb::kPresent:
        if (cmd.presentRegion.isEmpty() || !cmd.presentRegion.isFinite())
        {
            cmd.promise.set_value(Command::Result(Command::Result::Status::kBadCommand));
            break;
        }
        passToRenderer = true;
        break;

    case Command::Verb::kSchedEnterDeferring:
        fSchedStatus.store(SchedulerStatus::kDeferring);
        cmd.promise.set_value(Command::Result(Command::Result::Status::kFinished));
        break;

    case Command::Verb::kSchedLeaveDeferring:
        fSchedStatus.store(SchedulerStatus::kNormal);
        schedulerHandleDeferredQueue(cmd, deferredCmdQueue);
        cmd.promise.set_value(Command::Result(Command::Result::Status::kFinished));
        break;

    case Command::Verb::kSchedSpinWaitIdleAndDeferring:
        schedulerWaitForRenderIdle();
        fSchedStatus.store(SchedulerStatus::kDeferring);
        cmd.promise.set_value(Command::Result(Command::Result::Status::kFinished));
        break;
    }

    if (passToRenderer)
    {
        fRendererCommandQueue.push(std::move(cmd));
    }
}

void Renderer::schedulerOperateLayersConcurrently(Command& cmd)
{
    schedulerWaitForRenderIdle();
    for (Command::LayerOperationGroup& group : cmd.layerOperationGroup.value())
    {
        Handle<Layer> layer = fAggregator->getLayerById(group.id);
        if (!layer)
        {
            cmd.promise.set_value(Command::Result(Command::Result::Status::kBadCommand));
            return;
        }
        group._layer = layer;
    }

    std::vector<RenderMicroTask::MTask::Future> futures(cmd.layerOperationGroup->size());
    uint32_t cnt = 0;
    for (Command::LayerOperationGroup& group : cmd.layerOperationGroup.value())
    {
        Handle<Layer> layer(std::move(group._layer));
        Command::LayerOperationCb callback(std::move(group.op));
        futures[cnt++] = fMicroTasks.enqueue([callback, layer]() {
            return callback(layer);
        });
    }

    bool result = true;
    for (RenderMicroTask::MTask::Future& future : futures)
    {
        result = result && future.get();
    }
    cmd.promise.set_value(Command::Result(Command::Result::Status::kFinished, result));
}

void Renderer::schedulerWaitForRenderIdle()
{
    while (!fRendererCommandQueue.isEmpty() || fRendererCurrentCmdSeq.load() != CMD_SEQ_IMPOSSIBLE_VALUE)
        ;
}

void Renderer::schedulerHandleDeferredQueue(Command& cmd, std::queue<Command>& deferredCmdQueue)
{
    if (cmd.extend[0])
    {
        while (!deferredCmdQueue.empty())
        {
            deferredCmdQueue.front().promise.set_value(Command::Result(Command::Result::Status::kDropped));
            deferredCmdQueue.pop();
        }
        return;
    }

    while (!deferredCmdQueue.empty())
    {
        Command deferredCmd(std::move(deferredCmdQueue.front()));
        deferredCmdQueue.pop();
        schedulerHandleCommand(deferredCmd, deferredCmdQueue);
    }
}

class SchedulerDeferringScope
{
public:
    explicit SchedulerDeferringScope(Renderer& renderer, bool drop = false)
        : fRenderer(renderer)
        , fDrop(drop) {
        fRenderer.cmdSchedSpinWaitIdleAndDeferring().wait();
    }

    ~SchedulerDeferringScope() {
        fRenderer.cmdSchedLeaveDeferring(fDrop);
    }

private:
    Renderer&   fRenderer;
    bool        fDrop;
};

uint32_t Renderer::pushLayer(const LayerFactory& factory)
{
    SchedulerDeferringScope scope(*this);
    Handle<Layer> layer = fAggregator->pushLayer(factory);
    CHECK(layer);
    return layer->getLayerId();
}

uint32_t Renderer::insertLayer(uint32_t before, const LayerFactory& factory)
{
    SchedulerDeferringScope scope(*this);
    Handle<Layer> layer = fAggregator->insertLayerBefore(before, factory);
    CHECK(layer);
    return layer->getLayerId();
}

void Renderer::removeLayer(uint32_t id)
{
    SchedulerDeferringScope scope(*this, true);
    Handle<Layer> layer = fAggregator->getLayerById(id);
    if (!layer)
    {
        throw VanillaException(__func__,
                               fmt::format("Failed to acquire layer by ID #{}", id));
    }
    fAggregator->removeLayer(layer);
}

Renderer::FutureResult Renderer::cmdActivatePictureLayer(uint32_t id)
{
    Command cmd;
    cmd.verb = Command::Verb::kActivatePictureLayer;
    cmd.layerId = id;
    FutureResult future = cmd.promise.get_future();
    fSchedulerCommandQueue.push(std::move(cmd));
    return future;
}

Renderer::FutureResult Renderer::cmdPresent(const SkRect& region)
{
    Command cmd;
    cmd.verb = Command::Verb::kPresent;
    cmd.presentRegion = region;
    FutureResult future = cmd.promise.get_future();
    fSchedulerCommandQueue.push(std::move(cmd));
    return future;
}

Renderer::FutureResult Renderer::cmdOperateLayer(const Command::LayerOperationCb& op, uint32_t id)
{
    Command cmd;
    cmd.verb = Command::Verb::kOperateLayer;
    cmd.layerId = id;
    cmd.op = op;
    FutureResult future = cmd.promise.get_future();
    fSchedulerCommandQueue.push(std::move(cmd));
    return future;
}

Renderer::FutureResult Renderer::cmdSchedSpinWaitIdleAndDeferring()
{
    Command cmd;
    cmd.verb = Command::Verb::kSchedSpinWaitIdleAndDeferring;
    FutureResult future = cmd.promise.get_future();
    fSchedulerCommandQueue.push(std::move(cmd));
    return future;
}

Renderer::FutureResult Renderer::cmdSchedEnterDeferring()
{
    Command cmd;
    cmd.verb = Command::Verb::kSchedEnterDeferring;
    FutureResult future = cmd.promise.get_future();
    fSchedulerCommandQueue.push(std::move(cmd));
    return future;
}

Renderer::FutureResult Renderer::cmdSchedLeaveDeferring(bool dropCmds)
{
    Command cmd;
    cmd.verb = Command::Verb::kSchedLeaveDeferring;
    cmd.extend[0] = static_cast<uint8_t>(dropCmds);
    FutureResult future = cmd.promise.get_future();
    fSchedulerCommandQueue.push(std::move(cmd));
    return future;
}

Renderer::FutureResult Renderer::cmdOperateLayersConcurrently(const std::vector<Command::LayerOperationGroup>& groups)
{
    Command cmd;
    cmd.verb = Command::Verb::kOperateLayersConcurrently;
    cmd.layerOperationGroup = groups;
    FutureResult future = cmd.promise.get_future();
    fSchedulerCommandQueue.push(std::move(cmd));
    return future;
}

VANILLA_NS_END
