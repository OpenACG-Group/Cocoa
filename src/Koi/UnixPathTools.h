#ifndef COCOA_UNIXPATHTOOLS_H
#define COCOA_UNIXPATHTOOLS_H

#include "Koi/KoiBase.h"
KOI_NS_BEGIN

namespace unixpath {

/**
 * @note @a path must be a canonical string of path,
 *       which means given by @a vfs::Realpath.
 */
std::string SolveShortestPathRepresentation(const std::string& path);

}

KOI_NS_END
#endif //COCOA_UNIXPATHTOOLS_H
