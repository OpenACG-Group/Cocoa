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

#include "Core/Journal.h"
#include "Glamor/Moe/MoeJITContext.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Moe.MoeJITContext)

MoeJITContext::CodeHolderPtr MoeJITContext::GetInitializedCodeHolder()
{
    CodeHolderPtr ptr = std::make_unique<asmjit::CodeHolder>();
    asmjit::Error error = ptr->init(jit_runtime_.environment());
    if (error != asmjit::ErrorCode::kErrorOk)
    {
        QLOG(LOG_ERROR, "Failed in initializing code holder for JIT compiler: {}",
             asmjit::DebugUtils::errorAsString(error));
        return nullptr;
    }

    return ptr;
}



GLAMOR_NAMESPACE_END
