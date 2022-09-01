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

#include "Core/Journal.h"
#include "Core/subprocess/Message.h"
namespace cocoa::subproc {

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Core.subprocess.Message)

bool Message::SendMessageInternal(int32_t sockfd,
                                  const std::vector<std::shared_ptr<Data>>& data_payloads,
                                  const std::vector<int32_t>& descriptor_payloads)
{
    CHECK(sockfd);

    // Empty messages are invalid
    if (data_payloads.empty())
        return false;

    // Prepare message header structure
    struct msghdr host_msg{};
    std::memset(&host_msg, 0, sizeof(host_msg));

    // Fill binary datas
    size_t n_contents = data_payloads.size();
    std::vector<iovec> iovec_array(n_contents);
    std::vector<std::shared_ptr<Data>> duplicated_buffers;
    for (size_t i = 0; i < n_contents; i++)
    {
        std::shared_ptr<Data> data = data_payloads[i];
        if (!data->hasAccessibleBuffer())
        {
            data = Data::MakeLinearBuffer(data);
            CHECK(data);
            duplicated_buffers.push_back(data);
        }

        CHECK(data->hasAccessibleBuffer());

        iovec_array[i].iov_len = data->size();
        iovec_array[i].iov_base = const_cast<void*>(data->getAccessibleBuffer());
    }

    host_msg.msg_iov = iovec_array.data();
    host_msg.msg_iovlen = iovec_array.size();

    // Fill file descriptors
    size_t n_descriptors = descriptor_payloads.size();
    // Data payload for cmsghdr struct
    std::vector<uint8_t> aux_data_pod;

    if (n_descriptors > 0)
    {
        aux_data_pod.resize(CMSG_SPACE(sizeof(int32_t) * n_descriptors));
        memset(aux_data_pod.data(), 0, aux_data_pod.size());

        host_msg.msg_controllen = aux_data_pod.size();
        host_msg.msg_control = aux_data_pod.data();

        struct cmsghdr *ctl_header = CMSG_FIRSTHDR(&host_msg);
        CHECK(ctl_header);

        // NOLINTNEXTLINE
        ctl_header->cmsg_level = SOL_SOCKET;
        ctl_header->cmsg_type = SCM_RIGHTS;
        ctl_header->cmsg_len = aux_data_pod.size();
    }

    if (sendmsg(sockfd, &host_msg, 0) < 0)
    {
        QLOG(LOG_ERROR, "Error calling sendmsg: {}", strerror(errno));
        return false;
    }

    return true;
}

bool Message::ReceiveMessageInternal(int32_t sockfd,
                                     const std::shared_ptr<Data>& out_buffer,
                                     size_t *data_payloads_size,
                                     std::vector<int32_t>& descriptor_payloads)
{
    struct msghdr msg{};
    memset(&msg, 0, sizeof(msg));

    // Prepare buffer for data payloads
    std::shared_ptr<Data> write_buffer = out_buffer;
    struct iovec iov{};
    if (out_buffer)
    {
        if (!out_buffer->hasAccessibleBuffer())
        {
            write_buffer = Data::MakeFromSize(out_buffer->size());
            CHECK(write_buffer);
        }

        iov.iov_len = write_buffer->size();
        iov.iov_base = const_cast<void*>(write_buffer->getAccessibleBuffer());
        msg.msg_iovlen = 1;
        msg.msg_iov = &iov;
    }

    // Prepare buffer for descriptors
    constexpr size_t cmsg_pod_size = CMSG_SPACE(sizeof(int32_t) * kMaxDescriptorCount);
    uint8_t cmsg_pod[cmsg_pod_size];
    msg.msg_controllen = cmsg_pod_size;
    msg.msg_control = cmsg_pod;

    // Receive datas from socketpair
    ssize_t result = recvmsg(sockfd, &msg, MSG_CMSG_CLOEXEC);
    if (result < 0)
    {
        QLOG(LOG_ERROR, "Error calling recvmsg: {}", strerror(errno));
        return false;
    }

    if (msg.msg_flags & MSG_TRUNC)
    {
        QLOG(LOG_ERROR, "Error receiving messages: Buffer is too small");
        return false;
    }

    if (msg.msg_flags & MSG_CTRUNC)
    {
        QLOG(LOG_ERROR, "Error receiving messages: control data lost before delivery");
        return false;
    }

    if (write_buffer != out_buffer)
    {
        out_buffer->write(write_buffer->getAccessibleBuffer(),
                          write_buffer->size());
    }

    // Extract file descriptors
    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    if (!cmsg || cmsg->cmsg_type != SCM_RIGHTS)
    {
        // No file descriptors are sent.
        return true;
    }

    size_t fd_count = (cmsg->cmsg_len - sizeof(struct cmsghdr)) / sizeof(int32_t);
    if (fd_count == 0)
    {
        QLOG(LOG_WARNING, "No file descriptors should be received");
        return false;
    }

    auto *fds = reinterpret_cast<int32_t*>(CMSG_DATA(cmsg));
    for (size_t i = 0; i < fd_count; i++)
        descriptor_payloads.push_back(fds[i]);

    if (data_payloads_size)
        *data_payloads_size = result;

    return true;
}

} // namespace cocoa::subproc
