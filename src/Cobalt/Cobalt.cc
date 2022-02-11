#include "include/core/SkGraphics.h"
#include "include/core/SkTraceMemoryDump.h"
#include "Core/Journal.h"
#include "Cobalt/Cobalt.h"
#include "Cobalt/RenderHost.h"
#include "Cobalt/RenderClient.h"
COBALT_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Cobalt)

ContextOptions::ContextOptions()
    : fBackend(Backends::kDefault)
    , fSkiaJIT(COBALT_SKIA_JIT_DEFAULT)
{
}

Backends ContextOptions::GetBackend() const
{
    return fBackend;
}

bool ContextOptions::GetSkiaJIT() const
{
    return fSkiaJIT;
}

void ContextOptions::SetBackend(Backends backend)
{
    fBackend = backend;
}

void ContextOptions::SetSkiaJIT(bool allow)
{
    fSkiaJIT = allow;
}

GlobalScope::GlobalScope(const ContextOptions& options, EventLoop *loop)
    : fOptions(options)
    , event_loop_(loop)
    , render_host_(nullptr)
    , render_client_(nullptr)
{
    if (fOptions.GetSkiaJIT())
        SkGraphics::AllowJIT();
}

void GlobalScope::Initialize()
{
    if (render_host_ || render_client_)
    {
        QLOG(LOG_WARNING, "Initialize GlobalScope two or more times");
        return;
    }

    render_host_ = new RenderHost(event_loop_);
    render_client_ = new RenderClient(render_host_);
    render_host_->SetRenderClient(render_client_);
}

void GlobalScope::Dispose()
{
    if (render_host_ && render_client_)
    {
        delete render_client_;
        render_client_ = nullptr;
        render_host_->OnDispose();
        /* This method may be called in a callback of host invocation,
         * so we should not delete @p render_host_ here. */
    }
    else
    {
        QLOG(LOG_WARNING, "Disposing GlobalScope without an initialization");
    }
}

GlobalScope::~GlobalScope()
{
    if (render_client_)
    {
        QLOG(LOG_ERROR, "Destructing GlobalScope without disposing it is unexpected");
        delete render_client_;
    }
    delete render_host_;
}

ContextOptions& GlobalScope::GetOptions()
{
    return fOptions;
}

COBALT_NAMESPACE_END
