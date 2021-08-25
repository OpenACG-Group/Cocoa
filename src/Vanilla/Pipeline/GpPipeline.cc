#include <cassert>
#include <thread>
#include <mutex>
#include <optional>
#include <condition_variable>

#include "Core/Journal.h"
#include "Vanilla/Base.h"
#include "Vanilla/Pipeline/GpPipeline.h"
#include "Vanilla/Pipeline/GpElement.h"
#include "Vanilla/Pipeline/GpLinkage.h"
VANILLA_NS_BEGIN

GpPipeline::GpPipeline(int numThreads)
    : fNumWorkers(numThreads)
    , fThreadsStop(false)
    , fInstantiated(false)
{
    if (fNumWorkers <= 0)
        fNumWorkers = static_cast<int>(std::thread::hardware_concurrency());
}

GpPipeline::~GpPipeline()
{
    {
        std::scoped_lock<std::mutex> lock(fQueueMutex);
        fThreadsStop = true;
    }
    fWorkerFinishNotify.notify_all();
    if (fSchedThread->joinable())
        fSchedThread->join();

    fQueueCond.notify_all();
    for (auto& worker : fThreads)
    {
        if (worker.joinable())
            worker.join();
    }

    for (GpElement *element : fElements)
        delete element;
    for (GpLinkage *linkage : fLinkages)
        delete linkage;
}

Handle<GpPipeline> GpPipeline::Make(int numThreads)
{
    return std::make_shared<GpPipeline>(numThreads);
}

bool GpPipeline::hasElement(GpElement *element)
{
    if (!element)
        return false;
    return std::find(fElements.begin(), fElements.end(), element) != fElements.end();
}

void GpPipeline::addElement(GpElement *element)
{
    if (hasElement(element))
        return;
    fElements.push_back(element);
    if (element->getType() == GpElement::Type::kSrc)
        fSrcElements.push_back(element);
    else if (element->getType() == GpElement::Type::kSink)
        fSinkElements.push_back(element);
}

void GpPipeline::link(GpElement *src, int pad0, GpElement *dst, int pad1)
{
    assert(src && dst);
    assert(hasElement(src) && hasElement(dst));

    if (pad0 < 0 || pad1 < 0 || pad0 >= src->getOutPads() || pad1 >= dst->getInPads())
        throw VanillaException(__func__, "Bad linkage: Pad number is overrange");

    if (src->getOutPadsCategories()[pad0] != dst->getInPadsCategories()[pad1])
        throw VanillaException(__func__, "Bad linkage: Pad transfer categories does not fit");

    auto linkage = new GpLinkage(src, pad0, dst, pad1);
    fLinkages.push_back(linkage);
    src->setLinkageForOutPad(pad0, linkage);
    dst->setLinkageForInPad(pad1, linkage);
}

int GpPipeline::getIndexByElement(GpElement *element)
{
    size_t idx =  std::distance(fElements.begin(),
                                std::find(fElements.begin(), fElements.end(), element));
    return static_cast<int>(idx);
}

bool GpPipeline::instantiate()
{
    // TODO: Check and instantiate the pipeline DAG

    for (int i = 0; i < fNumWorkers; i++)
    {
        fThreads.emplace_back(&GpPipeline::workerRoutine, this);
        ::pthread_setname_np(fThreads.back().native_handle(),
                             fmt::format("GpPipeline#{}", i).c_str());
    }
    fSchedThread = std::make_optional<std::thread>(&GpPipeline::schedulerRoutine, this);
    ::pthread_setname_np(fSchedThread->native_handle(), "GpScheduler");
    fInstantiated = true;
    return fInstantiated;
}

void GpPipeline::notifySchedulerFinish(GpElement *element)
{
    {
        std::scoped_lock<std::mutex> lock(fWorkerFinishMutex);
        fWorkerFinishQueue.push(element);
    }
    fWorkerFinishNotify.notify_one();
}

bool GpPipeline::checkMutuallyExclusive(GpElement *element)
{
    std::scoped_lock<std::mutex> lock(fQueueMutex);
    for (auto& p : fElements)
    {
        GpElement::State state = p->getState();
        if ((state == GpElement::State::kRunning ||
            state == GpElement::State::kQueued) &&
            element != p)
        {
            if (element->isMutuallyExclusiveInConcurrencyWith(p))
                return false;
        }
    }
    return true;
}

void GpPipeline::schedulerRoutine()
{
    std::vector<bool> deferred(fElements.size(), false);
    std::vector<int> padsReached(fElements.size(), 0);

    while (true)
    {
        GpElement *finishedElement;
        {
            std::unique_lock lock(fWorkerFinishMutex);
            fWorkerFinishNotify.wait(lock, [this] { return fThreadsStop || !fWorkerFinishQueue.empty(); });
            if (fThreadsStop)
                break;
            finishedElement = fWorkerFinishQueue.front();
            fWorkerFinishQueue.pop();
        }

        for (int i = 0; i < static_cast<int>(deferred.size()); i++)
        {
            if (deferred[i] && checkMutuallyExclusive(fElements[i]))
            {
                enqueueElement(fElements[i]);
                deferred[i] = false;
            }
        }

        int outPads = finishedElement->getOutPads();
        for (int i = 0; i < outPads; i++)
        {
            GpElement *next = finishedElement->getLinkageForOutPad(i)->getDstElement();
            int idx = getIndexByElement(next);
            if (deferred[idx])
                continue;

            ++padsReached[idx];
            if (padsReached[idx] == next->getInPads())
            {
                padsReached[idx] = 0;
                if (checkMutuallyExclusive(next))
                    enqueueElement(next);
                else
                    deferred[idx] = true;
            }
        }
    }
}

void GpPipeline::workerRoutine()
{
    while (true)
    {
        GpElement *element;
        {
            std::unique_lock<std::mutex> lock(fQueueMutex);
            fQueueCond.wait(lock, [this] { return fThreadsStop || !fQueue.empty(); });
            if (fThreadsStop && fQueue.empty())
                break;
            element = fQueue.front();
            fQueue.pop();
        }
        element->setState(GpElement::State::kRunning);
        element->process();
        element->setState(GpElement::State::kNormal);
        notifySchedulerFinish(element);
    }
}

void GpPipeline::enqueueElement(GpElement *element)
{
    {
        std::scoped_lock<std::mutex> lock(fQueueMutex);
        fQueue.push(element);
        element->setState(GpElement::State::kQueued);
    }
    fQueueCond.notify_one();
}

void GpPipeline::process()
{
    for (GpElement *element : fSrcElements)
        enqueueElement(element);
}

VANILLA_NS_END
