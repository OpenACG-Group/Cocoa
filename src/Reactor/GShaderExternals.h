#ifndef COCOA_REACTOR_GSHADEREXTERNALS_H
#define COCOA_REACTOR_GSHADEREXTERNALS_H

#include "llvm/ADT/StringMap.h"

#include "Reactor/Reactor.h"
REACTOR_NAMESPACE_BEGIN

namespace external {

constexpr const int32_t START_USER_RET_FAILED = 1;
constexpr const int32_t START_USER_RET_NORMAL = 0;
constexpr const uint32_t HOST_CTX_MAGIC_NUMBER = 0x66ccff39U;

} // namespace anonymous

const llvm::StringMap<void *>& GetExternalSymbolMap();

REACTOR_NAMESPACE_END
#endif //COCOA_REACTOR_GSHADEREXTERNALS_H
