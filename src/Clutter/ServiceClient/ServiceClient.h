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

#ifndef COCOA_CLUTTER_SERVICE_CLIENT_SERVICECLIENT_H
#define COCOA_CLUTTER_SERVICE_CLIENT_SERVICECLIENT_H

#include <memory>
#include <cstdint>

#include "uv.h"

#include "Core/Project.h"

#define CLUTTER_SERVICE_CLIENT_NS_BEGIN namespace cocoa::clutter {
#define CLUTTER_SERVICE_CLIENT_NS_END   }

CLUTTER_SERVICE_CLIENT_NS_BEGIN

class HostConnection
{
public:
    HostConnection();
    ~HostConnection();

    static std::unique_ptr<HostConnection> Connect();

    g_nodiscard g_inline int GetIpcSocketPairFd() const {
        return ipc_socketpair_fd_;
    }

private:
    int             ipc_socketpair_fd_;
};

CLUTTER_SERVICE_CLIENT_NS_END
#endif //COCOA_CLUTTER_SERVICE_CLIENT_SERVICECLIENT_H
