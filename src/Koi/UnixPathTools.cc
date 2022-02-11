#include "Core/Properties.h"
#include "Core/Errors.h"
#include "Core/Utils.h"
#include "Koi/UnixPathTools.h"

KOI_NS_BEGIN
namespace unixpath
{

std::string SolveShortestPathRepresentation(const std::string& path)
{
    CHECK(utils::StrStartsWith(path, '/') && "Path should be an absolute path");
    auto cwd = prop::Cast<PropertyDataNode>(prop::Get()
                ->next("Runtime")
                ->next("CurrentPath"))
                ->extract<std::string>();
    CHECK(utils::StrStartsWith(path, '/') && "Runtime.CurrentPath should be an absolute path");
    if (cwd != "/" && !utils::StrStartsWith(path, '/'))
        cwd += '/';

    /* Find the longest common prefix (LCP) [0, cp) of `path` and `cwd` */
    int32_t cp = 0;
    while (cp < path.length() && cp < cwd.length() && path[cp] == cwd[cp])
        cp++;

    std::string result;
    for (int32_t i = cp; i < cwd.length(); i++)
    {
        if (cwd[i] == '/')
            result += "../";
    }
    result += path.substr(cp);
    return (result.length() < path.length()) ? result : path;
}

} // namespace unixpath
KOI_NS_END
