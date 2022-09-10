/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Core/Utils.h"
#include "Gallium/VMIntrospect.h"
#include "Gallium/binder/Convert.h"
#include "Gallium/binder/ThrowExcept.h"
#include "Gallium/binder/CallV8.h"
#include "Gallium/BindingManager.h"
#include "Gallium/bindings/Base.h"

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.Introspect)

GALLIUM_NS_BEGIN

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
    JS_THROW_IF(!args[0]->IsFunction(), "Callback must be a function", v8::Exception::TypeError);
    introspect->setCallbackSlot(kSlot, args[0].template As<v8::Function>());
}

void introspect_load_shared_object(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    VMIntrospect *introspect = get_bare_introspect_ptr(args);
    JS_THROW_IF(args.Length() != 1, "Invalid number of arguments", v8::Exception::Error);
    JS_THROW_IF(!args[0]->IsString(), "Shared object path must be a string", v8::Exception::TypeError);
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
        JS_THROW_IF(!args[1]->IsFunction(), "Callback is must be a function", v8::Exception::TypeError);
        task.callback = v8::Global<v8::Function>(isolate, v8::Local<v8::Function>::Cast(args[1]));
    }
    if (args.Length() == 3)
    {
        JS_THROW_IF(!args[2]->IsFunction(), "Callback is must be a function", v8::Exception::TypeError);
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
    JS_THROW_IF(!args[0]->IsString(), "str must be a string", v8::Exception::TypeError);
    v8::Isolate *isolate = get_bare_introspect_ptr(args)->getIsolate();
    std::printf("%s", binder::from_v8<std::string>(isolate, args[0]).c_str());
    std::fflush(stdout);
}

void introspect_has_synthetic_module(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    JS_THROW_IF(args.Length() != 1, "Invalid number of arguments", v8::Exception::Error);
    JS_THROW_IF(!args[0]->IsString(), "specifier must be a string", v8::Exception::TypeError);
    v8::Isolate *isolate = get_bare_introspect_ptr(args)->getIsolate();
    auto name = binder::from_v8<std::string>(isolate, args[0]);
    args.GetReturnValue().Set(BindingManager::Ref().search(name) != nullptr);
}

// NOLINTNEXTLINE
std::map<std::string, std::function<bool(const Runtime::Options&)>> policy_checker_map = {
        {
            "AllowLoadingSharedObject",
            [](const Runtime::Options& opts) -> bool {
                return opts.introspect_allow_loading_shared_object;
            }
        },
        {
            "ForbidLoadingSharedObject",
            [](const Runtime::Options& opts) -> bool {
                return !opts.introspect_allow_loading_shared_object;
            }
        },
        {
            "AllowWritingToJournal",
            [](const Runtime::Options& opts) -> bool {
                return opts.introspect_allow_write_journal;
            }
        },
        {
            "ForbidWritingToJournal",
            [](const Runtime::Options& opts) -> bool {
                return !opts.introspect_allow_write_journal;
            }
        }
};

void introspect_has_security_policy(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    JS_THROW_IF(args.Length() != 1, "Invalid number of arguments", v8::Exception::Error);
    JS_THROW_IF(!args[0]->IsString(), "Policy name must be a string", v8::Exception::TypeError);
    v8::Isolate *isolate = get_bare_introspect_ptr(args)->getIsolate();
    auto policy = binder::from_v8<std::string>(isolate, args[0]);
    JS_THROW_IF(!utils::MapContains(policy_checker_map, policy), "Invalid policy name", v8::Exception::Error);
    Runtime *rt = Runtime::GetBareFromIsolate(isolate);
    CHECK(rt != nullptr);
    args.GetReturnValue().Set(policy_checker_map[policy](rt->getOptions()));
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
                "Writing to journal is forbidden by current introspect policy",
                v8::Exception::Error);
    JS_THROW_IF(args.Length() != 2, "Invalid number of arguments", v8::Exception::Error);
    JS_THROW_IF(!args[0]->IsString() || !args[1]->IsString(), "arguments must be strings",
                v8::Exception::TypeError);
    auto level = binder::from_v8<std::string>(isolate, args[0]);
    JS_THROW_IF(!utils::MapContains(log_level_name_map, level), "Unrecognized journal level string",
                v8::Exception::Error);
    auto content = binder::from_v8<std::string>(isolate, args[1]);
    QLOG(log_level_name_map[level], "{}", content);
}

void introspect_stacktrace(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = get_bare_introspect_ptr(args)->getIsolate();
    v8::HandleScope scope(isolate);
    JS_THROW_IF(args.Length() > 1, "Too many arguments", v8::Exception::Error);

    int frameLimit = Runtime::GetBareFromIsolate(isolate)->getOptions().introspect_stacktrace_frame_limit;
    if (args.Length() == 1)
    {
        JS_THROW_IF(!args[0]->IsNumber(), "Frame limitation must be a number", v8::Exception::TypeError);
        frameLimit = binder::from_v8<int>(isolate, args[0]);
        JS_THROW_IF(frameLimit < 0, "Invalid frame limitation", v8::Exception::RangeError);
    }

    v8::Local<v8::StackTrace> trace = v8::StackTrace::CurrentStackTrace(isolate, frameLimit);
    JS_THROW_IF(trace.IsEmpty(), "Failed to capture stacktrace", v8::Exception::Error);

    v8::Local<v8::Array> result = v8::Array::New(isolate, trace->GetFrameCount());
    v8::Local<v8::Context> context = Runtime::GetBareFromIsolate(isolate)->GetContext();

    constexpr auto kNL = v8::Message::kNoLineNumberInfo;
    constexpr auto kNC = v8::Message::kNoColumnInfo;

    for (int32_t i = 0; i < trace->GetFrameCount(); i++)
    {
        v8::Local<v8::StackFrame> frame = trace->GetFrame(isolate, i);
        v8::Local<v8::Object> cur = v8::Object::New(isolate);
        int32_t line = frame->GetLineNumber() != kNL ? frame->GetLineNumber() : -1;
        int32_t column = frame->GetColumn() != kNC ? frame->GetColumn() : -1;
        cur->Set(context, binder::to_v8(isolate, "line"), binder::to_v8(isolate, line)).Check();
        cur->Set(context, binder::to_v8(isolate, "column"), binder::to_v8(isolate, column)).Check();
        v8::Local<v8::Value> scriptName = v8::Undefined(isolate);
        v8::Local<v8::Value> funcName = v8::Undefined(isolate);
        if (!frame->GetScriptName().IsEmpty())
            scriptName = frame->GetScriptName();
        if (!frame->GetFunctionName().IsEmpty())
            funcName = frame->GetFunctionName();
        cur->Set(context, binder::to_v8(isolate, "scriptName"), scriptName).Check();
        cur->Set(context, binder::to_v8(isolate, "functionName"), funcName).Check();
        cur->Set(context, binder::to_v8(isolate, "isEval"),
                 binder::to_v8(isolate, frame->IsEval())).Check();
        cur->Set(context, binder::to_v8(isolate, "isConstructor"),
                 binder::to_v8(isolate, frame->IsConstructor())).Check();
        cur->Set(context, binder::to_v8(isolate, "isWasm"),
                 binder::to_v8(isolate, frame->IsWasm())).Check();
        cur->Set(context, binder::to_v8(isolate, "isUserJavaScript"),
                 binder::to_v8(isolate, frame->IsUserJavaScript())).Check();
        result->Set(context, i, cur).Check();
    }
    args.GetReturnValue().Set(result);
}

} // namespace anonymous

std::unique_ptr<VMIntrospect> VMIntrospect::InstallGlobal(v8::Isolate *isolate)
{
    CHECK(isolate->InContext());
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    v8::Local<v8::Object> g = ctx->Global();

    v8::Local<v8::ObjectTemplate> object = v8::ObjectTemplate::New(isolate);
    object->SetInternalFieldCount(1);
    using FT = v8::FunctionTemplate;
    object->Set(isolate,
                "setUncaughtExceptionHandler",
                FT::New(isolate, introspect_set_callback_slot<CallbackSlot::kUncaughtException>));
    object->Set(isolate,
                "setBeforeExitHandler",
                FT::New(isolate, introspect_set_callback_slot<CallbackSlot::kBeforeExit>));
    object->Set(isolate,
                "setUnhandledPromiseRejectionHandler",
                FT::New(isolate, introspect_set_callback_slot<CallbackSlot::kUnhandledPromiseRejection>));
    object->Set(isolate,
                "setPromiseMultipleResolveHandler",
                FT::New(isolate, introspect_set_callback_slot<CallbackSlot::kPromiseMultipleResolve>));
    object->Set(isolate,
                "loadSharedObject",
                FT::New(isolate, introspect_load_shared_object));
    object->Set(isolate,
                "scheduleScriptEvaluate",
                FT::New(isolate, introspect_schedule_script_eval));
    object->Set(isolate,
                "scheduleModuleUrlEvaluate",
                FT::New(isolate, introspect_schedule_module_eval));
    object->Set(isolate,
                "print",
                FT::New(isolate, introspect_print));
    object->Set(isolate,
                "writeToJournal",
                FT::New(isolate, introspect_write_journal));
    object->Set(isolate,
                "hasSyntheticModule",
                FT::New(isolate, introspect_has_synthetic_module));
    object->Set(isolate,
                "hasSecurityPolicy",
                FT::New(isolate, introspect_has_security_policy));
    object->Set(isolate,
                "inspectStackTrace",
                FT::New(isolate, introspect_stacktrace));

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
    if (!utils::MapContains(fCallbackMap, slot))
        return {};
    return fCallbackMap[slot].Get(fIsolate);
}

namespace {

template<VMIntrospect::CallbackSlot kSlot, typename...ArgsT>
bool introspect_invoke_callback(VMIntrospect *this_, ArgsT&&...args)
{
    v8::Isolate *isolate = this_->getIsolate();
    v8::HandleScope scope(isolate);
    v8::Local<v8::Function> cb;
    if (!this_->getCallbackFromSlot(kSlot).ToLocal(&cb))
        return false;
    v8::TryCatch tryCatch(isolate);
    binder::Invoke(isolate, cb,
                   isolate->GetCurrentContext()->Global(), std::forward<ArgsT>(args)...);
    if (tryCatch.HasCaught())
    {
        Runtime::GetBareFromIsolate(this_->getIsolate())->ReportUncaughtExceptionInCallback(tryCatch);
        return false;
    }
    return true;
}

} // namespace anonymous

bool VMIntrospect::notifyUncaughtException(v8::Local<v8::Value> except)
{
    return introspect_invoke_callback<CallbackSlot::kUncaughtException>(this, except);
}

bool VMIntrospect::notifyBeforeExit()
{
    return introspect_invoke_callback<CallbackSlot::kBeforeExit>(this);
}

bool VMIntrospect::notifyUnhandledPromiseRejection(v8::Local<v8::Promise> promise, v8::Local<v8::Value> value)
{
    return introspect_invoke_callback<CallbackSlot::kUnhandledPromiseRejection>(this, promise, value);
}

bool VMIntrospect::notifyPromiseMultipleResolve(v8::Local<v8::Promise> promise, MultipleResolveAction action)
{
    std::string strAction;
    switch (action)
    {
    case MultipleResolveAction::kResolve:
        strAction = "resolve";
        break;
    case MultipleResolveAction::kReject:
        strAction = "reject";
        break;
    }
    return introspect_invoke_callback<CallbackSlot::kPromiseMultipleResolve>(this, promise, strAction);
}

VMIntrospect::PerformCheckpointResult VMIntrospect::performScheduledTasksCheckpoint()
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
        bool hasCaught;
        v8::Local<v8::Value> exception;
        {
            v8::TryCatch tryCatch(fIsolate);
            if (task.type == ScheduledTask::Type::kEvalScript)
            {
                value = rt->ExecuteScript("<anonymous@scheduled>", task.param.c_str())
                        .FromMaybe(v8::Local<v8::Value>());
            }
            else if (task.type == ScheduledTask::Type::kEvalModuleUrl)
            {
                try {
                    value = rt->EvaluateModule(task.param).FromMaybe(v8::Local<v8::Value>());
                } catch (const std::exception& e) {
                    binder::throw_(fIsolate, e.what(), v8::Exception::Error);
                }
            }
            hasCaught = tryCatch.HasCaught();
            exception = tryCatch.Exception();
        }

        v8::TryCatch tryCatch(fIsolate);
        tryCatch.SetVerbose(true);
        if (hasCaught)
        {
            if (!task.reject.IsEmpty())
                binder::Invoke(fIsolate, task.reject.Get(fIsolate), recv, exception);
            else
            {
                v8::Local<v8::String> str = exception->ToString(fIsolate->GetCurrentContext())
                                            .FromMaybe(v8::Local<v8::String>());
                std::string str_native = "<unknown>";
                if (!str.IsEmpty())
                    str_native = binder::from_v8<decltype(str_native)>(fIsolate, str);
                QLOG(LOG_ERROR, "%fg<re>Uncaught exception from scheduled evaluation: {}%reset", str_native);
                return PerformCheckpointResult::kThrow;
            }
        }
        else if (!task.callback.IsEmpty())
        {
            binder::Invoke(fIsolate, task.callback.Get(fIsolate), recv, value);
        }
        if (tryCatch.HasCaught())
            return PerformCheckpointResult::kThrow;
    }
    return PerformCheckpointResult::kOk;
}

GALLIUM_NS_END
