#ifndef COCOA_SHADERLIBRARY_H
#define COCOA_SHADERLIBRARY_H

#include <cstdint>
#include <memory>

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
#define vas_exported __attribute__((__visibility__("default")))
#else
#define vas_exported
#endif

#ifdef __cplusplus
#define VAS_EXPORTS_BEGIN   extern "C" {
#define VAS_EXPORTS_END     }
#else
#define VAS_EXPORTS_BEGIN
#define VAS_EXPORTS_END
#endif // __cplusplus

#define vas_mangle_name(name) __vshader_##name
#define vas_mangle_name_str(name) "__vshader_"#name

struct VaShaderHostInterface
{
    enum class MessageType
    {
        kDebug,
        kInfo,
        kWarning,
        kError
    };

    void *closure;

    void (*messenger)(MessageType, const char *, void *);
    uint32_t (*getCpuCount)(void*);
};

/* Library Description Table */
struct VaShaderLDT
{
    enum class Feature : uint32_t
    {
        kLang = (1 << 0)
    };

    const char         *vendor;
    uint32_t            version[3];
    uint32_t            featuresNum;
    Feature             features[128];

    bool (*initializer)(std::shared_ptr<VaShaderHostInterface> host) noexcept;
    void (*finalizer)() noexcept;
};

std::shared_ptr<VaShaderHostInterface> GetHostInterface();

#endif //COCOA_SHADERLIBRARY_H
