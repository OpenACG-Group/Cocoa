#define VK_USE_PLATFORM_XCB_KHR

#include <vector>
#include <string>
#include <memory>

#include <xcb/xcb.h>
#include <xcb/xcb_renderutil.h>
#include <vulkan/vulkan.h>

#include "Core/Journal.h"
#include "Ciallo/GrBase.h"
#include "Ciallo/DDR/GrBaseCompositor.h"
#include "Ciallo/DDR/GrGpuCompositor.h"
#include "Ciallo/DDR/GrCpuCompositor.h"
#include "Ciallo/DDR/GrBasePlatform.h"
#include "Ciallo/DDR/GrXcbPlatform.h"
#include "Ciallo/DDR/GrOpenCLCompositor.h"

CIALLO_BEGIN_NS

GrXcbPlatform::GrXcbPlatform(::xcb_connection_t *connection,
                             ::xcb_window_t window,
                             int32_t width,
                             int32_t height,
                             ::xcb_get_window_attributes_reply_t *attrs,
                             ::xcb_screen_t *screen,
                             const GrPlatformOptions& options)
    : GrBasePlatform(GrPlatformKind::kXcb, options),
      fConnection(connection),
      fWindow(window),
      fWidth(width),
      fHeight(height),
      fWindowAttributes(attrs),
      fScreen(screen),
      fXcbGContext(0),
      fBufferSize(0)
{
}

GrXcbPlatform::~GrXcbPlatform()
{
    if (fWindowAttributes)
        std::free(fWindowAttributes);

    if (fXcbGContext)
        ::xcb_free_gc(fConnection, fXcbGContext);
}

std::unique_ptr<GrBasePlatform> GrXcbPlatform::MakeFromXcbWindow(::xcb_connection_t *connection,
                                                                 ::xcb_window_t window,
                                                                 int screenp,
                                                                 const GrPlatformOptions &options)
{
    if (connection == nullptr)
        return nullptr;

    const ::xcb_setup_t *setup = ::xcb_get_setup(connection);
    ::xcb_screen_iterator_t screen_iterator = ::xcb_setup_roots_iterator(setup);
    for (int i = screenp; i > 0; i--)
        ::xcb_screen_next(&screen_iterator);
    ::xcb_screen_t *screen = screen_iterator.data;

    ::xcb_generic_error_t *xcbError = nullptr;
    auto geometry = ::xcb_get_geometry_reply(connection,
                                             ::xcb_get_geometry(connection, window),
                                             &xcbError);
    if (xcbError != nullptr)
    {
        log_write(LOG_ERROR) << "Failed to get geometry by XCB: "
                             << xcbError->error_code << log_endl;
        return nullptr;
    }

    int32_t width = geometry->width;
    int32_t height = geometry->height;
    std::free(geometry);

    auto attributes = ::xcb_get_window_attributes_reply(connection,
                                                        ::xcb_get_window_attributes(connection, window),
                                                        &xcbError);
    if (xcbError != nullptr)
    {
        log_write(LOG_ERROR) << "Failed to get window attributes by XCB: "
                             << xcbError->error_code << log_endl;
        return nullptr;
    }

    auto ret = std::make_unique<GrXcbPlatform>(connection, window, width, height,
                                               attributes, screen, options);
    ret->createXcbResources();
    return ret;
}

std::unique_ptr<::xcb_render_pictforminfo_t> GrXcbPlatform::getPictFormInfo()
{
    auto cookie = ::xcb_render_query_pict_formats(fConnection);

    ::xcb_generic_error_t *error = nullptr;
    ::xcb_render_query_pict_formats_reply_t *formatsReply =
            ::xcb_render_query_pict_formats_reply(fConnection, cookie, &error);
    if (formatsReply == nullptr)
        return nullptr;

    ::xcb_render_pictvisual_t *pictvisual = ::xcb_render_util_find_visual_format(formatsReply,
                                                                                 fScreen->root_visual);

    auto itr = ::xcb_render_query_pict_formats_formats_iterator(formatsReply);
    ::xcb_render_pictforminfo_t *result = nullptr;
    while (itr.rem)
    {
        if (itr.data->type == XCB_RENDER_PICT_TYPE_DIRECT
            && itr.data->id == pictvisual->format)
        {
            result = itr.data;
            break;
        }
        ::xcb_render_pictforminfo_next(&itr);
    }

    if (result == nullptr)
    {
        std::free(formatsReply);
        return nullptr;
    }

    auto ret = std::make_unique<::xcb_render_pictforminfo_t>();
    std::memcpy(ret.get(), result, sizeof(*ret));
    std::free(formatsReply);
    return ret;
}

void GrXcbPlatform::setPictureFormatInfo()
{
    auto pictforminfo = getPictFormInfo();
    auto direct = &pictforminfo->direct;

    if (direct->red_mask != 0xff    ||
        direct->green_mask != 0xff  ||
        direct->blue_mask != 0xff   ||
        direct->red_shift & 0x7     ||
        direct->green_shift & 0x7   ||
        direct->blue_shift & 0x7)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Unsupported color format")
                .make<RuntimeException>();
    }

    std::string format("xxxx");
#if defined(__LITTLE_ENDIAN__)
    constexpr int chmap[4] = { 0, 1, 2, 3 };
#else
    constexpr int chmap[4] = { 3, 2, 1, 0 };
#endif
    format[chmap[direct->red_shift >> 3]]   = 'R';
    format[chmap[direct->green_shift >> 3]] = 'G';
    format[chmap[direct->blue_shift >> 3]]  = 'B';
    if (format[chmap[direct->alpha_shift >> 3]] == 'x' &&
        direct->alpha_mask == 0xff)
        format[chmap[direct->alpha_shift >> 3]] = 'A';

    if (format == "RGBA" || format == "RGBx")
        fColorFormat = GrColorFormat::kColor_RGBA_8888;
    else if (format == "BGRA" || format == "BGRx")
        fColorFormat = GrColorFormat::kColor_BGRA_8888;
    else
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Unsupported color format: ")
                .append(format)
                .make<RuntimeException>();
    }
}

std::shared_ptr<GrBaseCompositor> GrXcbPlatform::onCreateCompositor()
{
    setPictureFormatInfo();

    bool failAccel = false;
    if (options().use_gpu_accel)
    {
        auto ret = createGpuCompositor();
        if (ret != nullptr)
            return ret;
        failAccel = true;
        log_write(LOG_WARNING) << "GPU rendering is unavailable" << log_endl;
    }
    else if (options().use_opencl_accel)
    {
        auto ret = createOpenCLCompositor();
        if (ret != nullptr)
            return ret;
        failAccel = true;
        log_write(LOG_WARNING) << "OpenCL compositor is unavailable" << log_endl;
    }

    if (failAccel && options().use_strict_accel)
        return nullptr;

    writableOptions().use_gpu_accel = false;
    writableOptions().use_opencl_accel = false;
    return createCpuCompositor();
}

std::shared_ptr<GrBaseCompositor> GrXcbPlatform::createGpuCompositor()
{
    std::vector<std::string> instanceExtensions = { "VK_KHR_surface", "VK_KHR_xcb_surface" };
    for (const std::string& ext : options().vulkan_required_instance_ext)
    {
        bool has = false;
        for (const std::string& req : instanceExtensions)
        {
            if (ext == req) {
                has = true;
                break;
            }
        }
        if (!has)
            instanceExtensions.push_back(ext);
    }

    ::VkXcbSurfaceCreateInfoKHR createInfo{
        .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .connection = fConnection,
        .window = fWindow
    };
    return GrGpuCompositor::Make(
            [&createInfo](::VkInstance instance) -> ::VkSurfaceKHR {
                ::VkSurfaceKHR surface;
                ::vkCreateXcbSurfaceKHR(instance, &createInfo, nullptr, &surface);
                return surface;
            },
            fWidth,
            fHeight,
            fColorFormat,
            this,
            instanceExtensions,
            options().vulkan_required_device_ext,
            options().vulkan_debug);
}

std::shared_ptr<GrBaseCompositor> GrXcbPlatform::createCpuCompositor()
{
    return GrCpuCompositor::MakeDirectCpu(fWidth,
                                          fHeight,
                                          fColorFormat,
                                          this);
}

std::shared_ptr<GrBaseCompositor> GrXcbPlatform::createOpenCLCompositor()
{
    return GrOpenCLCompositor::MakeOpenCL(fWidth,
                                          fHeight,
                                          fColorFormat,
                                          this,
                                          options().opencl_platform_keyword,
                                          options().opencl_device_keyword);
}

void GrXcbPlatform::createXcbResources()
{
    fBufferSize = fWidth * fHeight * sizeof(uint32_t);

    fXcbGContext = ::xcb_generate_id(fConnection);
    ::xcb_create_gc(fConnection,
                    fXcbGContext,
                    fWindow,
                    0, nullptr);
}

void GrXcbPlatform::onPresent(const uint8_t *pBuffer)
{
    xcb_put_image(fConnection,
                  XCB_IMAGE_FORMAT_Z_PIXMAP,
                  fWindow,
                  fXcbGContext,
                  fWidth,
                  fHeight,
                  0, 0,
                  0,
                  24,
                  fBufferSize,
                  pBuffer);
    xcb_flush(fConnection);
}

CIALLO_END_NS
