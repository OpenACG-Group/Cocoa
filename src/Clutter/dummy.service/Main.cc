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

#include "Clutter/ServiceClient/ServiceClient.h"

namespace cocoa {

int service_main(int argc, const char **argv)
{
    auto host_connection = clutter::HostConnection::Connect();
    if (!host_connection)
        return 1;

    return 0;
}

} // namespace cocoa

int main(int argc, const char **argv)
{
    return cocoa::service_main(argc, argv);
}
