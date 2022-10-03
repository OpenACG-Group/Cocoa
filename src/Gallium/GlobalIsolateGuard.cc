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
#include "Gallium/UnixPathTools.h"
GALLIUM_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium)

namespace {

void uncaught_exception(v8::Local<v8::Message> message, v8::Local<v8::Value> except)
{
    v8::Isolate *isolate = message->GetIsolate();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    Runtime *rt = Runtime::GetBareFromIsolate(isolate);
    CHECK(rt);

    v8::Local<v8::String> string = except->ToString(ctx).ToLocalChecked();
    QLOG(LOG_ERROR, "%fg<re>Uncaught exception: {}%reset", binder::from_v8<std::string>(isolate, string));

    QLOG(LOG_ERROR, "  %fg<re>Stack traceback:%reset");
    v8::Local<v8::StackTrace> trace = message->GetStackTrace();
    MeasuredTable mt(/* minSpace */ 1);
    for (int32_t i = 0; i < trace->GetFrameCount(); i++)
    {
        v8::Local<v8::StackFrame> frame = trace->GetFrame(isolate, i);
        std::string funcName = "<unknown>", scriptName = "<unknown>";
        std::string funcNamePrefix;

        if (frame->IsConstructor())
            funcNamePrefix = "new ";

        if (!frame->GetScriptName().IsEmpty())
        {
            scriptName = binder::from_v8<std::string>(isolate, frame->GetScriptName());
            if (utils::StrStartsWith(scriptName, "file://"))
            {
                scriptName = "file://" + unixpath::SolveShortestPathRepresentation(scriptName.substr(7));
            }
        }
        if (!frame->GetFunctionName().IsEmpty())
            funcName = binder::from_v8<std::string>(isolate, frame->GetFunctionName());

        if (frame->GetLineNumber() != v8::Message::kNoLineNumberInfo)
            scriptName.append(fmt::format(":{}", frame->GetLineNumber()));
        if (frame->GetColumn() != v8::Message::kNoColumnInfo)
            scriptName.append(fmt::format(":{}", frame->GetColumn()));

        mt.append(fmt::format("%fg<bl>#{}%reset %italic%fg<ye>{}{}%reset", i, funcNamePrefix, funcName),
                  fmt::format("%fg<cy>(from {})%reset", scriptName));
    }
    mt.flush([](const std::string& line) {
        QLOG(LOG_ERROR, "    {}", line);
    });

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

GlobalIsolateGuard::GlobalIsolateGuard(const std::shared_ptr<Runtime>& rt)
    : fRuntime(rt)
    , fIsolate(rt->GetIsolate())
{
    fIsolate->SetCaptureStackTraceForUncaughtExceptions(true);
    fIsolate->AddMessageListenerWithErrorLevel(per_isolate_message_listener,
                                               v8::Isolate::MessageErrorLevel::kMessageError |
                                               v8::Isolate::MessageErrorLevel::kMessageWarning);
    fIsolate->SetOOMErrorHandler(per_isolate_oom_handler);
    fIsolate->SetPromiseRejectCallback(per_isolate_promise_reject_handler);
}

GlobalIsolateGuard::~GlobalIsolateGuard()
{
    fIsolate->SetPromiseRejectCallback(nullptr);
    fIsolate->SetOOMErrorHandler(nullptr);
    fIsolate->RemoveMessageListeners(per_isolate_message_listener);
    fIsolate->SetCaptureStackTraceForUncaughtExceptions(false);
}

void GlobalIsolateGuard::pushMaybeUnhandledRejectPromise(v8::Local<v8::Promise> promise,
                                                         v8::Local<v8::Value> value)
{
    PromiseWithValue pack(fIsolate, promise, value);
    if (std::find(fRejectPromises.begin(), fRejectPromises.end(), pack) != fRejectPromises.end())
        return;
    fRejectPromises.emplace_back(std::move(pack));
}

void GlobalIsolateGuard::removeMaybeUnhandledRejectPromise(v8::Local<v8::Promise> promise)
{
    auto pv = std::find(fRejectPromises.begin(), fRejectPromises.end(), promise);
    if (pv == fRejectPromises.end())
        return;
    fRejectPromises.erase(pv);
}

void GlobalIsolateGuard::performUnhandledRejectPromiseCheck()
{
    auto& introspect = getRuntime()->GetIntrospect();
    if (!introspect)
    {
        QLOG(LOG_WARNING, "{} promise(s) was rejected but not handled (introspect not available)",
             fRejectPromises.size());
        fRejectPromises.clear();
        return;
    }
    for (PromiseWithValue& pv : fRejectPromises)
    {
        v8::HandleScope scope(fIsolate);
        v8::Local<v8::Promise> promise = pv.promise.Get(fIsolate);
        v8::Local<v8::Value> value = pv.value.Get(fIsolate);
        if (!introspect->notifyUnhandledPromiseRejection(promise, value))
            throw RuntimeException(__func__, "Uncaught and unhandled promise rejection");
    }
    fRejectPromises.clear();
}

void GlobalIsolateGuard::reportUncaughtExceptionFromCallback(const v8::TryCatch& caught)
{
    if (caught.HasCaught())
    {
        uncaught_exception(caught.Message(), caught.Exception());
    }
}

GALLIUM_NS_END
