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
