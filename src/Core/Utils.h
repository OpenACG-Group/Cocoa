#ifndef COCOA_UTILS_H
#define COCOA_UTILS_H

#include <functional>
#include <vector>
#include <string_view>

#include "Core/Exception.h"
#include "Core/Properties.h"
namespace cocoa::utils {

void DumpRuntimeException(const RuntimeException& except);
void DumpProperties(const std::shared_ptr<PropertyNode>& root);
void ChangeWorkDirectory(const std::string& dir);
std::string GetAbsoluteDirectory(const std::string& dir);
std::string GetExecutablePath();
std::string GetCpuModel();
size_t GetMemPageSize();
size_t GetMemTotalSize();
std::vector<std::string_view> SplitString(const std::string& str, std::string::value_type delimiter);

} // namespace cocoa

#endif //COCOA_UTILS_H
