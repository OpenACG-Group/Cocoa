#ifndef COCOA_CORE_PROCESSSIGNAL_H
#define COCOA_CORE_PROCESSSIGNAL_H

namespace cocoa {

/**
 * Primary signal handlers which handle the signal interruption directly.
 * Signal will interrupt the execution of program and the signal will be
 * handled immediately. Stack backtrace is supported.
 */
void InstallPrimarySignalHandler();

/**
 * Secondary signal handlers are based on main event loop.
 * Signal will not interrupt the execution of program and the signal will be
 * handled in main event loop.
 */
void InstallSecondarySignalHandler();

void BeforeEventLoopEntrypointHook();

} // namespace cocoa
#endif //COCOA_CORE_PROCESSSIGNAL_H
