#ifndef COCOA_EXPORTS_H
#define COCOA_EXPORTS_H

#include "Core/EventSource.h"
#include "Koi/bindings/Base.h"
#include "Koi/Runtime.h"
#include "Koi/binder/Class.h"

KOI_BINDINGS_NS_BEGIN

class CoreBinding : public BindingBase
{
public:
    CoreBinding();
    ~CoreBinding() override;
    const char *getUniqueId() override;
    void getModule(binder::Module& self) override;
    void setInstanceProperties(v8::Local<v8::Object> instance) override;
    const char **getExports() override;
};

void corePrint(const std::string& str);
v8::Local<v8::Value> coreDelay(uint64_t timeout);
v8::Local<v8::Value> coreGetProperty(const std::string& spec);
v8::Local<v8::Value> coreEnumeratePropertyNode(const std::string& spec);
bool coreHasProperty(const std::string& spec);

int32_t coreOpen(const std::string& path, const std::string& flags, int32_t mode);
int32_t coreOpenAt(int32_t dirfd, const std::string& path, const std::string& flags, int32_t mode);
void coreClose(int32_t fd);
int64_t coreSeek(int32_t fd, int32_t whence, int64_t offset);
void coreRename(const std::string& oldpath, const std::string& newpath);
void coreTruncate(const std::string& path, size_t length);
void coreFTruncate(int32_t fd, size_t length);
void coreMknod(const std::string& path, int32_t mode, int32_t dev);
void coreMknodAt(int32_t dirfd, const std::string& path, int32_t mode, int32_t dev);

void coreDump(const std::string& what);
void coreExit();

v8::Local<v8::Array> GetEscapableArgs();

class CoreTimer;

/**
 * If `setInterval()` returns an CoreTimer object directly,
 * it may be destroyed by V8's garbage collector. So we hold
 * CoreTimer instance in C++ and expose interfaces to JavaScript
 * by this proxy class.
 */
class CoreTimerProxy
{
public:
    explicit CoreTimerProxy();
    ~CoreTimerProxy();
    static binder::Class<CoreTimerProxy> GetClass();
    void setInterval(uint64_t timeout, int64_t repeat, v8::Local<v8::Value> cbValue);
    void setTimeout(uint64_t timeout, v8::Local<v8::Value> cbValue);
    void stop();
private:
    CoreTimer *fAttachedTimer;
};

class Buffer
{
public:
    explicit Buffer(const v8::FunctionCallbackInfo<v8::Value>& info);
    ~Buffer();

    static binder::Class<Buffer> GetClass();

    size_t length();
    uint8_t byteAt(int64_t idx);
    v8::Local<v8::Value> copy(const v8::FunctionCallbackInfo<v8::Value>& args);
    v8::Local<v8::Value> toDataView(const v8::FunctionCallbackInfo<v8::Value>& args);
    v8::Local<v8::Value> toString(const std::string& coding);
    static size_t GetEncodedSize(v8::Local<v8::Value> str, const std::string& encoding);

private:
    uint8_t *getWriteableDataPointerByte();

    v8::Global<v8::Uint8Array>  fArray;
    std::shared_ptr<v8::BackingStore> fBackingStore;
};

class FileHandle
{
};

KOI_BINDINGS_NS_END
#endif //COCOA_EXPORTS_H
