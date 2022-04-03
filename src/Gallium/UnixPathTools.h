#ifndef COCOA_GALLIUM_UNIXPATHTOOLS_H
#define COCOA_GALLIUM_UNIXPATHTOOLS_H

#include "Gallium/Gallium.h"
GALLIUM_NS_BEGIN

namespace unixpath {

/**
 * @note @a path must be a canonical string of path,
 *       which means given by @a vfs::Realpath.
 */
std::string SolveShortestPathRepresentation(const std::string& path);

}

GALLIUM_NS_END
#endif //COCOA_GALLIUM_UNIXPATHTOOLS_H
