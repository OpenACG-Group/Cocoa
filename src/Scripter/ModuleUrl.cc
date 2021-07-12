#include <tuple>
#include <fstream>
#include <streambuf>
#include <unistd.h>

#include "Scripter/ScripterBase.h"
#include "Scripter/ModuleUrl.h"
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
    // TODO: We always returns nullptr because there's no any internal modules yet.
    return nullptr;
}

std::string simplify_and_absolutize_url(const std::string& url)
{
    // TODO: Implement this.
}

} // namespace anonymous

v8::MaybeLocal<v8::String> ResolveModuleImportUrl(v8::Isolate *isolate,
                                                  const std::string& refererUrl,
                                                  const std::string& specifier)
{
    if (std::get<0>(is_internal_module_url(refererUrl)))
        return v8::MaybeLocal<v8::String>();

    auto [isInternal, internalName] = is_internal_module_url(specifier);
    if (isInternal)
    {
        const char *str = get_internal_module(internalName);
        if (str == nullptr)
            return v8::MaybeLocal<v8::String>();
        return v8::String::NewFromUtf8(isolate, str);
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

    std::ifstream fs(simplify_and_absolutize_url(result));
    if (!fs.is_open())
        return v8::MaybeLocal<v8::String>();

    std::string content((std::istreambuf_iterator<char>(fs)),
                        std::istreambuf_iterator<char>());
    return v8::String::NewFromUtf8(isolate, content.c_str());
}

SCRIPTER_NS_END
