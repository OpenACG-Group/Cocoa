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

#include <sstream>

#include "json/json.h"

#include "Core/Errors.h"
#include "Glamor/GraphicsResourcesTrackable.h"
GLAMOR_NAMESPACE_BEGIN

GraphicsResourcesTrackable::Tracer::Tracer()
    : tracings_(nullptr)
{
    root_value_["type"] = "Cocoa GLAMOR resources tracing";
    root_value_["tracer"] = "GLAMOR embedded tracer (GraphicsResourcesTrackable)";

    tracings_ = &root_value_["tracings"];
}

void GraphicsResourcesTrackable::Tracer::TraceMember(const std::string& annotation,
                                                     GraphicsResourcesTrackable *trackable)
{
    CHECK(!tracing_stack_.empty());
    CHECK(trackable);

    Json::Value& current_tracing = (*tracing_stack_.top());
    Json::Value member_tracing(Json::objectValue);
    member_tracing["annotation"] = annotation;
    member_tracing["objects"] = Json::Value(Json::arrayValue);
    member_tracing["members"] = Json::Value(Json::arrayValue);
    tracing_stack_.push(&member_tracing);

    trackable->Trace(this);

    tracing_stack_.pop();
    current_tracing["members"].append(member_tracing);
}

void GraphicsResourcesTrackable::Tracer::TraceResource(const std::string& annotation,
                                                       const char *type,
                                                       const char *device,
                                                       const char *ownership,
                                                       uint64_t id,
                                                       std::optional<size_t> size)
{
    CHECK(!tracing_stack_.empty());

    Json::Value object(Json::objectValue);
    object["annotation"] = annotation;
    object["type"] = type;
    object["device"] = device;
    object["ownership"] = ownership;
    object["id"] = id;
    if (size)
        object["size"] = *size;

    Json::Value& current_tracing = *tracing_stack_.top();
    current_tracing["objects"].append(object);
}

void GraphicsResourcesTrackable::Tracer::TraceRootObject(const std::string& annotation,
                                                         GraphicsResourcesTrackable *trackable)
{
    CHECK(tracing_stack_.empty());
    CHECK(trackable);

    Json::Value& current_tracing = tracings_->append(
            Json::Value(Json::objectValue));
    current_tracing["annotation"] = annotation;
    current_tracing["objects"] = Json::Value(Json::arrayValue);
    current_tracing["members"] = Json::Value(Json::arrayValue);
    tracing_stack_.push(&current_tracing);

    trackable->Trace(this);

    tracing_stack_.pop();
}

std::string GraphicsResourcesTrackable::Tracer::ToJsonString()
{
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "  ";
    builder["commentStyle"] = "None";
    builder["enableYAMLCompatibility"] = true;

    std::ostringstream oss;
    Unique<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(root_value_, &oss);

    return oss.str();
}

GLAMOR_NAMESPACE_END
