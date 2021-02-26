#include <memory>
#include <vector>

#include <CL/cl.h>
#include <CL/cl_platform.h>

#include "Core/Journal.h"
#include "Core/MeasuredTable.h"
#include "Ciallo/GrBase.h"
#include "Ciallo/DDR/GrBaseCompositor.h"
#include "Ciallo/DDR/GrOpenCLCompositor.h"
#include "Ciallo/DDR/GrBasePlatform.h"
#include "Ciallo/DDR/GrOpenCLRenderLayer.h"
CIALLO_BEGIN_NS

#define RET_CHECKED(cl_func, ...) \
    this->toRetChecked(::cl_func(__VA_ARGS__), __FUNCTION__, #cl_func)

#define BLEND_SRC_OVER_KERNEL_NAME     "blend_src_over_premultiplied"

static char const _clCompositeSrcOverProgram[] = "const sampler_t blend_sampler = CLK_NORMALIZED_COORDS_FALSE |\n"
                                                 "                                CLK_ADDRESS_CLAMP_TO_EDGE   |\n"
                                                 "                                CLK_FILTER_NEAREST;\n"
                                                 "\n"
                                                 "__kernel void blend_src_over_premultiplied(__read_only image2d_t src,\n"
                                                 "                                           __read_only image2d_t dst,\n"
                                                 "                                           __write_only image2d_t out,\n"
                                                 "                                           int2 src_clip_pos,\n"
                                                 "                                           int2 dst_clip_pos,\n"
                                                 "                                           int2 clip_dim)\n"
                                                 "{\n"
                                                 "    int2 item_id = (int2)(get_global_id(0),\n"
                                                 "                          get_global_id(1));\n"
                                                 "    if (item_id.s0 >= clip_dim.s0)\n"
                                                 "        return;\n"
                                                 "\n"
                                                 "    int2 src_pos = item_id + src_clip_pos;\n"
                                                 "    int2 dst_pos = item_id + dst_clip_pos;\n"
                                                 "\n"
                                                 "    float4 src_pixel = read_imagef(src, blend_sampler, src_pos);\n"
                                                 "    float4 dst_pixel = read_imagef(dst, blend_sampler, dst_pos);\n"
                                                 "    float4 final_pixel = src_pixel + (dst_pixel * (1 - src_pixel.s3));\n"
                                                 "    final_pixel.s3 = 1.0f;\n"
                                                 "    write_imagef(out, dst_pos, final_pixel);\n"
                                                 "}";

constexpr size_t _clSrcOverProgramSize = sizeof(_clCompositeSrcOverProgram) - 1;

std::shared_ptr<GrBaseCompositor> GrOpenCLCompositor::MakeOpenCL(int32_t width,
                                                                 int32_t height,
                                                                 GrColorFormat colorFormat,
                                                                 GrBasePlatform *platform,
                                                                 const std::string &platformKeyword,
                                                                 const std::string &deviceKeyword)
{
    std::shared_ptr<GrOpenCLCompositor> ret = std::make_shared<GrOpenCLCompositor>(width,
                                                                                   height,
                                                                                   colorFormat,
                                                                                   platform);
    if (ret == nullptr)
        return nullptr;
    ret->createOpenCL(platformKeyword, deviceKeyword);
    return ret;
}

GrOpenCLCompositor::GrOpenCLCompositor(int32_t width,
                                       int32_t height,
                                       GrColorFormat colorFormat,
                                       GrBasePlatform *platform)
    : GrBaseCompositor(CompositeDevice::kOpenCL,
                       width,
                       height,
                       colorFormat,
                       platform),
      fClPlatformId(nullptr),
      fClDeviceId(nullptr),
      fClContext(nullptr),
      fClCommandQueue(nullptr),
      fClProgram(nullptr),
      fClSrcOverKernel(nullptr),
      fImages{nullptr, nullptr},
      fCurrentOutputImage(0),
      fInWaitingEvent(nullptr),
      fHostBuffer(nullptr)
{
    fHostBuffer = static_cast<uint8_t*>(std::malloc(width * height * sizeof(uint32_t)));
    if (!fHostBuffer)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Failed to create host buffer")
                .make<RuntimeException>();
    }
}

GrOpenCLCompositor::~GrOpenCLCompositor()
{
    this->Dispose();

    if (fHostBuffer)
        std::free(fHostBuffer);

    if (fInWaitingEvent != nullptr)
        ::clReleaseEvent(fInWaitingEvent);
    for (auto& fImage : fImages)
        if (fImage != nullptr)
            ::clReleaseMemObject(fImage);
    if (fClSrcOverKernel)
        ::clReleaseKernel(fClSrcOverKernel);
    if (fClProgram)
        ::clReleaseProgram(fClProgram);
    if (fClCommandQueue)
        ::clReleaseCommandQueue(fClCommandQueue);
    if (fClContext)
        ::clReleaseContext(fClContext);
    if (fClDeviceId)
        ::clReleaseDevice(fClDeviceId);
}

CL_CALLBACK static void opencl_context_notifier(const char *errInfo,
                                                [[maybe_unused]] const void *privateInfo,
                                                [[maybe_unused]] size_t cb,
                                                void *_user_data)
{
    auto *_this = reinterpret_cast<GrOpenCLCompositor*>(_user_data);
    log_write(LOG_ERROR) << "<OpenCL.Context> [GrBaseCompositor@"
                         << static_cast<GrBaseCompositor*>(_this) << "] "
                         << errInfo << log_endl;
}

void GrOpenCLCompositor::createOpenCL(const std::string &platformKeyword,
                                      const std::string &deviceKeyword)
{
    // Choose a platform
    ::cl_uint numPlatforms;
    RET_CHECKED(clGetPlatformIDs, 0, nullptr, &numPlatforms);
    if (numPlatforms == 0)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("No available OpenCL platforms")
                .make<RuntimeException>();
    }

    std::vector<::cl_platform_id> platformIds(numPlatforms);
    RET_CHECKED(clGetPlatformIDs, numPlatforms, platformIds.data(), nullptr);

    for (size_t i = 0; i < numPlatforms; i++)
    {
        if (isPlatformSuitable(platformIds[i], platformKeyword))
        {
            fClPlatformId = platformIds[i];
            break;
        }
    }
    if (fClPlatformId == nullptr)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("No any suitable OpenCL platforms")
                .make<RuntimeException>();
    }

    // Choose a device
    ::cl_uint numDevices;
    RET_CHECKED(clGetDeviceIDs, fClPlatformId, CL_DEVICE_TYPE_ALL, 0, nullptr, &numDevices);
    if (numDevices == 0)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("No any available OpenCL devices")
                .make<RuntimeException>();
    }

    std::vector<::cl_device_id> deviceIds(numDevices);
    RET_CHECKED(clGetDeviceIDs,
                fClPlatformId,
                CL_DEVICE_TYPE_ALL,
                numDevices,
                deviceIds.data(),
                nullptr);

    for (size_t i = 0; i < numDevices; i++)
    {
        if (isDeviceSuitable(deviceIds[i], deviceKeyword))
        {
            fClDeviceId = deviceIds[i];
            break;
        }
    }
    if (fClDeviceId == nullptr)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("No any suitable OpenCL devices")
                .make<RuntimeException>();
    }

    // Create context and a command queue
    const ::cl_context_properties ctxProps[] = {
            CL_CONTEXT_PLATFORM,
            reinterpret_cast<::cl_context_properties>(fClPlatformId),
            0
    };
    ::cl_int errCode;
    fClContext = ::clCreateContext(ctxProps,
                                   1,
                                   &fClDeviceId,
                                   opencl_context_notifier,
                                   this,
                                   &errCode);
    this->toRetChecked(errCode, __FUNCTION__, "clCreateContext");

    fClCommandQueue = ::clCreateCommandQueueWithProperties(fClContext,
                                                           fClDeviceId,
                                                           nullptr,
                                                           &errCode);
    this->toRetChecked(errCode, __FUNCTION__, "clCreateCommandQueueWithProperties");

    prepareOpenCL();
}

bool GrOpenCLCompositor::isPlatformSuitable(::cl_platform_id platformId, const std::string &keyword)
{
    std::string name, vendor, profile, version;
    size_t strSize;

#define PROP_STR(prop, var) \
    RET_CHECKED(clGetPlatformInfo, platformId, prop, 0, nullptr, &strSize); \
    var.resize(strSize);    \
    RET_CHECKED(clGetPlatformInfo, platformId, prop, strSize, var.data(), nullptr); \
    var.pop_back();

    PROP_STR(CL_PLATFORM_PROFILE, profile)
    PROP_STR(CL_PLATFORM_VERSION, version)
    PROP_STR(CL_PLATFORM_NAME, name)
    PROP_STR(CL_PLATFORM_VENDOR, vendor)

#undef PROP_STR

    log_write(LOG_DEBUG) << "New OpenCL platform: \"" << name << "\"" << log_endl;
    MeasuredTable measuredTable;
    measuredTable.append("Name:", name);
    measuredTable.append("Vendor:", vendor);
    measuredTable.append("Profile:", profile);
    measuredTable.append("Version:", version);
    measuredTable.flush([](const std::string& str) -> void {
        log_write(LOG_DEBUG) << "  " << str << log_endl;
    });

    if (!keyword.empty() && name.find(keyword) == std::string::npos)
        return false;
    log_write(LOG_DEBUG) << "Using platform above" << log_endl;
    return true;
}

bool GrOpenCLCompositor::isDeviceSuitable(::cl_device_id deviceId, const std::string &keyword)
{
    ClDeviceProperties props;
    size_t strSize;

#define PROP(prop, field) \
    RET_CHECKED(clGetDeviceInfo, deviceId, prop, sizeof(props.field), &props.field, nullptr);

#define PROP_STR(prop, field) \
    RET_CHECKED(clGetDeviceInfo, deviceId, prop, 0, nullptr, &strSize); \
    props.field.resize(strSize);                                        \
    RET_CHECKED(clGetDeviceInfo, deviceId, prop, strSize, props.field.data(), nullptr); \
    props.field.pop_back();

    PROP(CL_DEVICE_TYPE, fDeviceType)
    PROP(CL_DEVICE_VENDOR_ID, fVendorId)
    PROP(CL_DEVICE_MAX_COMPUTE_UNITS, fMaxComputeUnits)
    PROP(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, fMaxWorkItemDimensions)
    PROP(CL_DEVICE_MAX_WORK_GROUP_SIZE, fMaxWorkGroupSize)
    PROP(CL_DEVICE_MAX_CLOCK_FREQUENCY, fMaxClockFrequency)
    PROP(CL_DEVICE_IMAGE_SUPPORT, fImageSupport)
    PROP(CL_DEVICE_IMAGE2D_MAX_WIDTH, fMaxImage2dWidth)
    PROP(CL_DEVICE_IMAGE2D_MAX_HEIGHT, fMaxImage2dHeight)
    PROP(CL_DEVICE_MAX_SAMPLERS, fMaxSamplers)
    PROP(CL_DEVICE_AVAILABLE, fAvailable)
    PROP(CL_DEVICE_COMPILER_AVAILABLE, fCompilerAvailable)

    PROP_STR(CL_DEVICE_NAME, fName)
    PROP_STR(CL_DEVICE_VENDOR, fVendor)
    PROP_STR(CL_DEVICE_VERSION, fVersion)
    PROP_STR(CL_DRIVER_VERSION, fDriverVersion)

#undef PROP
#undef PROP_STR

    return isDevicePropertiesSuitable(props, keyword);
}

bool GrOpenCLCompositor::isDevicePropertiesSuitable(const ClDeviceProperties &props,
                                                    const std::string &keyword)
{
    log_write(LOG_DEBUG) << "New OpenCL device: \"" << props.fName << "\"" << log_endl;

    MeasuredTable measuredTable;
    measuredTable.append("Vendor ID:", props.fVendorId);
    measuredTable.append("Max compute units:", props.fMaxComputeUnits);
    measuredTable.append("Max work item dimensions:", props.fMaxWorkItemDimensions);
    measuredTable.append("Max work group size:", props.fMaxWorkGroupSize);
    measuredTable.append("Max clock frequency (MHz):", props.fMaxClockFrequency);
    measuredTable.append("Image support:", props.fImageSupport);
    measuredTable.append("Max image width:", props.fMaxImage2dWidth);
    measuredTable.append("Max image height:", props.fMaxImage2dHeight);
    measuredTable.append("Available:", props.fAvailable);
    measuredTable.append("Compile available:", props.fCompilerAvailable);
    measuredTable.append("Vendor:", props.fVendor);
    measuredTable.append("Version:", props.fVersion);
    measuredTable.flush([](const std::string& str) -> void {
        log_write(LOG_DEBUG) << "  " << str << log_endl;
    });

    if (!keyword.empty() && props.fName.find(keyword) == std::string::npos)
        return false;
    if (!props.fAvailable || !props.fCompilerAvailable
        || !props.fImageSupport)
        return false;
    if (props.fMaxImage2dWidth < this->width()
        || props.fMaxImage2dHeight < this->height())
        return false;
    log_write(LOG_DEBUG) << "Using OpenCL device above" << log_endl;
    fClDeviceProperties = props;

    switch (props.fDeviceType)
    {
    case CL_DEVICE_TYPE_ACCELERATOR:
        setDriverSpecDeviceTypeInfo(CompositeDriverSpecDeviceType::kOpenCL_Accelerator);
        break;
    case CL_DEVICE_TYPE_ALL:
        setDriverSpecDeviceTypeInfo(CompositeDriverSpecDeviceType::kOpenCL_All);
        break;
    case CL_DEVICE_TYPE_CPU:
        setDriverSpecDeviceTypeInfo(CompositeDriverSpecDeviceType::kOpenCL_Cpu);
        break;
    case CL_DEVICE_TYPE_CUSTOM:
        setDriverSpecDeviceTypeInfo(CompositeDriverSpecDeviceType::kOpenCL_Custom);
        break;
    case CL_DEVICE_TYPE_DEFAULT:
        setDriverSpecDeviceTypeInfo(CompositeDriverSpecDeviceType::kOpenCL_Default);
        break;
    case CL_DEVICE_TYPE_GPU:
        setDriverSpecDeviceTypeInfo(CompositeDriverSpecDeviceType::kOpenCL_Gpu);
        break;
    default:
        setDriverSpecDeviceTypeInfo(CompositeDriverSpecDeviceType::kOpenCL_Other);
        break;
    }
    return true;
}

void GrOpenCLCompositor::prepareOpenCL()
{
    ::cl_int errCode;
    char const *sourcePtr = _clCompositeSrcOverProgram;
    fClProgram = ::clCreateProgramWithSource(fClContext,
                                             1,
                                             &sourcePtr,
                                             &_clSrcOverProgramSize,
                                             &errCode);
    this->toRetChecked(errCode, __FUNCTION__, "clCreateProgramWithSource");

    errCode = ::clBuildProgram(fClProgram,
                               1,
                               &fClDeviceId,
                               nullptr,
                               nullptr,
                               nullptr);
    if (errCode != CL_SUCCESS)
    {
        size_t strSize;
        std::string buildLog;
        RET_CHECKED(clGetProgramBuildInfo,
                    fClProgram,
                    fClDeviceId,
                    CL_PROGRAM_BUILD_LOG,
                    0,
                    nullptr,
                    &strSize);
        buildLog.resize(strSize);
        RET_CHECKED(clGetProgramBuildInfo,
                    fClProgram,
                    fClDeviceId,
                    CL_PROGRAM_BUILD_LOG,
                    strSize,
                    buildLog.data(),
                    nullptr);
        buildLog.pop_back();
        printProgramBuildLog(buildLog);

        throw RuntimeException::Builder(__FUNCTION__)
                .append("Failed to build OpenCL program")
                .make<RuntimeException>();
    }
    ::clUnloadPlatformCompiler(fClPlatformId);

    fClSrcOverKernel = ::clCreateKernel(fClProgram,
                                        BLEND_SRC_OVER_KERNEL_NAME,
                                        &errCode);
    this->toRetChecked(errCode, __FUNCTION__, "clCreateKernel");

    ::cl_image_format imageFormat;
    ::cl_image_desc imageDesc;
    imageFormat.image_channel_data_type = CL_UNORM_INT8;
    imageFormat.image_channel_order = ToOpenCLChannelOrder(colorFormat());

    imageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
    imageDesc.image_width = this->width();
    imageDesc.image_height = this->height();
    imageDesc.image_array_size = 0;
    imageDesc.image_row_pitch = 0;
    imageDesc.image_slice_pitch = 0;
    imageDesc.num_mip_levels = 0;
    imageDesc.num_samples = 0;
    imageDesc.buffer = nullptr;

    for (auto & image : fImages)
    {
        image = ::clCreateImage(fClContext,
                                CL_MEM_READ_WRITE,
                                &imageFormat,
                                &imageDesc,
                                nullptr,
                                &errCode);
        this->toRetChecked(errCode, __FUNCTION__, "clCreateImage");
    }

    computePreferredWorkSize();
}

void GrOpenCLCompositor::printProgramBuildLog(const std::string& log)
{
    std::string buffer;
    for (char ch : log)
    {
        if (ch == '\n')
        {
            log_write(LOG_ERROR) << "<OpenCL.Compiler>" << buffer << log_endl;
            buffer.clear();
        }
        else
            buffer += ch;
    }
}

void GrOpenCLCompositor::toRetChecked(::cl_int ret, const char *func, const char *cl_func)
{
    if (ret == CL_SUCCESS)
        return;
    throw RuntimeException::Builder(__FUNCTION__)
            .append("Failed to invoke ").append(cl_func)
            .append(": returned ").append(ret)
            .make<RuntimeException>();
}

void GrOpenCLCompositor::computePreferredWorkSize()
{
    size_t preferredWorkGroupSizeMultiple;
    RET_CHECKED(clGetKernelWorkGroupInfo,
                fClSrcOverKernel,
                fClDeviceId,
                CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                sizeof(size_t),
                &preferredWorkGroupSizeMultiple,
                nullptr);
    fClDeviceProperties.fPreferredWorkGroupSizeMultiple = preferredWorkGroupSizeMultiple;
}

void GrOpenCLCompositor::computeWorkSize(int32_t width, int32_t height)
{
    size_t multiple = fClDeviceProperties.fPreferredWorkGroupSizeMultiple;
    if (width * height < multiple)
    {
        fGlobalWorkSize[0] = width;
        fGlobalWorkSize[1] = height;
        fLocalWorkSize[0] = 1;
        fLocalWorkSize[1] = 1;
    }
    else if (width * height == multiple)
    {
        fGlobalWorkSize[0] = width;
        fGlobalWorkSize[1] = height;
        fLocalWorkSize[0] = width;
        fLocalWorkSize[1] = height;
    }
    else
    {
        int st = 1;
        while (st * multiple < width)
            st++;
        fGlobalWorkSize[0] = st * multiple;
        fGlobalWorkSize[1] = height;
        fLocalWorkSize[0] = multiple;
        fLocalWorkSize[1] = 1;
    }
}

void GrOpenCLCompositor::clComposite(::cl_mem target,
                                     ::cl_mem image,
                                     const SkRect &srcClip,
                                     const SkRect &dstClip)
{
    if (fInWaitingEvent != nullptr)
    {
        RET_CHECKED(clWaitForEvents, 1, &fInWaitingEvent);
        ::clReleaseEvent(fInWaitingEvent);
        fInWaitingEvent = nullptr;
    }

    ::cl_int2 srcClipVec, dstClipVec, clipDimVec;
    srcClipVec.s0 = static_cast<::cl_int>(srcClip.left());
    srcClipVec.s1 = static_cast<::cl_int>(srcClip.top());
    dstClipVec.s0 = static_cast<::cl_int>(dstClip.left());
    dstClipVec.s1 = static_cast<::cl_int>(dstClip.top());
    clipDimVec.s0 = static_cast<::cl_int>(srcClip.width());
    clipDimVec.s1 = static_cast<::cl_int>(srcClip.height());
    computeWorkSize(srcClip.width(), srcClip.height());

    RET_CHECKED(clSetKernelArg, fClSrcOverKernel, 0, sizeof(cl_mem), &image);
    RET_CHECKED(clSetKernelArg, fClSrcOverKernel, 1, sizeof(cl_mem), &fImages[fCurrentOutputImage ^ 1]);
    RET_CHECKED(clSetKernelArg, fClSrcOverKernel, 2, sizeof(cl_mem), &target);
    RET_CHECKED(clSetKernelArg, fClSrcOverKernel, 3, sizeof(cl_int2), &srcClipVec);
    RET_CHECKED(clSetKernelArg, fClSrcOverKernel, 4, sizeof(cl_int2), &dstClipVec);
    RET_CHECKED(clSetKernelArg, fClSrcOverKernel, 5, sizeof(cl_int2), &clipDimVec);

    ::clEnqueueNDRangeKernel(fClCommandQueue,
                             fClSrcOverKernel,
                             2,
                             nullptr,
                             fGlobalWorkSize,
                             fLocalWorkSize,
                             0,
                             nullptr,
                             &fInWaitingEvent);

    fCurrentOutputImage ^= 1;
}

GrTargetSurface GrOpenCLCompositor::onTargetSurface()
{
    return GrTargetSurface(fImages[fCurrentOutputImage]);
}

void GrOpenCLCompositor::onPresent()
{
    size_t origin[3] = { 0, 0, 0 };
    size_t region[3];
    region[0] = this->width();
    region[1] = this->height();
    region[2] = 1;

    cl_int ret = ::clEnqueueReadImage(fClCommandQueue,
                                      fImages[fCurrentOutputImage],
                                      CL_TRUE,
                                      origin,
                                      region,
                                      0, 0,
                                      fHostBuffer,
                                      0,
                                      nullptr,
                                      nullptr);
    this->toRetChecked(ret, __FUNCTION__, "clEnqueueReadImage");
    getPlatform()->present(fHostBuffer);
}

GrBaseRenderLayer * GrOpenCLCompositor::onCreateRenderLayer(int32_t width,
                                                            int32_t height,
                                                            int32_t left,
                                                            int32_t top,
                                                            int zindex)
{
    ::cl_image_format imageFormat;
    ::cl_image_desc imageDesc;
    imageFormat.image_channel_data_type = CL_UNORM_INT8;
    imageFormat.image_channel_order = ToOpenCLChannelOrder(colorFormat());

    imageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
    imageDesc.image_width = width;
    imageDesc.image_height = height;
    imageDesc.image_array_size = 0;
    imageDesc.image_row_pitch = 0;
    imageDesc.image_slice_pitch = 0;
    imageDesc.num_mip_levels = 0;
    imageDesc.num_samples = 0;
    imageDesc.buffer = nullptr;

    ::cl_int errCode;
    ::cl_mem image = ::clCreateImage(fClContext,
                                     CL_MEM_READ_WRITE,
                                     &imageFormat,
                                     &imageDesc,
                                     nullptr,
                                     &errCode);
    this->toRetChecked(errCode, __FUNCTION__, "clCreateImage");

    SkImageInfo imageInfo = SkImageInfo::Make(width, height,
                                              ToSkColorType(colorFormat()),
                                              SkAlphaType::kPremul_SkAlphaType);

    return new GrOpenCLRenderLayer(left, top, zindex,
                                   width, height,
                                   imageInfo,
                                   image,
                                   fClCommandQueue);
}

cl_channel_order GrOpenCLCompositor::ToOpenCLChannelOrder(GrColorFormat colorFormat)
{
    switch (colorFormat)
    {
    case GrColorFormat::kColor_BGRA_8888:
        return CL_BGRA;
    case GrColorFormat::kColor_RGBA_8888:
        return CL_RGBA;
    }
}

CIALLO_END_NS
