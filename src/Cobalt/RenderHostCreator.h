#ifndef COCOA_RENDERHOSTCREATOR_H
#define COCOA_RENDERHOSTCREATOR_H

#include "Cobalt/Cobalt.h"
#include "Cobalt/RenderClientObject.h"
COBALT_NAMESPACE_BEGIN

#define COBALT_OP_RENDERHOSTCREATOR_CREATE_DISPLAY      1

#define COBALT_SI_RENDERHOSTCREATOR_CREATED             1

class Display;

class RenderHostCreator : public RenderClientObject
{
public:
    RenderHostCreator();
    ~RenderHostCreator() override = default;

    co_sp<Display> CreateDisplay(const std::string& name);
};

COBALT_NAMESPACE_END
#endif //COCOA_RENDERHOSTCREATOR_H
