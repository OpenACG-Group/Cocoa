#include <string>

#include "Koi/KoiBase.h"
#include "Koi/Internals.h"
KOI_NS_BEGIN

namespace {

struct InternalScript
{
    const char *name;
    const char *script;
};

#define SCRIPT_TITLE(s) .name = (s),
#define SCRIPT_BODY     .script

const InternalScript internals[] = {
        {
            SCRIPT_TITLE("core")
            SCRIPT_BODY = R"(
/**
 * CocoaJs Core Object, Javascript ES6 Standard.
 * Copyright(C) 2021, Jingheng Luo (masshiro.io@qq.com)
 */
export let Core = new Object();
(function (core) {
    core.opCall = (name, args) => {
        return __cocoa_op_call(name, args);
    };

    core.getScripterInfo = () => {
        return {
            version: [1, 0, 0],
            manufacture: "org.OpenACG.Cocoa",
            capabilities: ["capabilities::lang", "capabilities::opCall"]
        };
    };
})(Core);
)"
        }
};

} // namespace anonymous

const char *GetInternalScript(const std::string& name)
{
    for (const InternalScript& internal : internals)
    {
        if (name == internal.name)
            return internal.script;
    }
    return nullptr;
}

KOI_NS_END
