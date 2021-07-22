#ifndef COCOA_MODULEURL_H
#define COCOA_MODULEURL_H

#include <string>
#include <tuple>

#include "include/v8.h"
#include "Scripter/ScripterBase.h"
SCRIPTER_NS_BEGIN

std::tuple<v8::MaybeLocal<v8::String>, std::string>
        ResolveModuleImportUrl(v8::Isolate *isolate,
                               const std::string& refererUrl,
                               const std::string& specifier);

SCRIPTER_NS_END
#endif //COCOA_MODULEURL_H
