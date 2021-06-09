#include <vector>
#include <string>
#include <sstream>
#include <iostream>

std::vector<std::string> split_path(const std::string& path)
{
    std::vector<std::string> result;
    std::string segment;
    for (char i : path)
    {
        if (i == '/' && !segment.empty())
        {
            result.push_back(std::move(segment));
            segment = "";
        }
        else if (i != '/')
            segment.push_back(i);
    }
    if (!segment.empty())
        result.push_back(std::move(segment));
    return result;
}

std::string NormalizePath(const std::string& path,
                          const std::string& currentDir)
{
    std::string absolute;
    if (path[0] == '/')
        absolute = path;
    else
        absolute = currentDir + '/' + path;
    auto segments = split_path(absolute);

    std::vector<std::string> result;
    for (const std::string& seg : segments)
    {
        if (seg == "..")
        {
            if (!result.empty())
                result.pop_back();
        }
        else if (seg != ".")
            result.push_back(seg);
    }

    std::ostringstream oss;
    for (const std::string& p : result)
        oss << '/' << p;
    return oss.str();
}

int main(int argc, const char **argv)
{
    std::cout << NormalizePath(argv[1], argv[2]) << std::endl;
    return 0;
}
