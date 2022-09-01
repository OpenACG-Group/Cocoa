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

#include <sys/mman.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include "fmt/format.h"
#include "Core/Journal.h"
#include "Core/Data.h"
#include "Core/subprocess/Message.h"

namespace cocoa {

void recv_fd(int socket)
{
    Journal::New(LOG_LEVEL_DEBUG, Journal::OutputDevice::kStandardOut, true);

    std::vector<int32_t> descriptors;
    auto data = Data::MakeFromSize(1024);

    size_t payload_size;
    subproc::Message::ReceiveMessageInternal(socket, data, &payload_size, descriptors);

    fmt::print("data-payload: {}\n", reinterpret_cast<const char*>(data->getAccessibleBuffer()));
    fmt::print("payload-size: {}\n", payload_size);

    for (int32_t fd : descriptors)
    {
        fmt::print("descriptor: {}\n", fd);
    }

    Journal::Delete();
}

} // namespace cocoa

int main(int argc, const char **argv)
{
    cocoa::recv_fd(STDIN_FILENO);
    return 0;
}
