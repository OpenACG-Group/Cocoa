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

#include "uv.h"

#include "Clutter/ServiceClient/ServiceClient.h"
CLUTTER_SERVICE_CLIENT_NS_BEGIN

std::unique_ptr<HostConnection> HostConnection::Connect()
{
    auto app = std::make_unique<HostConnection>();

    // Stdin is redirected to the IPC socketpair by parent process
    app->ipc_socketpair_fd_ = 1;

    return app;
}

HostConnection::HostConnection()
    : ipc_socketpair_fd_(-1)
{
}

HostConnection::~HostConnection() = default;

CLUTTER_SERVICE_CLIENT_NS_END
