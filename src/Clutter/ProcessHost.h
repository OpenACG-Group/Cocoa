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

#ifndef COCOA_CLUTTER_PROCESSHOST_H
#define COCOA_CLUTTER_PROCESSHOST_H

#include <string>
#include <vector>
#include <optional>

#include "uv.h"

#include "Clutter/Clutter.h"
CLUTTER_BEGIN_NAMESPACE

class ProcessListener;

class ProcessHost
{
public:
    struct Options
    {
        using StringVector = std::vector<std::string>;
        template<typename T>
        using Maybe = std::optional<T>;

        std::string         execfile;
        std::string         working_dir;
        StringVector        args;
        bool                inherit_envs = false;
        StringVector        envs;
        Maybe<uv_uid_t>     uid;
        Maybe<uv_gid_t>     gid;
    };

    struct ProcessExitInfo
    {
        int ret;
        int signal;
    };

    static std::shared_ptr<ProcessHost> Start(const Options& options);

    ProcessHost();
    ~ProcessHost();

    void Dispose();

    g_nodiscard g_inline bool IsDisposed() const {
        return disposed_;
    }

    g_nodiscard g_inline ServiceStatus GetStatus() const {
        return current_status_;
    }

    g_nodiscard g_inline std::shared_ptr<ProcessListener> GetProcessListener() const {
        return listener_;
    }

    g_inline void RemoveProcessListener() {
        listener_.reset();
    }

    g_inline void SetProcessListener(std::shared_ptr<ProcessListener> listener) {
        listener_ = std::move(listener);
    }

private:
    static void on_process_exit(uv_process_t *proc, int64_t status, int signal);
    static void on_ipc_poll_dispatch(uv_poll_t *poll, int status, int events);

    bool                                disposed_;
    uv_process_t                       *uv_process_;
    std::shared_ptr<ProcessListener>    listener_;
    ServiceStatus                       current_status_;
    std::optional<ProcessExitInfo>      exit_info_;
    uv_poll_t                          *ipc_socketpair_poll_;
    int                                 ipc_socketpair_parent_fd_;
};

CLUTTER_END_NAMESPACE
#endif //COCOA_CLUTTER_PROCESSHOST_H
