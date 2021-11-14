#include <tuple>
#include <fstream>
#include <optional>
#include <streambuf>

#include "Core/Properties.h"
#include "Core/Utils.h"
#include "Core/Filesystem.h"
#include "Koi/KoiBase.h"
#include "Koi/ModuleUrl.h"
#include "Koi/Internals.h"
KOI_NS_BEGIN

namespace {

std::optional<std::string_view> ExtractInternal(const std::string& url)
{
    if (!url.starts_with("internal:"))
        return {};

    std::string_view view(url);
    view.remove_prefix(std::string("internal:").length());
    return std::make_optional(view);
}

std::string NormalizeFilePath(std::string url)
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

std::tuple<v8::MaybeLocal<v8::String>, std::string> ResolveLocalFileModule(v8::Isolate *isolate,
                                                                           const std::string& refererUrl,
                                                                           const std::string& specifier)
{
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

    std::string path = NormalizeFilePath(result);
    try
    {
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
    catch (const std::runtime_error& e)
    {
        return {{}, std::string()};
    }
}

} // namespace anonymous

std::tuple<v8::MaybeLocal<v8::String>, std::string>
        ResolveModuleImportUrl(v8::Isolate *isolate,
                               const std::string& refererUrl,
                               const std::string& specifier)
{
    /* Synthetic modules is not allowed to import other modules */
    if (refererUrl.starts_with("synthetic://"))
        return {v8::MaybeLocal<v8::String>(), std::string()};

    /* A special case if specifier refers to an internal module */
    if (auto internalName = ExtractInternal(specifier))
    {
        const char *str = GetInternalScript(std::string(*internalName));
        if (str == nullptr)
            return {v8::MaybeLocal<v8::String>(), std::string()};
        return {v8::String::NewFromUtf8(isolate, str), std::string(*internalName)};
    }

   return ResolveLocalFileModule(isolate, refererUrl, specifier);
}

KOI_NS_END
