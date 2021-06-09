#ifndef COCOA_OPERATIONS_H
#define COCOA_OPERATIONS_H

#include "virtfs/Base.h"
VFS_NS_BEGIN

/**
 * @brief Create a absolute root. Then you can call
 *        Mount() to mount a root filesystem.
 */
void Initialize();
void Finalize();

void Mkdir(const std::string& path, const std::string& dir);
void Chdir(const std::string& dir);

void Mount(const std::string& driver, const std::string& device,
           const std::string& mountpoint, const std::string& extraOptions = "");
void Umount(const std::string& mountpoint);

VFS_NS_END
#endif // COCOA_OPERATIONS_H
