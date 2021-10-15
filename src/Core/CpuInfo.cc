#include "Core/CpuInfo.h"

namespace cocoa {

CpuInfo::CpuInfo()
    : fInfo(cpu_features::GetX86Info())
{
}

}
