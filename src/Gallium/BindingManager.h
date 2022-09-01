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

#ifndef COCOA_GALLIUM_BINDINGMANAGER_H
#define COCOA_GALLIUM_BINDINGMANAGER_H

#include <vector>

#include "Core/UniquePersistent.h"
#include "Gallium/Gallium.h"
#include "Gallium/Runtime.h"
GALLIUM_NS_BEGIN

namespace bindings { class BindingBase; }

class BindingManager : public UniquePersistent<BindingManager>
{
public:
    explicit BindingManager(const Runtime::Options& options);
    ~BindingManager();

    g_nodiscard inline bool isAllowOverride() const {
        return fAllowOverride;
    }

    void loadDynamicObject(const std::string& path);
    bindings::BindingBase *search(const std::string& name);

    static void NotifyIsolateHasCreated(v8::Isolate *isolate);

private:
    bool appendBinding(bindings::BindingBase *ptr);

    bool                            fAllowOverride;
    std::vector<std::string>        fBlacklist;
    std::vector<bindings::BindingBase*> fBindings;
    std::vector<void*>              fLibHandles;
};

GALLIUM_NS_END
#endif //COCOA_GALLIUM_BINDINGMANAGER_H
