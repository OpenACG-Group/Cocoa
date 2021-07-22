#include <tuple>
#include <fstream>
#include <streambuf>

#include "Core/Properties.h"
#include "Core/Utils.h"
#include "Scripter/ScripterBase.h"
#include "Scripter/ModuleUrl.h"
#include "Scripter/Internals.h"
SCRIPTER_NS_BEGIN

namespace {

std::tuple<bool, std::string_view> is_internal_module_url(const std::string& url)
{
    if (!url.starts_with("internal:"))
        return {false, std::string_view()};

    std::string_view view(url);
    view.remove_prefix(std::string("internal:").length());
    return {true, view};
}

const char *get_internal_module(const std::string_view& name)
{
    return GetInternalScript(std::string(name));
}

std::string simplify_and_absolutize_url(std::string url)
{
    if (!url.starts_with('/'))
    {
        auto workingDir = prop::Cast<PropertyDataNode>(prop::Get()->next("runtime")->next("working-path"))
                ->extract<std::string>();
        workingDir.push_back('/');
        workingDir.append(url);
        url = workingDir;
    }
    return utils::GetAbsoluteDirectory(url);
}

} // namespace anonymous

std::tuple<v8::MaybeLocal<v8::String>, std::string>
        ResolveModuleImportUrl(v8::Isolate *isolate,
                               const std::string& refererUrl,
                               const std::string& specifier)
{
    if (std::get<0>(is_internal_module_url(refererUrl)))
        return {v8::MaybeLocal<v8::String>(), std::string()};

    auto [isInternal, internalName] = is_internal_module_url(specifier);
    if (isInternal)
    {
        const char *str = get_internal_module(internalName);
        if (str == nullptr)
            return {v8::MaybeLocal<v8::String>(), std::string()};
        return {v8::String::NewFromUtf8(isolate, str), {internalName.begin(), internalName.end()}};
    }

    std::string result;
    if (specifier.starts_with('/'))
        result = specifier;
    else
    {
        auto where = refererUrl.find_last_of('/');
        if (where != std::string::npos)
            result = refererUrl.substr(0, where + 1);
        result.append(specifier);
    }

    std::string path = simplify_and_absolutize_url(result);

    std::ifstream fs(path);
    if (!fs.is_open())
    {
        if (path.ends_with(".js"))
            return {v8::MaybeLocal<v8::String>(), std::string()};
        else
        {
            path.append(".js");
            fs = std::ifstream(path);
            if (!fs.is_open())
                return {v8::MaybeLocal<v8::String>(), std::string()};
        }
    }

    std::string content((std::istreambuf_iterator<char>(fs)),
                        std::istreambuf_iterator<char>());
    return {v8::String::NewFromUtf8(isolate, content.c_str()), path};
}

SCRIPTER_NS_END
