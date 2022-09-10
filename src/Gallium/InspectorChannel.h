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

#ifndef COCOA_GALLIUM_INSPECTORCHANNEL_H
#define COCOA_GALLIUM_INSPECTORCHANNEL_H

#include "include/v8-inspector.h"

#include "Gallium/Gallium.h"
GALLIUM_NS_BEGIN

class InspectorClient;

class InspectorChannel : public v8_inspector::V8Inspector::Channel
{
public:
    explicit InspectorChannel(InspectorClient *client);
    ~InspectorChannel() override = default;

    void sendResponse(int callId, std::unique_ptr<v8_inspector::StringBuffer> message) override;
    void sendNotification(std::unique_ptr<v8_inspector::StringBuffer> message) override;
    void flushProtocolNotifications() override;

private:
    InspectorClient *client_;
};

GALLIUM_NS_END
#endif //COCOA_GALLIUM_INSPECTORCHANNEL_H
