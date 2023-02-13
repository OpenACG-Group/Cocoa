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
#include <sys/wait.h>
#include <cstring>

#include "Core/Journal.h"
#include "Core/EventLoop.h"
#include "Core/Exception.h"
#include "Clutter/ProcessHost.h"
CLUTTER_BEGIN_NAMESPACE

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Clutter.ProcessHost)

std::shared_ptr<ProcessHost> ProcessHost::Start(const Options& options)
{
    uv_loop_t *loop = EventLoop::Ref().handle();

    auto host = std::make_shared<ProcessHost>();

    uv_process_options_t uvopt{};
    uvopt.exit_cb = &ProcessHost::on_process_exit;
    uvopt.file = options.execfile.c_str();

    // Set working directory
    std::string working_dir = options.working_dir;
    if (working_dir.empty())
    {
        char buf[PATH_MAX];
        std::memset(buf, 0, PATH_MAX);
        if (!getcwd(buf, PATH_MAX))
        {
            QLOG(LOG_ERROR, "Failed get current working directory: {}", strerror(errno));
            return nullptr;
        }
        working_dir = buf;
    }
    uvopt.cwd = working_dir.c_str();

    // Set subprocess UID and GID
    uvopt.flags |= (options.uid ? UV_PROCESS_SETUID : 0)
                 | (options.gid ? UV_PROCESS_SETGID : 0);
    uvopt.uid = options.uid.value_or(0);
    uvopt.gid = options.gid.value_or(0);

    // Setup IPC socketpair
    int socketpair_fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socketpair_fds) < 0)
    {
        QLOG(LOG_ERROR, "Failed to create IPC socketpair: {}", strerror(errno));
        return nullptr;
    }

    host->ipc_socketpair_parent_fd_ = socketpair_fds[0];
    host->ipc_socketpair_poll_ = new uv_poll_t;
    uv_poll_init(loop, host->ipc_socketpair_poll_, socketpair_fds[0]);
    uv_poll_start(host->ipc_socketpair_poll_, UV_READABLE | UV_DISCONNECT,
                  &ProcessHost::on_ipc_poll_dispatch);
    host->ipc_socketpair_poll_->data = host.get();

    ScopeExitAutoInvoker socketpair_fds_closer([socketpair_fds] {
        // As parent process, we can close the child IPC socket fd
        close(socketpair_fds[1]);
    });

    ScopeExitAutoInvoker fail_disposer([host] {
        host->Dispose();
    });

    // Set stdio containers
    std::array<uv_stdio_container_t, 3> stdio_containers{};
    stdio_containers[0].flags = UV_INHERIT_FD;
    stdio_containers[0].data.fd = socketpair_fds[1];

    // stdout and stderr can be written by subprocess freely
    for (int i = 1; i <= 2; i++)
    {
        stdio_containers[i].flags = UV_INHERIT_FD;
        stdio_containers[i].data.fd = i;
    }

    // Set environment variables
    size_t n_envs = options.envs.size();
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

    for (int i = 0; i < options.envs.size(); i++)
        envs_vec[inherit_env_count + i] = const_cast<char*>(options.envs[i].c_str());

    envs_vec[n_envs] = nullptr;
    uvopt.env = envs_vec.get();

    // Set commandline arguments
    auto args_vec = std::make_unique<char*[]>(options.args.size() + 2);
    args_vec[0] = const_cast<char*>(options.execfile.c_str());
    for (int i = 0; i < options.args.size(); i++)
        args_vec[i + 1] = const_cast<char*>(options.args[i].c_str());

    args_vec[options.args.size() + 2] = nullptr;
    uvopt.args = args_vec.get();

    host->uv_process_ = new uv_process_t;
    if (int ret = uv_spawn(loop, host->uv_process_, &uvopt))
    {
        QLOG(LOG_ERROR, "Failed to execute {}: {}", options.execfile, uv_err_name(ret));
        return nullptr;
    }
    host->uv_process_->data = host.get();

    // TODO(sora): Handshake Stage: send child process "hello" message

    // Now the subprocess is executed and a new process has been created successfully.
    // The subprocess is expected to respond our "hello" message through IPC.
    // Once we have received that response from subprocess, we change `current_status_`
    // to `kRunning`, which means that the handshake stage has been accomplished successfully
    // and the subprocess is running normally.
    host->current_status_ = ServiceStatus::kStarting;

    fail_disposer.cancel();
    return GlobalContext::Ref().AddProcessHost(std::move(host));
}

void ProcessHost::on_process_exit(uv_process_t *proc, int64_t status, int signal)
{
}

void ProcessHost::on_ipc_poll_dispatch(uv_poll_t *poll, int status, int events)
{
}

ProcessHost::ProcessHost()
    : disposed_(false)
    , uv_process_(nullptr)
    , current_status_(ServiceStatus::kStopped)
    , ipc_socketpair_poll_(nullptr)
    , ipc_socketpair_parent_fd_(-1)
{
}

ProcessHost::~ProcessHost()
{
    CHECK(disposed_);
}

void ProcessHost::Dispose()
{
    if (disposed_)
        return;

    if (uv_process_ && (current_status_ != ServiceStatus::kStopped &&
        current_status_ != ServiceStatus::kTerminated))
    {
        uv_process_kill(uv_process_, SIGTERM);
        // TODO(sora): use `waitpid` to wait subprocess to terminate
        current_status_ = ServiceStatus::kTerminated;
    }

    if (uv_process_)
    {
        uv_close(reinterpret_cast<uv_handle_t*>(uv_process_),
                 [](uv_handle_t *handle) {
            delete reinterpret_cast<uv_process_t*>(handle);
        });

        uv_process_ = nullptr;
    }

    if (ipc_socketpair_poll_)
    {
        uv_close(reinterpret_cast<uv_handle_t*>(ipc_socketpair_poll_),
                 [](uv_handle_t *handle) {
            delete reinterpret_cast<uv_poll_t*>(handle);
        });
        close(ipc_socketpair_parent_fd_);

        ipc_socketpair_parent_fd_ = -1;
        ipc_socketpair_poll_ = nullptr;
    }

    disposed_ = true;
}

CLUTTER_END_NAMESPACE
