#ifndef COCOA_COREBINDING_H
#define COCOA_COREBINDING_H

#include "Koi/lang/Base.h"
#include "Koi/Runtime.h"

KOI_LANG_NS_BEGIN

class CoreBindingModule : public BaseBindingModule
{
public:
    struct Options
    {
        static Options CopyRuntime(const Runtime::Options& opts) {
            Options result;
            result.script_args = opts.script_args;
            return result;
        }

        Options() = default;
        Options(const Options& lhs) = default;
        Options(Options&& rhs) noexcept : script_args(std::move(rhs.script_args)) {}

        std::vector<std::string> script_args;
    };

    CoreBindingModule();
    ~CoreBindingModule() override;
    void getModule(binder::Module& self) override;

private:
    Options     fOptions;
};

KOI_LANG_NS_END
#endif //COCOA_COREBINDING_H
