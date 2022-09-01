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

#ifndef COCOA_CORE_SUBPROCESS_MESSAGE_H
#define COCOA_CORE_SUBPROCESS_MESSAGE_H

#include <cstdint>

#include "Core/Project.h"
#include "Core/Data.h"
#include "Core/Errors.h"

namespace cocoa::subproc {

struct MessageHeaderPayload
{
    enum MessageType
    {
        kSharedMemoryRequest    = 0xa1,
        kUserPayload            = 0xa2
    };

    constexpr static uint32_t kMagic = 0x7cc8ffaa;

    uint32_t        magic;
    uint8_t         type;
};

class Message
{
public:
    constexpr static size_t kMaxPayloadSize = 8192ULL;
    constexpr static size_t kMaxDescriptorCount = 16ULL;

    g_private_api static bool SendMessageInternal(int32_t sockfd,
                                                  const std::vector<std::shared_ptr<Data>>& data_payloads,
                                                  const std::vector<int32_t>& descriptor_payloads);

    g_private_api static bool ReceiveMessageInternal(int32_t sockfd,
                                                     const std::shared_ptr<Data>& out_buffer,
                                                     size_t *data_payloads_size,
                                                     std::vector<int32_t>& descriptor_payloads);
};

class MessageBuilder
{
public:
    MessageBuilder() = default;
    ~MessageBuilder() = default;

    g_inline MessageBuilder& AddContent(const std::shared_ptr<Data>& data) {
        CHECK(data);
        data_list_.push_back(data);
        return *this;
    }

    g_inline MessageBuilder& AddDescriptor(int32_t fd) {
        CHECK(fd >= 0);
        fds_list_.push_back(fd);
        return *this;
    }

    g_private_api g_nodiscard g_inline const auto& GetDataList() {
        return data_list_;
    }

    g_private_api g_nodiscard g_inline const auto& GetDescriptorList() {
        return fds_list_;
    }

private:
    std::vector<std::shared_ptr<Data>> data_list_;
    std::vector<int32_t> fds_list_;
};

} // namespace cocoa::subproc
#endif //COCOA_CORE_SUBPROCESS_MESSAGE_H
