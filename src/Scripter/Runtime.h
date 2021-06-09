#ifndef COCOA_RUNTIME_H
#define COCOA_RUNTIME_H

#include <memory>

#include "include/v8.h"
#include "Core/UniquePersistent.h"
#include "Scripter/ScripterBase.h"
#include "Scripter/AsyncOpTask.h"
#include "Scripter/AsyncWorker.h"
SCRIPTER_NS_BEGIN

#define CHECKED(E)  E.ToLocalChecked()

#define ISOLATE_DATA_SLOT_RUNTIME_PTR       0

void Initialize();
void Dispose();

class Runtime
{
public:
    struct Options
    {
        Options();
        uint32_t    v8_platform_thread_pool;
    };

    Runtime(std::unique_ptr<v8::Platform> platform,
            v8::ArrayBuffer::Allocator *allocator,
            v8::Isolate *isolate,
            v8::Global<v8::Context> context);
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    ~Runtime();

    static std::shared_ptr<Runtime> MakeFromSnapshot(const std::string& snapshotFile,
                                                     const std::string& icuDataFile,
                                                     const Options& options = Options());


    inline v8::Local<v8::Context> context()
    { return fContext.Get(fIsolate); }

    inline v8::Isolate *isolate()
    { return fIsolate; }

    v8::Local<v8::Value> execute(const char *str);
    v8::Local<v8::Value> execute(const char *scriptName, const char *str);

    void runPromisesCheckpoint();

private:
    std::unique_ptr<v8::Platform>   fPlatform;
    v8::ArrayBuffer::Allocator     *fArrayBufferAllocator;
    v8::Isolate                    *fIsolate;
    v8::Global<v8::Context>         fContext;
};

SCRIPTER_NS_END
#endif //COCOA_RUNTIME_H
