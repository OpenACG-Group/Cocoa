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

#ifndef COCOA_CORE_GROUPEDCALLBACKMANAGER_H
#define COCOA_CORE_GROUPEDCALLBACKMANAGER_H

#include <unordered_map>
#include <functional>

#include "Core/Project.h"
#include "Core/Errors.h"
COCOA_BEGIN_NAMESPACE

template<typename GroupT>
class GroupedCallbackManager
{
public:
    static_assert(std::is_enum<GroupT>::value || std::is_integral<GroupT>::value);

    enum class AfterCallBehaviour
    {
        kRemove,
        kOnceMore
    };

    using IdsMap = std::unordered_map<uint64_t, std::function<AfterCallBehaviour(void)>>;
    using GroupsMap = std::unordered_map<GroupT, IdsMap>;

    GroupedCallbackManager() : id_cnt_(0) {}
    ~GroupedCallbackManager() = default;

    g_nodiscard bool HasGroup(GroupT g) const {
        typename GroupsMap::const_iterator itr = g_map_.find(g);
        if (itr == g_map_.cend())
            return false;
        return itr->second.size() > 0;
    }

    uint64_t Add(GroupT g, std::function<AfterCallBehaviour(void)> func) {
        CHECK(func && "Invalid callback");
        uint64_t id = id_cnt_++;
        g_map_[g].emplace(id, std::move(func));
        return id;
    }

    void Remove(GroupT g, uint64_t id) {
        typename GroupsMap::iterator itr = g_map_.find(g);
        if (itr == g_map_.end())
            return;
        itr->second.erase(id);
    }

    void CallGroup(GroupT g) {
        typename GroupsMap::iterator g_itr = g_map_.find(g);
        if (g_itr == g_map_.end() || g_itr->second.size() == 0)
            return;

        auto itr = g_itr->second.begin();
        while (itr != g_itr->second.end())
        {
            CHECK(itr->second && "Invalid callback");
            if (itr->second() == AfterCallBehaviour::kRemove)
                itr = g_itr->second.erase(itr);
            else
                itr++;
        }
    }

private:
    uint64_t        id_cnt_;
    GroupsMap       g_map_;
};

COCOA_END_NAMESPACE
#endif //COCOA_CORE_GROUPEDCALLBACKMANAGER_H
