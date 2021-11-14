#ifndef COCOA_COREBINDING_H
#define COCOA_COREBINDING_H

#include "Core/EventSource.h"
#include "Koi/lang/Base.h"
#include "Koi/Runtime.h"
#include "Koi/binder/Class.h"

KOI_LANG_NS_BEGIN

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

void jni_core_print(const std::string& str);
v8::Local<v8::Value> jni_core_delay(uint64_t timeout);
v8::Local<v8::Value> jni_core_getProperty(const std::string& spec);
v8::Local<v8::Value> jni_core_enumeratePropertyNode(const std::string& spec);
bool jni_core_hasProperty(const std::string& spec);
int32_t jni_core_open(const v8::FunctionCallbackInfo<v8::Value>& info);
void jni_core_close(int32_t fd);
void jni_core_dump(const std::string& what);
void jni_core_exit();

class jni_core_Timer : public TimerSource
{
public:
    jni_core_Timer();
    ~jni_core_Timer() override = default;

    static binder::Class<jni_core_Timer> GetClass();

    void setInterval(uint64_t timeout, int64_t repeat, v8::Local<v8::Value> cbValue);
    void setTimeout(uint64_t timeout, v8::Local<v8::Value> cbValue);
    void stop();

private:
    KeepInLoop timerDispatch() override;

    v8::Global<v8::Function> fCallback;
};

KOI_LANG_NS_END
#endif //COCOA_COREBINDING_H
