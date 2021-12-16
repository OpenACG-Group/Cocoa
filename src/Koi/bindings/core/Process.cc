#include "Core/Journal.h"
#include "Core/EventLoop.h"
#include "Core/Properties.h"
#include "Koi/bindings/core/Exports.h"
#include "Koi/bindings/core/FdRandomize.h"

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Koi.bindings)

KOI_BINDINGS_NS_BEGIN

void corePrint(const std::string& str)
{
    if (str.empty())
        return;
    std::fwrite(str.c_str(), str.size(), 1, stdout);
}

void coreDump(const std::string& what)
{
    if (what == "descriptors-info")
    {
        FDLRDumpMappingInfo();
    }
    else
    {
        JS_THROW_IF(false, "Unknown dump target");
    }
}

void coreExit()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    isolate->TerminateExecution();
    EventLoop::Instance()->dispose();
    QLOG(LOG_DEBUG, "Program will be terminated directly by JavaScript");
}

v8::Local<v8::Array> GetEscapableArgs()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope scope(isolate);
    v8::Local<v8::Array> array = v8::Array::New(isolate);

    auto args = prop::Cast<PropertyArrayNode>(prop::Get()->next("runtime")->next("script")->next("args"));
    uint32_t index = 0;
    for (const std::shared_ptr<PropertyNode>& node : *args)
    {
        const std::string& str = prop::Cast<PropertyDataNode>(node)->extract<std::string>();
        array->Set(isolate->GetCurrentContext(), index, binder::to_v8(isolate, str)).Check();
        index++;
    }
    return scope.Escape(array);
}

KOI_BINDINGS_NS_END
