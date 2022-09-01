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

#ifndef COCOA_GLAMOR_RENDERCLIENTSIGNALEMIT_H
#define COCOA_GLAMOR_RENDERCLIENTSIGNALEMIT_H

#include "Glamor/Glamor.h"
#include "Glamor/RenderClientTransfer.h"
#include "Glamor/RenderClientEmitterInfo.h"
GLAMOR_NAMESPACE_BEGIN

class RenderClientSignalEmit : public RenderClientTransfer
{
public:
    using SignalCode = uint32_t;

    RenderClientSignalEmit(RenderClientEmitterInfo info, const Shared<RenderClientObject>& emitter,
                           SignalCode code)
        : RenderClientTransfer(RenderClientTransfer::Type::kSignalEmit)
        , emitter_(emitter)
        , signal_code_(code)
        , args_vector_(info.MoveArgs()) {}
    ~RenderClientSignalEmit() override = default;

    g_nodiscard g_inline Shared<RenderClientObject> GetEmitter() const {
        return emitter_;
    }

    g_nodiscard g_inline std::vector<std::any>& GetArgs() {
        return args_vector_;
    }

    g_nodiscard g_inline SignalCode GetSignalCode() const {
        return signal_code_;
    }

private:
    Shared<RenderClientObject>   emitter_;
    SignalCode                  signal_code_;
    std::vector<std::any>       args_vector_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_RENDERCLIENTSIGNALEMIT_H
