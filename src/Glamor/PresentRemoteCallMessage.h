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

#ifndef COCOA_GLAMOR_PRESENTREMOTECALLMESSAGE_H
#define COCOA_GLAMOR_PRESENTREMOTECALLMESSAGE_H

#include <utility>
#include <chrono>

#include "Glamor/Glamor.h"
#include "Glamor/PresentRemoteCall.h"
#include "Glamor/PresentRemoteCallReturn.h"
#include "Glamor/PresentMessage.h"
GLAMOR_NAMESPACE_BEGIN

class PresentRemoteHandle;

class PresentRemoteCallMessage : public PresentMessage
{
public:
    PresentRemoteCallMessage(std::shared_ptr<PresentRemoteHandle> receiver, PresentRemoteCall info,
                             PresentRemoteCallResultCallback callback)
        : PresentMessage(PresentMessage::Type::kRemoteCall)
        , receiver_(std::move(receiver))
        , client_call_info_(std::move(info))
        , host_callback_(std::move(callback)) {}

    PresentRemoteCallMessage(const PresentRemoteCallMessage&) = delete;
    ~PresentRemoteCallMessage() override = default;

    PresentRemoteCallMessage& operator=(const PresentRemoteCallMessage&) = delete;

    g_nodiscard g_inline std::shared_ptr<PresentRemoteHandle> GetReceiver() const {
        return receiver_;
    }

    g_nodiscard g_inline PresentRemoteCall& GetClientCallInfo() {
        return client_call_info_;
    }

    g_nodiscard g_inline PresentRemoteCallResultCallback GetHostCallback() const {
        return host_callback_;
    }

private:
    std::shared_ptr<PresentRemoteHandle>    receiver_;
    PresentRemoteCall                       client_call_info_;
    PresentRemoteCallResultCallback         host_callback_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_PRESENTREMOTECALLMESSAGE_H
