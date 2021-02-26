#ifndef COCOA_GROPENCLCOMPOSITOR_H
#define COCOA_GROPENCLCOMPOSITOR_H

#include <memory>
#include <string>

#include <CL/cl.h>

#include "Ciallo/GrBase.h"
#include "Ciallo/DDR/GrBaseCompositor.h"
CIALLO_BEGIN_NS

struct ClDeviceProperties
{
    ::cl_device_type        fDeviceType;
    ::cl_uint               fVendorId;
    ::cl_uint               fMaxComputeUnits;
    ::cl_uint               fMaxWorkItemDimensions;
    size_t                  fMaxWorkGroupSize;
    ::cl_uint               fMaxClockFrequency;
    ::cl_bool               fImageSupport;
    size_t                  fMaxImage2dWidth;
    size_t                  fMaxImage2dHeight;
    ::cl_uint               fMaxSamplers;
    ::cl_bool               fAvailable;
    ::cl_bool               fCompilerAvailable;
    std::string             fName;
    std::string             fVendor;
    std::string             fDriverVersion;
    std::string             fVersion;
    ::size_t                fPreferredWorkGroupSizeMultiple;
};

class GrOpenCLCompositor : public GrBaseCompositor
{
public:
    GrOpenCLCompositor(int32_t width,
                       int32_t height,
                       GrColorFormat colorFormat,
                       GrBasePlatform *platform);
    ~GrOpenCLCompositor() override;

    static std::shared_ptr<GrBaseCompositor> MakeOpenCL(int32_t width,
                                                        int32_t height,
                                                        GrColorFormat colorFormat,
                                                        GrBasePlatform *platform,
                                                        const std::string& platformKeyword,
                                                        const std::string& deviceKeyword);

private:
    void createOpenCL(const std::string& platformKeyword,
                      const std::string& deviceKeyword);
    bool isPlatformSuitable(::cl_platform_id platformId, const std::string& keyword);
    bool isDeviceSuitable(::cl_device_id deviceId, const std::string& keyword);
    bool isDevicePropertiesSuitable(const ClDeviceProperties& props, const std::string& keyword);
    void prepareOpenCL();
    static void printProgramBuildLog(const std::string& log);

    void clComposite(::cl_mem target, ::cl_mem image,
                     const SkRect& srcClip, const SkRect& dstClip) override;
    GrTargetSurface onTargetSurface() override;
    void onPresent() override;
    GrBaseRenderLayer *onCreateRenderLayer(int32_t width,
                                           int32_t height,
                                           int32_t left,
                                           int32_t top,
                                           int zindex) override;

    void toRetChecked(::cl_int ret, const char *func, const char *cl_func);

    void computePreferredWorkSize();
    void computeWorkSize(int32_t width, int32_t height);

    static cl_channel_order ToOpenCLChannelOrder(GrColorFormat colorFormat);

private:
    ::cl_platform_id            fClPlatformId;
    ::cl_device_id              fClDeviceId;
    ::cl_context                fClContext;
    ::cl_command_queue          fClCommandQueue;
    ::cl_program                fClProgram;
    ::cl_kernel                 fClSrcOverKernel;
    ::cl_mem                    fImages[2];
    int8_t                      fCurrentOutputImage;
    ClDeviceProperties          fClDeviceProperties;
    ::cl_event                  fInWaitingEvent;
    uint8_t                    *fHostBuffer;

    size_t                      fGlobalWorkSize[2]{};
    size_t                      fLocalWorkSize[2]{};
};

CIALLO_END_NS
#endif //COCOA_GROPENCLCOMPOSITOR_H
