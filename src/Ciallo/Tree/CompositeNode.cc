#include <memory>

#include "Ciallo/Ciallo.h"
#include "Ciallo/Tree/CompositeNode.h"
CIALLO_BEGIN_NS

CompositeNode::CompositeNode(std::shared_ptr<GrBaseCompositor>&& compositor)
    : BaseNode(NodeKind::kCompositeNode, nullptr),
      fCompositor(compositor)
{
}

std::shared_ptr<GrBaseCompositor> CompositeNode::asCompositor()
{
    return fCompositor;
}

CompositeNode::NodeBackendKind CompositeNode::backendKind() const
{
    switch (fCompositor->getDeviceType())
    {
    case CompositeDevice::kCpuDevice:
        return NodeBackendKind::kRaster;
    case CompositeDevice::kGpuVulkan:
        return NodeBackendKind::kGpu;
    case CompositeDevice::kOpenCL:
        return NodeBackendKind::kRaster;
    }
}

CIALLO_END_NS
