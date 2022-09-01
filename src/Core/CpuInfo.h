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

#ifndef COCOA_CORE_CPUINFO_H
#define COCOA_CORE_CPUINFO_H

#include "Core/Project.h"
#include "Core/UniquePersistent.h"
#include "include/cpuinfo_x86.h"

namespace cocoa {

class CpuInfo : public UniquePersistent<CpuInfo>
{
public:
    CpuInfo();
    ~CpuInfo() = default;

    g_nodiscard const cpu_features::X86Info& getX86Info() {
        return fInfo;
    }

private:
    cpu_features::X86Info       fInfo;
};

}

#endif //COCOA_CORE_CPUINFO_H
