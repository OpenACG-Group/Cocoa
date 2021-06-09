#ifndef COCOA_SHADERCONTEXT_H
#define COCOA_SHADERCONTEXT_H

#include "include/core/SkCanvas.h"
#include "include/core/SkPicture.h"
#include "include/core/SkImage.h"
#include "include/core/SkShader.h"

#include "Komorebi/Namespace.h"
KOMOREBI_NS_BEGIN

class ShaderValue
{
public:
    enum class Type
    {
        kScalar,
        kVec2f,
        kVec3f,
        kVec4f,
        kString
    };

    union ValueUnion
    {

    };
};

using CoSLScalar = float;

class ShaderContext
{
public:
    ShaderContext(SkCanvas *pCanvas);
    ~ShaderContext();

    void setUniform(const std::string& name, CoSLScalar scalar);
    void setUniform(const std::string& name, const char *str);
    void setUniform(const std::string& name, const sk_sp<SkImage>& image);
    void setUniform(const std::string& name, const sk_sp<SkShader>& shader);

private:
    SkCanvas    *fCanvas;
};

KOMOREBI_NS_END
#endif //COCOA_SHADERCONTEXT_H
