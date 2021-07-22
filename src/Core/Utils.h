#ifndef COCOA_UTILS_H
#define COCOA_UTILS_H

#include <functional>

#include "Core/Exception.h"
namespace cocoa::utils {

void DumpRuntimeException(const RuntimeException& except);

void ChangeWorkDirectory(const std::string& dir);
std::string GetAbsoluteDirectory(const std::string& dir);
std::string GetExecutablePath();

} // namespace cocoa

#endif //COCOA_UTILS_H
