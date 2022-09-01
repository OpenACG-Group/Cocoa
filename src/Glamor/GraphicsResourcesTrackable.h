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

#ifndef COCOA_GLAMOR_GRAPHICSRESOURCETRACKABLE_H
#define COCOA_GLAMOR_GRAPHICSRESOURCETRACKABLE_H

#include <stack>
#include <optional>

#include "json/value.h"

#include "Glamor/Glamor.h"

GLAMOR_NAMESPACE_BEGIN

#define TRACKABLE_TYPE_TEXTURE          "Texture"
#define TRACKABLE_TYPE_BITMAP           "Bitmap"
#define TRACKABLE_TYPE_REPRESENT        "Represent"
#define TRACKABLE_TYPE_HANDLE           "Handle"
#define TRACKABLE_TYPE_CLASS_OBJECT     "ClassObject"
#define TRACKABLE_TYPE_POOL             "Pool"

#define TRACKABLE_DEVICE_GPU            "GPU"
#define TRACKABLE_DEVICE_CPU            "CPU"

#define TRACKABLE_OWNERSHIP_STRICT_OWNED      "StrictOwned"
#define TRACKABLE_OWNERSHIP_SHARED            "Shared"
#define TRACKABLE_OWNERSHIP_WEAK              "WeakReference"

class GraphicsResourcesTrackable
{
public:
    class Tracer
    {
    public:
        explicit Tracer();
        ~Tracer() = default;

        void TraceMember(const std::string& annotation,
                         GraphicsResourcesTrackable *trackable);

        void TraceResource(const std::string& annotation,
                           const char *type,
                           const char *device,
                           const char *ownership,
                           uint64_t id,
                           std::optional<size_t> size = {});

        void TraceRootObject(const std::string& annotation,
                             GraphicsResourcesTrackable *trackable);

        std::string ToJsonString();

    private:
        Json::Value     root_value_;
        Json::Value    *tracings_;
        std::stack<Json::Value*> tracing_stack_;
    };

    static uint64_t TraceIdFromPointer(void *pointer) {
        return reinterpret_cast<uint64_t>(pointer);
    }

    virtual ~GraphicsResourcesTrackable() = default;

    virtual void Trace(Tracer *tracer) noexcept = 0;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_GRAPHICSRESOURCETRACKABLE_H
