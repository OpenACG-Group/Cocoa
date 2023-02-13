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

#include "Clutter/Clutter.h"
#include "Clutter/ProcessHost.h"
CLUTTER_BEGIN_NAMESPACE

GlobalContext::GlobalContext() = default;
GlobalContext::~GlobalContext() = default;

std::shared_ptr<ProcessHost> GlobalContext::GetProcessHostIf(const Predicate& pred)
{
    for (const auto& host : process_hosts_)
    {
        if (pred(host))
            return host;
    }
    return nullptr;
}

bool GlobalContext::RemoveProcessHostIf(const Predicate& pred)
{
    bool removed = false;

    for (auto itr = process_hosts_.begin(); itr != process_hosts_.end(); itr++)
    {
        if (pred(*itr))
        {
            itr = process_hosts_.erase(itr);
            removed = true;
            if (itr == process_hosts_.end())
                break;
        }
    }

    return removed;
}

CLUTTER_END_NAMESPACE
