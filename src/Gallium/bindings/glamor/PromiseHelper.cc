#include "Core/Exception.h"
#include "Gallium/bindings/glamor/PromiseHelper.h"

#include <utility>
#include "Gallium/Runtime.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

namespace i = ::cocoa::glamor;

PromiseClosure::PromiseClosure(v8::Isolate *isolate, InfoConverter conv)
        : isolate_(isolate)
        , info_converter_(std::move(conv))
{
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    auto resolver = v8::Promise::Resolver::New(ctx).ToLocalChecked();
    resolver_ = v8::Global<v8::Promise::Resolver>(isolate, resolver);
}

PromiseClosure::~PromiseClosure()
{
    resolver_.Reset();
}

std::shared_ptr<PromiseClosure> PromiseClosure::New(v8::Isolate *isolate,
                                                    const InfoConverter& converter)
{
    return std::make_shared<PromiseClosure>(isolate, converter);
}

bool PromiseClosure::rejectIfEssential(i::RenderHostCallbackInfo& info)
{
    std::string reason = "[RenderClient<Response>] ";
    v8::Local<v8::Context> ctx = isolate_->GetCurrentContext();
    switch (info.GetReturnStatus())
    {
    case i::RenderClientCallInfo::Status::kOpCodeInvalid:
        reason += "Invalid operation code";
        break;
    case i::RenderClientCallInfo::Status::kArgsInvalid:
        reason += "Invalid arguments";
        break;
    case i::RenderClientCallInfo::Status::kCaught:
        reason += info.GetCaughtException().what();
        break;
    case i::RenderClientCallInfo::Status::kOpFailed:
        reason += "Operation failed";
        break;
    default:
        return false;
    }
    auto error = v8::Exception::Error(binder::to_v8(isolate_, reason)).As<v8::Object>();
    error->Set(ctx, binder::to_v8(isolate_, "opcode"),
               binder::to_v8(isolate_, info.GetOpcode())).Check();
    resolver_.Get(isolate_)->Reject(ctx, error).Check();
    return true;
}

void PromiseClosure::HostCallback(i::RenderHostCallbackInfo& info)
{
    auto this_ = info.GetClosure<std::shared_ptr<PromiseClosure>>();

    v8::HandleScope scope(this_->isolate_);
    if (this_->rejectIfEssential(info))
        return;

    v8::Local<v8::Promise::Resolver> r = this_->resolver_.Get(this_->isolate_);
    v8::Local<v8::Context> ctx = this_->isolate_->GetCurrentContext();
    if (info.HasReturnValue())
    {
        r->Resolve(ctx, this_->info_converter_(this_->isolate_, info)).Check();
    }
    else
    {
        r->Resolve(ctx, v8::Undefined(this_->isolate_)).Check();
    }
}

v8::Local<v8::Promise> PromiseClosure::getPromise()
{
    return resolver_.Get(isolate_)->GetPromise();
}

namespace {

void slot_closure_callback(SlotClosure *closure, i::RenderHostSlotCallbackInfo& info)
{
    v8::HandleScope scope(closure->isolate_);
    v8::Local<v8::Context> ctx = closure->isolate_->GetCurrentContext();

    v8::Local<v8::Function> cb = closure->callback_.Get(closure->isolate_);
    std::vector<v8::Local<v8::Value>> vec;

    if (closure->acceptor_)
    {
        auto maybe = closure->acceptor_(closure->isolate_, info);
        if (!maybe.has_value())
        {
            throw RuntimeException(__func__,
                                   "Values emitted by RenderClient could not be accepted");
        }
        vec = std::move(maybe.value());
    }

    v8::TryCatch catchBlock(closure->isolate_);
    v8::MaybeLocal<v8::Value> ret;

    if (vec.empty())
        ret = cb->Call(ctx, ctx->Global(), 0, nullptr);
    else
        ret = cb->Call(ctx, ctx->Global(), static_cast<int>(vec.size()), vec.data());

    // CHECK(!ret.IsEmpty());

    if (catchBlock.HasCaught())
    {
        Runtime *rt = Runtime::GetBareFromIsolate(closure->isolate_);
        rt->reportUncaughtExceptionInCallback(catchBlock);
    }
}

} // namespace anonymous

std::unique_ptr<SlotClosure> SlotClosure::New(v8::Isolate *isolate,
                                              int32_t signal,
                                              const i::Shared<i::RenderClientObject>& client,
                                              v8::Local<v8::Function> callback,
                                              InfoAcceptor acceptor)
{
    CHECK(client);
    auto closure = std::make_unique<SlotClosure>();
    CHECK(closure);

    closure->isolate_ = isolate;
    closure->callback_.Reset(isolate, callback);
    closure->client_ = client;
    closure->acceptor_ = std::move(acceptor);
    closure->slot_id_ = client->Connect(signal, [ptr = closure.get()](auto&& info) {
        slot_closure_callback(ptr, std::forward<decltype(info)>(info));
    });
    closure->signal_code_ = signal;

    return closure;
}

SlotClosure::~SlotClosure()
{
    client_->Disconnect(slot_id_);
    callback_.Reset();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
