#ifndef COCOA_GLAMOR_RENDERHOSTCREATOR_H
#define COCOA_GLAMOR_RENDERHOSTCREATOR_H

#include "Glamor/Glamor.h"
#include "Glamor/RenderClientObject.h"
GLAMOR_NAMESPACE_BEGIN

#define CROP_RENDERHOSTCREATOR_CREATE_DISPLAY      1
#define CROP_RENDERHOSTCERATOT_CREATE_BLENDER      2

#define CRSI_RENDERHOSTCREATOR_CREATED             1

class Display;

class RenderHostCreator : public RenderClientObject
{
public:
    RenderHostCreator();
    ~RenderHostCreator() override = default;

    Shared<RenderClientObject> CreateDisplay(const std::string& name);
    Shared<RenderClientObject> CreateBlender(const Shared<RenderClientObject>& surface);
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_RENDERHOSTCREATOR_H
