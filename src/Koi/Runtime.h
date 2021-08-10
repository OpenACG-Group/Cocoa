#ifndef COCOA_RUNTIME_H
#define COCOA_RUNTIME_H

#include <memory>
#include <map>

#include "include/v8.h"
#include "Core/UniquePersistent.h"
#include "Core/EventSource.h"
#include "Core/Exception.h"
#include "Koi/KoiBase.h"
#include "Koi/Ops.h"
#include "Koi/ResourceDescriptorPool.h"
KOI_NS_BEGIN

#define CHECKED(E)  E.ToLocalChecked()

#define ISOLATE_DATA_SLOT_RUNTIME_PTR       0

void Initialize();
void Dispose();

class Runtime : public LoopPrologueSource
{
public:
    struct Options
    {
        Options();
        uint32_t    v8_platform_thread_pool;
    };

    Runtime(EventLoop *loop,
            std::unique_ptr<v8::Platform> platform,
            v8::ArrayBuffer::Allocator *allocator,
            v8::Isolate *isolate,
            v8::Global<v8::Context> context);
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    ~Runtime() override;

    static std::shared_ptr<Runtime> MakeFromSnapshot(EventLoop *loop,
                                                     const std::string& snapshotFile,
                                                     const std::string& icuDataFile = std::string(),
                                                     const Options& options = Options());


    inline v8::Local<v8::Context> context()
    { return fContext.Get(fIsolate); }

    inline v8::Isolate *isolate()
    { return fIsolate; }

    inline ResourceDescriptorPool& resourcePool()
    { return *fResourcePool; }

    v8::Local<v8::Value> execute(const char *str);
    v8::Local<v8::Value> execute(const char *scriptName, const char *str);

    v8::Local<v8::Module> compileModule(const std::string& url);
    v8::Local<v8::Module> compileModule(const std::string& refererUrl, const std::string& url);
    v8::MaybeLocal<v8::Value> evaluateModule(const std::string& url);

    void takeOverNativeException(const RuntimeException& exception);

private:
    KeepInLoop loopPrologueDispatch() override;

    std::unique_ptr<v8::Platform>   fPlatform;
    v8::ArrayBuffer::Allocator     *fArrayBufferAllocator;
    v8::Isolate                    *fIsolate;
    v8::Global<v8::Context>         fContext;
    std::map<std::string, v8::Global<v8::Module>>
                                    fModuleCache;
    ResourceDescriptorPool         *fResourcePool;
};

KOI_NS_END
#endif //COCOA_RUNTIME_H
