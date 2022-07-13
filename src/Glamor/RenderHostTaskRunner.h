#ifndef COCOA_GLAMOR_RENDERHOSTTASKRUNNER_H
#define COCOA_GLAMOR_RENDERHOSTTASKRUNNER_H

#include <functional>

#include "Glamor/Glamor.h"
#include "Glamor/RenderClientObject.h"
GLAMOR_NAMESPACE_BEGIN

#define CROP_TASKRUNNER_RUN     1

class RenderHostTaskRunner : public RenderClientObject
{
public:
    using Task = std::function<void(void)>;

    RenderHostTaskRunner();
    ~RenderHostTaskRunner() override = default;

    g_async_api void Run(const Task& task);
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_RENDERHOSTTASKRUNNER_H
