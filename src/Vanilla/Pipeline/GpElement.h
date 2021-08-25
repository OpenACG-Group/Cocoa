#ifndef COCOA_GPELEMENT_H
#define COCOA_GPELEMENT_H

#include <string>
#include <vector>
#include <atomic>

#include "Vanilla/Base.h"
#include "Vanilla/Pipeline/GpLinkage.h"
VANILLA_NS_BEGIN

class GpPipeline;

class GpElement : public std::enable_shared_from_this<GpElement>
{
    friend class GpPipeline;

public:
    enum class Type
    {
        /* Who generates data. Src elements have no input pads. */
        kSrc,
        /* A sink element means an output device. Sink elements have no output pads. */
        kSink,
        kProcessor
    };

    enum class State
    {
        kQueued,
        kRunning,
        kNormal
    };

    GpElement(Type type,
              std::string name,
              const std::vector<GpLinkage::Transfer::Category>& inPadCategories,
              const std::vector<GpLinkage::Transfer::Category>& outPadCategories,
              const Handle<GpPipeline>& pipeline);
    virtual ~GpElement() = default;

    va_nodiscard inline Type getType() const
    { return fType; }
    va_nodiscard inline const std::string& getName() const
    { return fName; }
    va_nodiscard inline Handle<GpPipeline> getPipeline() const
    { return fPipeline.lock(); }

    va_nodiscard inline int getInPads() const
    { return fInPads; }
    va_nodiscard inline int getOutPads() const
    { return fOutPads; }

    va_nodiscard inline auto& getInPadsCategories()
    { return fInPadsCategories; }
    va_nodiscard inline auto& getOutPadsCategories()
    { return fOutPadsCategories; }

    va_nodiscard inline State getState()
    { return fState; }

    virtual void process() = 0;
    /**
     * If this element is mutually exclusive with the `other` element
     * in concurrency (that means you can't call `process` method of them
     * at the same time or concurrently), returns true.
     * Otherwise returns false.
     */
    virtual bool isMutuallyExclusiveInConcurrencyWith(GpElement *other) = 0;

protected:
    void setLinkageForInPad(int pad, GpLinkage *linkage);
    void setLinkageForOutPad(int pad, GpLinkage *linkage);
    GpLinkage *getLinkageForInPad(int pad);
    GpLinkage *getLinkageForOutPad(int pad);
    inline void setState(State st)
    { fState = st; }

private:
    Type                            fType;
    std::string                     fName;
    int                             fInPads;
    int                             fOutPads;
    std::vector<GpLinkage*>         fInPadsLinkage;
    std::vector<GpLinkage*>         fOutPadsLinkage;
    std::vector<GpLinkage::Transfer::Category>
                                    fInPadsCategories;
    std::vector<GpLinkage::Transfer::Category>
                                    fOutPadsCategories;
    State                           fState;
    WeakHandle<GpPipeline>          fPipeline;
};

VANILLA_NS_END
#endif //COCOA_GPELEMENT_H
