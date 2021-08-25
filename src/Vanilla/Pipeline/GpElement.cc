#include <utility>
#include <cassert>

#include "Vanilla/Base.h"
#include "Vanilla/Pipeline/GpElement.h"
#include "Vanilla/Pipeline/GpPipeline.h"
VANILLA_NS_BEGIN

GpElement::GpElement(Type type,
                     std::string name,
                     const std::vector<GpLinkage::Transfer::Category>& inPadCategories,
                     const std::vector<GpLinkage::Transfer::Category>& outPadCategories,
                     const Handle<GpPipeline>& pipeline)
    : fType(type)
    , fName(std::move(name))
    , fInPads(static_cast<int>(inPadCategories.size()))
    , fOutPads(static_cast<int>(outPadCategories.size()))
    , fInPadsLinkage(fInPads)
    , fOutPadsLinkage(fOutPads)
    , fInPadsCategories(inPadCategories)
    , fOutPadsCategories(outPadCategories)
    , fState(State::kNormal)
    , fPipeline(pipeline)
{
}

void GpElement::setLinkageForInPad(int pad, GpLinkage *linkage)
{
    assert(pad < fInPads && pad >= 0);
    assert(linkage);
    fInPadsLinkage[pad] = linkage;
}

void GpElement::setLinkageForOutPad(int pad, GpLinkage *linkage)
{
    assert(pad < fOutPads && pad >= 0);
    assert(linkage);
    fOutPadsLinkage[pad] = linkage;
}

GpLinkage *GpElement::getLinkageForInPad(int pad)
{
    if (pad >= fInPads)
        return nullptr;
    return fInPadsLinkage[pad];
}

GpLinkage *GpElement::getLinkageForOutPad(int pad)
{
    if (pad >= fOutPads)
        return nullptr;
    return fOutPadsLinkage[pad];
}

VANILLA_NS_END
