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

#ifndef COCOA_REACTOR_GSHADEREXTERNALS_H
#define COCOA_REACTOR_GSHADEREXTERNALS_H

#include "llvm/ADT/StringMap.h"

#include "Reactor/Reactor.h"
REACTOR_NAMESPACE_BEGIN

namespace external {

constexpr const int32_t START_USER_RET_FAILED = 1;
constexpr const int32_t START_USER_RET_NORMAL = 0;
constexpr const uint32_t HOST_CTX_MAGIC_NUMBER = 0x66ccff39U;

/* Function family: Trigonometry */

/**
 * Naming convenience:
 *
 * kCosSinF2R = (x, y) -> (cos(x), sin(x))
 * ^      ^ ^
 * |      | `------- Respectively
 * |      `--------- float2
 * `---------------- Constant
 */

/* float sinf(float): x -> sin(x) */
constexpr const int32_t kSinF = 0x01;

/* float cosf(float): x -> cos(x) */
constexpr const int32_t kCosF = 0x02;

/* float tanf(float): x -> tan(x) */
constexpr const int32_t kTanF = 0x03;

/* float2 sinf2(float2): (x, y) -> (sin(x), sin(y)) */
constexpr const int32_t kSinF2 = 0x04;

/* float2 cosf2(float2): (x, y) -> (cos(x), cos(y)) */
constexpr const int32_t kCosF2 = 0x05;

/* float2 tanf2(float2): (x, y) -> (tan(x), tan(y)) */
constexpr const int32_t kTanF2 = 0x06;

/* float2 sincosf2r(float2): (x, y) -> (sin(x), cos(y)) */
constexpr const int32_t kSinCosF2R = 0x07;

/* float2 cossinf2r(float2): (x, y) -> (cos(x), sin(y)) */
constexpr const int32_t kCosSinF2R = 0x08;

/* Function family: Builtins */

/* void __builtin_v8_trampoline(void *, i32) */
constexpr const int32_t kBuiltinV8Trampoline = 0xa0;

/* i32 __builtin_check_host_context(void *) */
constexpr const int32_t kBuiltinCheckHostContext = 0xa1;

} // namespace anonymous

const llvm::StringMap<void *>& GetExternalSymbolMap();
llvm::FunctionType *GetExternalFunctionType(llvm::LLVMContext& context, int32_t id);
const char *GetExternalFunctionName(int32_t id);

REACTOR_NAMESPACE_END
#endif //COCOA_REACTOR_GSHADEREXTERNALS_H
