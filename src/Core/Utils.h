#ifndef COCOA_UTILS_H
#define COCOA_UTILS_H

#include <functional>

#include "Core/Exception.h"
#include "Core/Properties.h"
namespace cocoa::utils {

void DumpRuntimeException(const RuntimeException& except);
void DumpProperties(const std::shared_ptr<PropertyNode>& root);
void ChangeWorkDirectory(const std::string& dir);
std::string GetAbsoluteDirectory(const std::string& dir);
std::string GetExecutablePath();
size_t GetMemPageSize();

} // namespace cocoa

#endif //COCOA_UTILS_H
