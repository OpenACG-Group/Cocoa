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

#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/Host.h"
#include "llvm/ExecutionEngine/Orc/ExecutorProcessControl.h"

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Reactor/Reactor.h"
#include "Reactor/JitSession.h"
REACTOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Reactor.Reactor)

void InitializePlatform(const Options& options)
{
    /* Initialize LLVM here */
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    auto EPC = llvm::orc::SelfExecutorProcessControl::Create();
    CHECK(EPC && "llvm::orc::SelfExecutorProcessControl creation failed");

    llvm::orc::JITTargetMachineBuilder JTMB((*EPC)->getTargetTriple());

    llvm::StringMap<bool> cpuFeatures;
    bool ok = llvm::sys::getHostCPUFeatures(cpuFeatures);

#if defined(__i386__) || defined(__x86_64__) || \
    (defined(__linux__) && (defined(__arm__) || defined(__aarch64__)))
    CHECK(ok && "llvm::sys::getHostCPUFeatures returned false");
#else
    // getHostCPUFeatures always returns false on other platforms.
    (void) ok;
#endif

    QLOG(LOG_DEBUG, "Host CPU features for code generation:");
    for (auto& feature : cpuFeatures)
    {
        JTMB.getFeatures().AddFeature(feature.first(), feature.second);
        QLOG(LOG_DEBUG, "  %fg<bl>%italic<>{}%reset: {}",
             feature.first().str(), feature.second ? "%fg<gr>enabled%reset" : "%fg<re>disabled%reset");
    }
    JTMB.setCPU(std::string(llvm::sys::getHostCPUName()));

    auto DL = JTMB.getDefaultDataLayoutForTarget();
    CHECK(DL && "JITTargetMachineBuilder::getDefaultDataLayoutForTarget failed");

    JitSession::New(options, std::move(JTMB), std::move(*DL));
}

void DisposePlatform()
{
    JitSession::Delete();
}

REACTOR_NAMESPACE_END
