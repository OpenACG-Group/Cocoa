#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Koi/VMIntrospect.h"
#include "Koi/binder/Convert.h"
#include "Koi/binder/ThrowExcept.h"
#include "Koi/binder/CallV8.h"
#include "Koi/BindingManager.h"
#include "Koi/bindings/Base.h"

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Koi.Introspect)

KOI_NS_BEGIN

namespace {

VMIntrospect *get_bare_introspect_ptr(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    void *ptr = info.This()->GetAlignedPointerFromInternalField(0);
    CHECK(ptr);
    return reinterpret_cast<VMIntrospect*>(ptr);
}

template<VMIntrospect::CallbackSlot kSlot>
void introspect_set_callback_slot(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    VMIntrospect *introspect = get_bare_introspect_ptr(args);
    JS_THROW_IF(args.Length() != 1, "Invalid number of arguments", v8::Exception::Error);
    JS_THROW_IF(!args[0]->IsFunction(), "Callback is not a function", v8::Exception::TypeError);
    introspect->setCallbackSlot(kSlot, args[0].template As<v8::Function>());
}

void introspect_load_shared_object(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    VMIntrospect *introspect = get_bare_introspect_ptr(args);
    JS_THROW_IF(args.Length() != 1, "Invalid number of arguments", v8::Exception::Error);
    JS_THROW_IF(!args[0]->IsString(), "Shared object path is not a string", v8::Exception::TypeError);
    auto path = binder::from_v8<std::string>(introspect->getIsolate(), args[0]);
    if (!Runtime::GetBareFromIsolate(introspect->getIsolate())->getOptions().introspect_allow_loading_shared_object)
    {
        QLOG(LOG_WARNING,
             "JavaScript is trying to load shared object {}, which is forbidden by current introspect policy", path);
        JS_THROW_IF(true, "Loading shared object is forbidden by current introspect policy", v8::Exception::Error);
    }
    try {
        BindingManager::Instance()->loadDynamicObject(path);
    } catch (const std::exception& e) {
        JS_THROW_IF(true, e.what(), v8::Exception::Error);
    }
}

template<VMIntrospect::ScheduledTask::Type kType>
void introspect_schedule_task(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    VMIntrospect *introspect = get_bare_introspect_ptr(args);
    v8::Isolate *isolate = introspect->getIsolate();
    JS_THROW_IF(args.Length() < 1 || args.Length() > 3, "Invalid number of arguments", v8::Exception::Error);

    JS_THROW_IF(!args[0]->IsString(), "Script/module name is not a string", v8::Exception::TypeError);
    VMIntrospect::ScheduledTask task{};
    task.type = kType;
    task.param = binder::from_v8<std::string>(isolate, args[0]);

    if (args.Length() >= 2)
    {
        JS_THROW_IF(!args[1]->IsFunction(), "Callback is not a function", v8::Exception::TypeError);
        task.callback = v8::Global<v8::Function>(isolate, v8::Local<v8::Function>::Cast(args[1]));
    }
    if (args.Length() == 3)
    {
        JS_THROW_IF(!args[2]->IsFunction(), "Callback is not a function", v8::Exception::TypeError);
        task.reject = v8::Global<v8::Function>(isolate, v8::Local<v8::Function>::Cast(args[2]));
    }
    introspect->scheduledTaskEnqueue(std::move(task));
}

void introspect_schedule_script_eval(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    introspect_schedule_task<VMIntrospect::ScheduledTask::Type::kEvalScript>(args);
}

void introspect_schedule_module_eval(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    introspect_schedule_task<VMIntrospect::ScheduledTask::Type::kEvalModuleUrl>(args);
}

void introspect_print(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    JS_THROW_IF(args.Length() != 1, "Invalid number of arguments", v8::Exception::Error);
    JS_THROW_IF(!args[0]->IsString(), "Argument is not a string", v8::Exception::TypeError);
    v8::Isolate *isolate = get_bare_introspect_ptr(args)->getIsolate();
    std::printf("%s", binder::from_v8<std::string>(isolate, args[0]).c_str());
    std::fflush(stdout);
}

void introspect_has_synthetic_module(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    JS_THROW_IF(args.Length() != 1, "Invalid number of arguments", v8::Exception::Error);
    JS_THROW_IF(!args[0]->IsString(), "Argument is not a string", v8::Exception::TypeError);
    v8::Isolate *isolate = get_bare_introspect_ptr(args)->getIsolate();
    auto name = binder::from_v8<std::string>(isolate, args[0]);
    args.GetReturnValue().Set(BindingManager::Ref().search(name) != nullptr);
}

// NOLINTNEXTLINE
std::map<std::string, LogType> log_level_name_map = {
        { "debug", LOG_DEBUG },
        { "info", LOG_INFO },
        { "warning", LOG_WARNING },
        { "warn", LOG_WARNING },
        { "error", LOG_ERROR },
        { "err", LOG_ERROR },
        { "exception", LOG_EXCEPTION },
        { "except", LOG_EXCEPTION }
};

void introspect_write_journal(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = get_bare_introspect_ptr(args)->getIsolate();
    Runtime *rt = Runtime::GetBareFromIsolate(isolate);
    JS_THROW_IF(!rt->getOptions().introspect_allow_write_journal,
                "Writing to journal is forbidden by current introspect policy", v8::Exception::Error);
    JS_THROW_IF(args.Length() != 2, "Invalid number of arguments", v8::Exception::Error);
    JS_THROW_IF(!args[0]->IsString() || !args[1]->IsString(), "Arguments is not strings", v8::Exception::TypeError);
    auto level = binder::from_v8<std::string>(isolate, args[0]);
    JS_THROW_IF(!log_level_name_map.contains(level), "Unrecognized journal level string", v8::Exception::Error);
    auto content = binder::from_v8<std::string>(isolate, args[1]);
    QLOG(log_level_name_map[level], "{}", content);
}

} // namespace anonymous

std::unique_ptr<VMIntrospect> VMIntrospect::InstallGlobal(v8::Isolate *isolate)
{
    CHECK(isolate->InContext());
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    v8::Local<v8::Object> g = ctx->Global();

    v8::Local<v8::ObjectTemplate> object = v8::ObjectTemplate::New(isolate);
    object->SetInternalFieldCount(1);
    object->Set(isolate,
                "setUncaughtExceptionHandler",
                v8::FunctionTemplate::New(isolate, introspect_set_callback_slot<CallbackSlot::kUncaughtException>));
    object->Set(isolate,
                "setBeforeExitHandler",
                v8::FunctionTemplate::New(isolate, introspect_set_callback_slot<CallbackSlot::kBeforeExit>));
    object->Set(isolate,
                "loadSharedObject",
                v8::FunctionTemplate::New(isolate, introspect_load_shared_object));
    object->Set(isolate,
                "scheduleScriptEvaluate",
                v8::FunctionTemplate::New(isolate, introspect_schedule_script_eval));
    object->Set(isolate,
                "scheduleModuleUrlEvaluate",
                v8::FunctionTemplate::New(isolate, introspect_schedule_module_eval));
    object->Set(isolate,
                "print",
                v8::FunctionTemplate::New(isolate, introspect_print));
    object->Set(isolate,
                "writeToJournal",
                v8::FunctionTemplate::New(isolate, introspect_write_journal));
    object->Set(isolate,
                "hasSyntheticModule",
                v8::FunctionTemplate::New(isolate, introspect_has_synthetic_module));
    object->Set(isolate,
                "hasSecurityPolicy",
                v8::FunctionTemplate::New(isolate, nullptr)); // TODO: Complete this

    auto introspect = std::make_unique<VMIntrospect>(isolate);
    CHECK(introspect);
    v8::Local<v8::Object> instance = object->NewInstance(ctx).ToLocalChecked();
    instance->SetAlignedPointerInInternalField(0, introspect.get());
    g->Set(ctx, binder::to_v8(isolate, "introspect"), instance).Check();
    return introspect;
}

VMIntrospect::VMIntrospect(v8::Isolate *isolate)
    : fIsolate(isolate)
{
}

VMIntrospect::~VMIntrospect()
{
    for (auto& item : fCallbackMap)
        item.second.Reset();
}

void VMIntrospect::setCallbackSlot(CallbackSlot slot, v8::Local<v8::Function> func)
{
    fCallbackMap[slot].Reset();
    fCallbackMap[slot] = v8::Global<v8::Function>(fIsolate, func);
}

v8::MaybeLocal<v8::Function> VMIntrospect::getCallbackFromSlot(CallbackSlot slot)
{
    if (!fCallbackMap.contains(slot))
        return {};
    return fCallbackMap[slot].Get(fIsolate);
}

namespace {

template<VMIntrospect::CallbackSlot kSlot, typename...ArgsT>
bool introspect_invoke_callback(VMIntrospect *this_, ArgsT&&...args)
{
    v8::Local<v8::Function> cb;
    if (!this_->getCallbackFromSlot(kSlot).template ToLocal(&cb))
        return false;
    v8::Isolate *isolate = this_->getIsolate();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    v8::TryCatch tryCatch(isolate);
    binder::call_v8(isolate, cb,
                    isolate->GetCurrentContext()->Global(), std::forward<ArgsT>(args)...);
    if (tryCatch.HasCaught())
    {
        v8::Local<v8::String> string = tryCatch.Exception()->ToString(ctx).ToLocalChecked();
        auto msg = binder::from_v8<std::string>(isolate, string);
        QLOG(LOG_ERROR, "An unexpected exception was thrown in UncaughtException handler:");
        QLOG(LOG_ERROR, "  Exception: {}", msg);
        return false;
    }
    return true;
}

} // namespace anonymous

bool VMIntrospect::notifyUncaughtException(v8::Local<v8::Value> except)
{
    return introspect_invoke_callback<CallbackSlot::kUncaughtException>(this, except);
}

bool VMIntrospect::notifyBeforeExit(v8::Local<v8::Value> exitCode)
{
    return introspect_invoke_callback<CallbackSlot::kBeforeExit>(this, exitCode);
}

void VMIntrospect::performScheduledTasksCheckpoint()
{
    Runtime *rt = Runtime::GetBareFromIsolate(fIsolate);
    v8::HandleScope scope(fIsolate);
    v8::Local<v8::Object> recv = fIsolate->GetCurrentContext()->Global();
    while (!fScheduledTaskQueue.empty())
    {
        ScheduledTask task(std::move(fScheduledTaskQueue.front()));
        fScheduledTaskQueue.pop();
        CHECK(task.type != ScheduledTask::Type::kInvalid);

        v8::Local<v8::Value> value;
        v8::TryCatch tryCatch(fIsolate);
        if (task.type == ScheduledTask::Type::kEvalScript)
        {
            if (!rt->execute("<anonymous@scheduled>", task.param.c_str()).ToLocal(&value))
            {
                QLOG(LOG_DEBUG, "Introspect.ScheduleEval: An exception was thrown when executing script");
            }
        }
        else if (task.type == ScheduledTask::Type::kEvalModuleUrl)
        {
            if (!rt->evaluateModule(task.param).ToLocal(&value))
            {
                QLOG(LOG_DEBUG, "Introspect.ScheduleEval: An exception was thrown when evaluating module");
            }
        }

        if (tryCatch.HasCaught() && !task.reject.IsEmpty())
            binder::call_v8(fIsolate, task.reject.Get(fIsolate), recv, tryCatch.Exception());
        else if (!tryCatch.HasCaught() && !task.callback.IsEmpty())
            binder::call_v8(fIsolate, task.callback.Get(fIsolate), recv, value);
    }
}

KOI_NS_END
