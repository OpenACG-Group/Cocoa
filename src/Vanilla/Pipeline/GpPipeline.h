#ifndef COCOA_GPPIPELINE_H
#define COCOA_GPPIPELINE_H

#include <cassert>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <optional>

#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class GpElement;
class GpLinkage;

class GpPipeline
{
public:
    explicit GpPipeline(int numThreads);
    ~GpPipeline();

    static Handle<GpPipeline> Make(int numThreads = 0);

    template<typename T, typename...ArgsT>
    T *newElement(ArgsT&&...args)
    {
        auto ptr = new T(std::forward<ArgsT>(args)...);
        assert(ptr);
        addElement(ptr);
        return ptr;
    }

    void link(GpElement *src, int pad0, GpElement *dst, int pad1);
    bool instantiate();

    va_nodiscard inline bool isInstantiated() const
    { return fInstantiated; }

    bool hasElement(GpElement *element);
    void process();

private:
    void addElement(GpElement *element);
    int getIndexByElement(GpElement *element);
    void enqueueElement(GpElement *element);
    void notifySchedulerFinish(GpElement *element);
    bool checkMutuallyExclusive(GpElement *element);

    void schedulerRoutine();
    void workerRoutine();

    std::vector<GpElement*>         fElements;
    std::vector<GpElement*>         fSrcElements;
    std::vector<GpElement*>         fSinkElements;
    std::vector<GpLinkage*>         fLinkages;

    /* Scheduler thread */
    std::optional<std::thread>      fSchedThread;
    std::condition_variable         fWorkerFinishNotify;
    std::mutex                      fWorkerFinishMutex;
    std::queue<GpElement*>          fWorkerFinishQueue;

    /* Workers thread pool */
    int                             fNumWorkers;
    std::vector<std::thread>        fThreads;
    std::mutex                      fQueueMutex;
    std::condition_variable         fQueueCond;
    std::queue<GpElement*>          fQueue;
    bool                            fThreadsStop;

    bool                            fInstantiated;
};

VANILLA_NS_END
#endif //COCOA_GPPIPELINE_H
