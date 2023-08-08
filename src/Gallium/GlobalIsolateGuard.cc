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

#include "Core/MeasuredTable.h"
#include "Core/Journal.h"
#include "Core/Utils.h"
#include "Gallium/GlobalIsolateGuard.h"
#include "Gallium/Runtime.h"
#include "Gallium/binder/Convert.h"
#include "Gallium/Infrastructures.h"
GALLIUM_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium)

namespace {

void uncaught_exception(v8::Local<v8::Message> message, v8::Local<v8::Value> except)
{
    v8::Isolate *isolate = message->GetIsolate();
    Runtime *rt = Runtime::GetBareFromIsolate(isolate);

    infra::ReportUncaughtException(isolate, message, except);

    if (rt->GetIntrospect())
        rt->GetIntrospect()->notifyUncaughtException(except);
}

void per_isolate_message_listener(v8::Local<v8::Message> message,
                                  v8::Local<v8::Value> except)
{
    v8::Isolate *isolate = message->GetIsolate();
    Runtime *rt = Runtime::GetBareFromIsolate(isolate);
    CHECK(rt);

    auto level = message->ErrorLevel();
    switch (level)
    {
    case v8::Isolate::MessageErrorLevel::kMessageWarning:
    {
        auto script = binder::from_v8<std::string>(isolate, message->GetScriptResourceName());
        auto content = binder::from_v8<std::string>(isolate, message->Get());
        QLOG(LOG_WARNING, "%fg<hl>(Isolate)%reset Warning from script {} line {}:", script,
             message->GetLineNumber(rt->GetContext()).FromMaybe(-1));
        QLOG(LOG_WARNING, "  {}", content);
        break;
    }
    case v8::Isolate::MessageErrorLevel::kMessageError:
        uncaught_exception(message, except);
        break;
    default:
        MARK_UNREACHABLE();
    }
}

void per_isolate_oom_handler(const char *location, const v8::OOMDetails& details)
{
    QLOG(LOG_ERROR, "%fg<re,hl>(V8) Out of memory: {}%reset", location);
    QLOG(LOG_ERROR, "(V8) OOM detailed information:");
    QLOG(LOG_ERROR, "  Heap OOM: {}", details.is_heap_oom);
    QLOG(LOG_ERROR, "  Details: {}", details.detail);

    // Crash the whole program.
    fatal_oom_error();

    MARK_UNREACHABLE();
}

void per_isolate_promise_reject_handler(v8::PromiseRejectMessage message)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    Runtime *pRuntime = Runtime::GetBareFromIsolate(isolate);
    auto& guard = pRuntime->GetUniqueGlobalIsolateGuard();

    VMIntrospect::MultipleResolveAction multipleResolveAction;
    switch (message.GetEvent())
    {
    case v8::kPromiseRejectWithNoHandler:
        guard->pushMaybeUnhandledRejectPromise(message.GetPromise(), message.GetValue());
        return;
    case v8::kPromiseHandlerAddedAfterReject:
        guard->removeMaybeUnhandledRejectPromise(message.GetPromise());
        return;
    case v8::kPromiseRejectAfterResolved:
        multipleResolveAction = VMIntrospect::MultipleResolveAction::kReject;
        break;
    case v8::kPromiseResolveAfterResolved:
        multipleResolveAction = VMIntrospect::MultipleResolveAction::kResolve;
        break;
    }
    if (pRuntime->GetIntrospect())
        pRuntime->GetIntrospect()->notifyPromiseMultipleResolve(message.GetPromise(),
                                                                multipleResolveAction);
}

} // namespace anonymous

GlobalIsolateGuard::GlobalIsolateGuard(Runtime *rt)
    : runtime_(rt)
    , isolate_(rt->GetIsolate())
{
    isolate_->SetCaptureStackTraceForUncaughtExceptions(true);
    isolate_->AddMessageListenerWithErrorLevel(per_isolate_message_listener,
                                               v8::Isolate::MessageErrorLevel::kMessageError |
                                               v8::Isolate::MessageErrorLevel::kMessageWarning);
    isolate_->SetOOMErrorHandler(per_isolate_oom_handler);
    isolate_->SetPromiseRejectCallback(per_isolate_promise_reject_handler);
}

GlobalIsolateGuard::~GlobalIsolateGuard()
{
    isolate_->SetPromiseRejectCallback(nullptr);
    isolate_->SetOOMErrorHandler(nullptr);
    isolate_->RemoveMessageListeners(per_isolate_message_listener);
    isolate_->SetCaptureStackTraceForUncaughtExceptions(false);
}

void GlobalIsolateGuard::pushMaybeUnhandledRejectPromise(v8::Local<v8::Promise> promise,
                                                         v8::Local<v8::Value> value)
{
    PromiseWithValue pack(isolate_, promise, value);
    if (std::find(reject_promises_.begin(), reject_promises_.end(), pack) != reject_promises_.end())
        return;
    reject_promises_.emplace_back(std::move(pack));
}

void GlobalIsolateGuard::removeMaybeUnhandledRejectPromise(v8::Local<v8::Promise> promise)
{
    auto pv = std::find(reject_promises_.begin(), reject_promises_.end(), promise);
    if (pv == reject_promises_.end())
        return;
    reject_promises_.erase(pv);
}

void GlobalIsolateGuard::performUnhandledRejectPromiseCheck()
{
    auto& introspect = getRuntime()->GetIntrospect();
    if (!introspect)
    {
        QLOG(LOG_WARNING, "{} promise(s) was rejected but not handled (introspect not available)",
             reject_promises_.size());
        reject_promises_.clear();
        return;
    }
    for (PromiseWithValue& pv : reject_promises_)
    {
        v8::HandleScope scope(isolate_);
        v8::Local<v8::Promise> promise = pv.promise.Get(isolate_);
        v8::Local<v8::Value> value = pv.value.Get(isolate_);
        if (!introspect->notifyUnhandledPromiseRejection(promise, value))
            throw RuntimeException(__func__, "Uncaught and unhandled promise rejection");
    }
    reject_promises_.clear();
}

void GlobalIsolateGuard::reportUncaughtExceptionFromCallback(const v8::TryCatch& caught)
{
    if (caught.HasCaught())
    {
        uncaught_exception(caught.Message(), caught.Exception());
    }
}

GALLIUM_NS_END
