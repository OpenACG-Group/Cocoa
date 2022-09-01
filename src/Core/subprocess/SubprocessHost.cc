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

#include <unistd.h>
#include <sys/socket.h>
#include <sys/mman.h>

#include "Core/Journal.h"
#include "Core/subprocess/SubprocessHost.h"
#include "Core/subprocess/HostMessageListener.h"
#include "Core/subprocess/Message.h"
#include "Core/Exception.h"
#include "Core/Errors.h"
#include "Core/ScalableWriteBuffer.h"
namespace cocoa::subproc {

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Core.subprocess.SubprocessHost)

namespace {

void on_process_exit(uv_process_t *proc, int64_t status, int signal)
{
    CHECK(proc && proc->data);
    auto *process = reinterpret_cast<SubprocessHost*>(proc->data);

    for (const auto& listener : process->GetMessageListeners())
    {
        listener->OnSubprocessExit(status, signal);
    }

    uv_close(reinterpret_cast<uv_handle_t*>(proc), [](uv_handle_t *hnd) {
        CHECK(hnd && hnd->data);
        auto *process = reinterpret_cast<SubprocessHost*>(hnd->data);
        SubprocessHostRegistry::Ref().RemoveProcess(process);
    });
}

} // namespace anonymous

void SubprocessHostRegistry::AddProcess(const std::shared_ptr<SubprocessHost>& proc)
{
    procs_list_.push_back(proc);
}

void SubprocessHostRegistry::RemoveProcess(SubprocessHost *proc)
{
    procs_list_.remove_if([proc](const std::shared_ptr<SubprocessHost>& host) {
        return (proc == host.get());
    });
}

void SubprocessHostRegistry::RemoveProcess(const std::shared_ptr<SubprocessHost>& proc)
{
    procs_list_.remove(proc);
}

std::shared_ptr<SubprocessHost> SubprocessHost::Run(uv_loop_t *loop,
                                                    const SubprocessHost::Options& options)
{
    CHECK(loop);
    auto proc = std::make_shared<SubprocessHost>();
    proc->ipc_enabled_ = options.take_over_ipc;

    uv_process_options_t uv_options{};
    uv_options.exit_cb = on_process_exit;
    uv_options.file = options.executable_path.c_str();

    // Set working directory
    std::string cwd;
    if (options.working_directory.empty())
    {
        char buf[PATH_MAX];
        std::memset(buf, 0, PATH_MAX);
        if (!getcwd(buf, PATH_MAX))
        {
            QLOG(LOG_ERROR, "Failed to get current working directory: {}", strerror(errno));
            return nullptr;
        }
        cwd = buf;
    }
    else
    {
        cwd = options.working_directory;
    }
    uv_options.cwd = cwd.c_str();

    // Set uid and gid
    uv_options.flags = (options.uid ? UV_PROCESS_SETUID : 0)
                     | (options.gid ? UV_PROCESS_SETGID : 0);
    uv_options.uid = options.uid.value_or(0);
    uv_options.gid = options.gid.value_or(0);

    // Set stdio containers
    auto stdio_containers = std::make_unique<uv_stdio_container_t[]>(3);

    if (options.take_over_ipc)
    {
        // If IPC is enabled, we replace the stdin of the subprocess with
        // the socketpair pipe, because we cannot pass other file descriptors
        // except stdin, stdout and stderr to the subprocess.
        uv_pipe_init(loop, &proc->ipc_pipe_stream_, true);
        proc->ipc_pipe_stream_.data = proc.get();

        constexpr int flags = UV_CREATE_PIPE | UV_WRITABLE_PIPE | UV_READABLE_PIPE;
        stdio_containers[0].flags = static_cast<uv_stdio_flags>(flags);
        stdio_containers[0].data.stream = reinterpret_cast<uv_stream_t*>(&proc->ipc_pipe_stream_);
    }
    else
    {
        stdio_containers[0].flags = UV_INHERIT_FD;
        stdio_containers[0].data.fd = STDIN_FILENO;
    }

    stdio_containers[1].flags = UV_INHERIT_FD;
    stdio_containers[1].data.fd = STDOUT_FILENO;
    stdio_containers[2].flags = UV_INHERIT_FD;
    stdio_containers[2].data.fd = STDERR_FILENO;
    uv_options.stdio_count = 3;
    uv_options.stdio = stdio_containers.get();

    // Set environment variables
    size_t n_envs = options.extra_env.size();
    int inherit_env_count = 0;
    if (options.inherit_envs)
    {
        char **p = environ;
        while (*p && ++inherit_env_count)
            p++;
        n_envs += inherit_env_count;
    }

    auto envs_vec = std::make_unique<char*[]>(n_envs + 1);
    if (inherit_env_count > 0)
        std::memcpy(envs_vec.get(), environ, sizeof(char*) * inherit_env_count);

    for (int i = 0; i < options.extra_env.size(); i++)
        envs_vec[inherit_env_count + i] = const_cast<char*>(options.extra_env[i].c_str());

    envs_vec[n_envs] = nullptr;
    uv_options.env = envs_vec.get();

    // Set commandline arguments
    auto args_vec = std::make_unique<char*[]>(options.args.size() + 2);
    args_vec[0] = const_cast<char*>(options.executable_path.c_str());
    for (int i = 0; i < options.args.size(); i++)
        args_vec[i + 1] = const_cast<char*>(options.args[i].c_str());

    args_vec[options.args.size() + 2] = nullptr;
    uv_options.args = args_vec.get();

    // Start the process
    if (uv_spawn(loop, &proc->uv_process_, &uv_options) < 0)
    {
        QLOG(LOG_ERROR, "Failed to execute {}: {}", options.executable_path, uv_err_name(errno));
        return nullptr;
    }
    proc->uv_process_.data = proc.get();
    SubprocessHostRegistry::Ref().AddProcess(proc);

    QLOG(LOG_DEBUG, "Spawn subprocess {}", options.executable_path);
    return proc;
}


SubprocessHost::SubprocessHost()
    : uv_process_{}
    , ipc_enabled_(false)
    , ipc_pipe_stream_{}
{
}

SubprocessHost::~SubprocessHost()
{
    // TODO: do some cleanup
}

void SubprocessHost::AddMessageListener(const std::shared_ptr<HostMessageListener>& listener)
{
    if (std::find(listeners_.begin(), listeners_.end(), listener) != listeners_.end())
        return;
    listeners_.push_back(listener);
}

void SubprocessHost::RemoveMessageListener(const std::shared_ptr<HostMessageListener>& listener)
{
    listeners_.remove(listener);
}

bool SubprocessHost::SendMessageFromBuilder(MessageBuilder& builder)
{
    if (!ipc_enabled_)
        return false;

    int32_t socket_fd;
    uv_fileno(reinterpret_cast<uv_handle_t*>(&ipc_pipe_stream_), &socket_fd);

    return Message::SendMessageInternal(socket_fd,
                                        builder.GetDataList(),
                                        builder.GetDescriptorList());
}

} // namespace cocoa::subproc
