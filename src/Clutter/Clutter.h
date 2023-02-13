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

#ifndef COCOA_CLUTTER_CLUTTER_H
#define COCOA_CLUTTER_CLUTTER_H

#include <cstdint>
#include <memory>
#include <list>
#include <functional>

#include "Core/UniquePersistent.h"
#include "Core/Project.h"
#include "Core/Errors.h"

#define CLUTTER_BEGIN_NAMESPACE namespace cocoa::clutter {
#define CLUTTER_END_NAMESPACE   }

CLUTTER_BEGIN_NAMESPACE

class ProcessHost;

enum class EmbeddedService
{
    kClutterDesktopIntegration
};

enum class ServiceStatus
{
    kRunning,       //!< Service is running
    kStopped,       //!< Service was stopped or has not been started yet
    kStarting,      //!< Service is starting (will become `kRunning` soon)
    kStopping,      //!< Service is stopping (will become `kStopped` soon)
    kTerminated,    //!< Service was terminated unexpectedly (signal, program crash, etc.)
};

class GlobalContext : public UniquePersistent<GlobalContext>
{
public:
    GlobalContext();
    ~GlobalContext();

    g_inline std::shared_ptr<ProcessHost>& AddProcessHost(std::shared_ptr<ProcessHost> host) {
        CHECK(host);
        process_hosts_.emplace_back(std::move(host));
        return process_hosts_.back();
    }

    using Predicate = std::function<bool(const std::shared_ptr<ProcessHost>&)>;

    std::shared_ptr<ProcessHost> GetProcessHostIf(const Predicate& pred);
    bool RemoveProcessHostIf(const Predicate& pred);

private:
    std::list<std::shared_ptr<ProcessHost>> process_hosts_;
};

CLUTTER_END_NAMESPACE
#endif //COCOA_CLUTTER_CLUTTER_H
