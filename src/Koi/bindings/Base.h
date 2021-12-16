#ifndef COCOA_LANG_BASE_H
#define COCOA_LANG_BASE_H

#include <string>
#include <functional>
#include <utility>

#include "Koi/KoiBase.h"
#include "Koi/binder/Module.h"
#include "Koi/binder/Factory.h"

KOI_BINDINGS_NS_BEGIN

#define JS_THROW_IF(cond, msg, ...)                                                     \
    do {                                                                                \
        if ((cond)) {                                                                   \
            binder::throw_(v8::Isolate::GetCurrent(), msg __VA_OPT__(,) __VA_ARGS__);   \
            return;                                                                     \
        }                                                                               \
    } while (false)

#define JS_THROW_RET_IF(cond, msg, ret, ...)                                            \
    do {                                                                                \
        if ((cond)) {                                                                   \
            binder::throw_(v8::Isolate::GetCurrent(), msg __VA_OPT__(,) __VA_ARGS__);   \
            return ret;                                                                 \
        }                                                                               \
    } while (false)


class BindingBase
{
public:
    BindingBase(std::string name, std::string desc)
        : fName(std::move(name)),
          fDescription(std::move(desc)) {}
    virtual ~BindingBase() = default;

    koi_nodiscard inline const std::string& name() const
    { return fName; }
    koi_nodiscard inline const std::string& description() const
    { return fDescription; }

    virtual const char *getUniqueId() { return nullptr; }

    virtual void setInstanceProperties(v8::Local<v8::Object> instance) {}

    /**
     * Override but don't implement this virtual method
     * because the corresponding implementation will be generated automatically.
     */
    virtual void getModule(binder::Module& mod) = 0;

    /**
     * Override but don't implement this virtual method
     * because the corresponding implementation will be generated automatically.
     */
    virtual const char **getExports() = 0;

private:
    std::string   fName;
    std::string   fDescription;
};

KOI_BINDINGS_NS_END
#endif //COCOA_LANG_BASE_H
