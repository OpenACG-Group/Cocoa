#ifndef COCOA_COBALT_RENDERHOSTCREATOR_H
#define COCOA_COBALT_RENDERHOSTCREATOR_H

#include "Cobalt/Cobalt.h"
#include "Cobalt/RenderClientObject.h"
COBALT_NAMESPACE_BEGIN

#define CROP_RENDERHOSTCREATOR_CREATE_DISPLAY      1

#define CRSI_RENDERHOSTCREATOR_CREATED             1

class Display;

class RenderHostCreator : public RenderClientObject
{
public:
    RenderHostCreator();
    ~RenderHostCreator() override = default;

    co_sp<RenderClientObject> CreateDisplay(const std::string& name);
};

COBALT_NAMESPACE_END
#endif //COCOA_COBALT_RENDERHOSTCREATOR_H
