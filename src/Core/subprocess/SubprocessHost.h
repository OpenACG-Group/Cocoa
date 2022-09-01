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

#ifndef COCOA_CORE_SUBPROCESS_SUBPROCESSHOST_H
#define COCOA_CORE_SUBPROCESS_SUBPROCESSHOST_H

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <list>

#include "uv.h"
#include "Core/Project.h"
#include "Core/Data.h"
#include "Core/UniquePersistent.h"

namespace cocoa::subproc {

class HostMessageListener;
class MessageBuilder;

class SubprocessHost
{
public:
    struct Options
    {
        using StringVector = std::vector<std::string>;
        template<typename T> using Opt = std::optional<T>;

        // Path pointing to the program to be executed.
        std::string       executable_path;

        // Current working directory of the subprocess.
        // If empty, the subprocess will inherit it from the host process.
        std::string       working_directory;

        // If true, current environment variables will be passed to the subprocess.
        bool              inherit_envs;

        // Environment variables passed to the subprocess.
        // This is not affected by `inherit_envs`
        StringVector      extra_env;

        // Commandline arguments passed to the subprocess.
        // In the subprocess, contents that you provided in `args` here start from `argv[1]`
        // in `main` function, while `argv[0]` is `executable_path`.
        StringVector      args;

        // Optional user and group ID of subprocess.
        Opt<uv_uid_t>     uid;
        Opt<uv_gid_t>     gid;

        // If true, a communication protocol via socketpair between the subprocess and host process
        // is enabled. The subprocess should create an instance of `SubprocessClient` to handle
        // the messages, and the host process should handle messages by message listeners.
        // This IPC mechanism is suitable for file descriptor passing between processes.
        bool              take_over_ipc;
    };

    SubprocessHost();
    ~SubprocessHost();

    g_nodiscard static std::shared_ptr<SubprocessHost> Run(uv_loop_t *loop,
                                                           const Options& options);

    g_nodiscard g_inline uv_process_t *GetUvProcess() {
        return &uv_process_;
    }

    g_nodiscard g_inline uv_pipe_t *GetUvIpcPipe() {
        return &ipc_pipe_stream_;
    }

    g_nodiscard g_inline uv_stream_t *GetUvIpcPipeAsStream() {
        return reinterpret_cast<uv_stream_t*>(&ipc_pipe_stream_);
    }

    g_nodiscard g_inline const auto& GetMessageListeners() {
        return listeners_;
    }

    void AddMessageListener(const std::shared_ptr<HostMessageListener>& listener);
    void RemoveMessageListener(const std::shared_ptr<HostMessageListener>& listener);

    bool SendMessageFromBuilder(MessageBuilder& builder);

private:
    uv_process_t         uv_process_;
    bool                 ipc_enabled_;
    uv_pipe_t            ipc_pipe_stream_;
    std::list<std::shared_ptr<HostMessageListener>> listeners_;
};

class SubprocessHostRegistry : public UniquePersistent<SubprocessHostRegistry>
{
public:
    SubprocessHostRegistry() = default;
    ~SubprocessHostRegistry() = default;

    void AddProcess(const std::shared_ptr<SubprocessHost>& proc);
    void RemoveProcess(const std::shared_ptr<SubprocessHost>& proc);
    void RemoveProcess(SubprocessHost *proc);

private:
    std::list<std::shared_ptr<SubprocessHost>> procs_list_;
};

} // namespace cocoa::subproc
#endif //COCOA_CORE_SUBPROCESS_SUBPROCESSHOST_H
