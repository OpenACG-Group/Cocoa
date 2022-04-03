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

    co_nodiscard const cpu_features::X86Info& getX86Info() {
        return fInfo;
    }

private:
    cpu_features::X86Info       fInfo;
};

}

#endif //COCOA_CORE_CPUINFO_H
