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

#include "include/core/SkTraceMemoryDump.h"
#include "include/core/SkGraphics.h"
#include "json/json.h"
#include "fmt/format.h"

#include "Gallium/bindings/glamor/Exports.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

namespace {

class SkTraceMemoryDumpImpl : public SkTraceMemoryDump
{
public:
    SkTraceMemoryDumpImpl()
        : root_(Json::objectValue)
        , tracings_(root_["tracings"])
    {
        root_["type"] = "cocoa.gl.tracings.skiamemory";
        root_["tracer"] = "cocoa.gallium.bindings.glamor_wrap.SkTraceMemoryDumpImpl";
    }
    ~SkTraceMemoryDumpImpl() override = default;

    void dumpStringValue(const char *dump_name,
                         const char *value_name,
                         const char *value) override
    {
        Json::Value& value_node = tracings_[dump_name][value_name];
        value_node["type"] = "string";
        value_node["value"] = value;
    }

    void dumpNumericValue(const char *dump_name,
                          const char *value_name,
                          const char *units,
                          uint64_t value) override
    {
        Json::Value& value_node = tracings_[dump_name][value_name];
        value_node["type"] = "numeric";
        value_node["value"] = fmt::format("{}", value);
        value_node["units"] = units;
    }

    void setMemoryBacking(const char *dump_name,
                          const char *backing_type,
                          const char *backing_object_id) override
    {
        Json::Value& value_node = tracings_[dump_name];
        value_node["memory_backing"]["type"] = backing_type;
        value_node["memory_backing"]["object_id"] = backing_object_id;
    }

    void setDiscardableMemoryBacking(const char *dump_name,
                                     const SkDiscardableMemory& memory) override
    {
        // `SkDiscardableMemory` is a Chromium API, and we should not use it.
        // See `//third_party/skia/include/private/chromium/SkDiscardableMemory.h`
        // for more details about this API.
    }

    g_nodiscard LevelOfDetail getRequestedDetails() const override
    {
        return kObjectsBreakdowns_LevelOfDetail;
    }

    std::string ToString()
    {
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        builder["commentStyle"] = "None";
        builder["enableYAMLCompatibility"] = true;
        std::ostringstream oss;
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(root_, &oss);
        return oss.str();
    }

private:
    Json::Value     root_;
    Json::Value&    tracings_;
};

}

v8::Local<v8::Value> TraceSkiaMemoryJSON()
{
    SkTraceMemoryDumpImpl tracer_impl;
    SkGraphics::DumpMemoryStatistics(&tracer_impl);
    std::string result = tracer_impl.ToString();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return v8::String::NewFromUtf8(isolate, result.c_str())
           .ToLocalChecked();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
