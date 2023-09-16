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

#ifndef COCOA_GLAMOR_PRESENTSIGNALMESSAGE_H
#define COCOA_GLAMOR_PRESENTSIGNALMESSAGE_H

#include "Glamor/Glamor.h"
#include "Glamor/PresentMessage.h"
#include "Glamor/PresentSignal.h"
GLAMOR_NAMESPACE_BEGIN

class PresentSignalMessage : public PresentMessage
{
public:
    using SignalCode = uint32_t;

    PresentSignalMessage(std::shared_ptr<PresentSignal> info,
                         const std::shared_ptr<PresentRemoteHandle>& emitter,
                         SignalCode code)
        : PresentMessage(PresentMessage::Type::kSignalEmit)
        , emitter_(emitter)
        , signal_code_(code)
        , signal_info_(std::move(info)) {}
    ~PresentSignalMessage() override = default;

    g_nodiscard std::shared_ptr<PresentRemoteHandle> GetEmitter() const {
        return emitter_;
    }

    g_nodiscard SignalCode GetSignalCode() const {
        return signal_code_;
    }

    g_nodiscard std::shared_ptr<PresentSignal> GetSignalInfo() const {
        return signal_info_;
    }

private:
    std::shared_ptr<PresentRemoteHandle> emitter_;
    SignalCode                           signal_code_;
    std::shared_ptr<PresentSignal>       signal_info_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_PRESENTSIGNALMESSAGE_H
