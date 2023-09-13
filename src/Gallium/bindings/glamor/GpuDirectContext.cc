/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

#include "include/core/SkColorSpace.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "fmt/format.h"

#include "Glamor/Glamor.h"
#include "Glamor/HWComposeOffscreen.h"

#include "Gallium/binder/ThrowExcept.h"
#include "Gallium/binder/Class.h"
#include "Gallium/bindings/glamor/CkSurfaceWrap.h"
#include "Gallium/bindings/glamor/GpuDirectContext.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

namespace {

std::unique_ptr<GrBackendSemaphore[]>
fill_flush_info_signal_semaphores(v8::Local<v8::Context> ctx,
                                  v8::Local<v8::Array> array,
                                  GrFlushInfo& target)
{
    v8::Isolate *isolate = ctx->GetIsolate();

    int32_t num_semaphores = static_cast<int32_t>(array->Length());
    if (num_semaphores == 0)
        return nullptr;

    auto store = std::make_unique<GrBackendSemaphore[]>(num_semaphores);
    for (int32_t i = 0; i < num_semaphores; i++)
    {
        v8::Local<v8::Value> element;
        if (!array->Get(ctx, i).ToLocal(&element))
            return nullptr;
        GpuBinarySemaphore *wrap =
                binder::UnwrapObject<GpuBinarySemaphore>(isolate, element);
        if (!wrap)
            return nullptr;
        store[i].initVulkan(wrap->GetVkSemaphore());
    }

    target.fNumSemaphores = num_semaphores;
    target.fSignalSemaphores = store.get();
    return store;
}

struct GpuFlushCallbackContext
{
    GpuFlushCallbackContext(v8::Isolate *isolate_, v8::Local<v8::Function> func_,
                            v8::Local<v8::Object> js_receiver_)
        : isolate(isolate_), func(isolate_, func_), js_receiver(isolate_, js_receiver_) {}
    v8::Isolate *isolate;
    v8::Global<v8::Function> func;
    v8::Global<v8::Object> js_receiver;

    static void OnFinishedProc(GrGpuFinishedContext self_)
    {
        auto *self = static_cast<GpuFlushCallbackContext*>(self_);
        v8::HandleScope handle_scope(self->isolate);
        v8::Local<v8::Function> func = self->func.Get(self->isolate);
        v8::Local<v8::Context> ctx = self->isolate->GetCurrentContext();
        (void) func->Call(ctx, self->js_receiver.Get(self->isolate), 0, nullptr);
        delete self;
    }

    static void OnSubmittedProc(GrGpuSubmittedContext self_, bool success)
    {
        auto *self = static_cast<GpuFlushCallbackContext*>(self_);
        v8::HandleScope handle_scope(self->isolate);
        v8::Local<v8::Function> func = self->func.Get(self->isolate);
        v8::Local<v8::Context> ctx = self->isolate->GetCurrentContext();
        v8::Local<v8::Value> args[] = { v8::Boolean::New(self->isolate, success) };
        (void) func->Call(ctx, self->js_receiver.Get(self->isolate), 1, args);
        delete self;
    }
};

} // namespace anonymous

std::tuple<GrFlushInfo, std::unique_ptr<GrBackendSemaphore[]>>
GpuDirectContext::ExtractGrFlushInfo(v8::Isolate *isolate, v8::Local<v8::Object> object,
                                     v8::Local<v8::Object> direct_context)
{
#define PROP_STR(s) v8::String::NewFromUtf8Literal(isolate, s)

    GrFlushInfo flush_info;
    v8::HandleScope handle_scope(isolate);

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    v8::Local<v8::Value> prop;

    // As `GrFlushInfo` itself does not contain the ownership of
    // array `GrFlushInfo::fSignalSemaphores`, it is our responsibility
    // to delete the memory when `GrFlushInfo` will not be used anymore.
    std::unique_ptr<GrBackendSemaphore[]> backend_semaphores_store;
    prop = object->Get(ctx, PROP_STR("signalSemaphores")).ToLocalChecked();
    if (!prop->IsNullOrUndefined())
    {
        if (!prop->IsArray())
            g_throw(TypeError, "GpuFlushInfo: Property `signalSemaphores` is not an array");
        backend_semaphores_store = fill_flush_info_signal_semaphores(
                ctx, prop.As<v8::Array>(), flush_info);
        if (!backend_semaphores_store)
            g_throw(TypeError, "GpuFlushInfo: Property `signalSemaphores` is invalid");
    }

    prop = object->Get(ctx, PROP_STR("onFinished")).ToLocalChecked();
    if (!prop->IsNullOrUndefined())
    {
        if (!prop->IsFunction())
            g_throw(TypeError, "GrFlushInfo: Property `onFinished` is not a function");
        flush_info.fFinishedContext = new GpuFlushCallbackContext(
                isolate, prop.As<v8::Function>(), direct_context);
        flush_info.fFinishedProc = GpuFlushCallbackContext::OnFinishedProc;
    }

    prop = object->Get(ctx, PROP_STR("onSubmitted")).ToLocalChecked();
    if (!prop->IsNullOrUndefined())
    {
        if (!prop->IsFunction())
            g_throw(TypeError, "GrFlushInfo: Property `onSubmitted` is not a function");
        flush_info.fSubmittedContext = new GpuFlushCallbackContext(
                isolate, prop.As<v8::Function>(), direct_context);
        flush_info.fSubmittedProc = GpuFlushCallbackContext::OnSubmittedProc;
    }

#undef PROP_STR

    return std::make_tuple(flush_info, std::move(backend_semaphores_store));
}

GpuDirectContext *GpuBinarySemaphore::GetGpuContext()
{
    CheckDisposedOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::UnwrapObjectFast<GpuDirectContext>(isolate, context_object_.Get(isolate));
}

void GpuBinarySemaphore::dispose()
{
    CheckDisposedOrThrow();
    VkDevice device = GetGpuContext()
            ->GetHWComposeOffscreen()->GetDevice()->GetVkDevice();
    vkDestroySemaphore(device, semaphore_, nullptr);
    semaphore_ = VK_NULL_HANDLE;
    context_object_.Reset();
}

void GpuBinarySemaphore::detach()
{
    semaphore_ = VK_NULL_HANDLE;
    context_object_.Reset();
}

bool GpuBinarySemaphore::isDetachedOrDisposed()
{
    return (semaphore_ == VK_NULL_HANDLE);
}

void GpuBinarySemaphore::CheckDisposedOrThrow()
{
    if (semaphore_ == VK_NULL_HANDLE)
        g_throw(Error, "GpuBinarySemaphore has been disposed or detached");
}

GpuExportedFd::GpuExportedFd(v8::Local<v8::Object> gpu_context, int32_t fd)
    : gpu_context_(v8::Isolate::GetCurrent(), gpu_context)
    , fd_(fd)
{
}

GpuExportedFd::~GpuExportedFd()
{
    if (fd_ >= 0)
        ::close(fd_);
}

void GpuExportedFd::close()
{
    if (fd_ < 0)
        g_throw(Error, "Exported file descriptor has been closed");
    ::close(fd_);
    fd_ = -1;
    gpu_context_.Reset();
}

bool GpuExportedFd::isImportedOrClosed() const
{
    return (fd_ < 0);
}

int32_t GpuExportedFd::CheckAndTakeDescriptor()
{
    if (fd_ < 0)
        g_throw(Error, "Exported file descriptor has been closed");
    int32_t fd = fd_;
    fd_ = -1;
    gpu_context_.Reset();
    return fd;
}

v8::Local<v8::Value> GpuDirectContext::Make()
{
    std::shared_ptr<gl::HWComposeContext> hw_context =
            gl::GlobalScope::Ref().GetHWComposeContext();
    if (!hw_context)
        g_throw(Error, "Failed to create an hardware compose context");

    std::unique_ptr<gl::HWComposeOffscreen> offscreen =
            gl::HWComposeOffscreen::Make(hw_context);
    if (!offscreen)
        g_throw(Error, "Failed to create offscreen rendering GPU context");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::NewObject<GpuDirectContext>(isolate, std::move(offscreen));
}

GpuDirectContext::GpuDirectContext(std::unique_ptr<gl::HWComposeOffscreen> context)
    : context_(std::move(context))
{
}

void GpuDirectContext::CheckDisposedOrThrow()
{
    if (!context_)
        g_throw(Error, "GPU context has been disposed");
}

void GpuDirectContext::dispose()
{
    CheckDisposedOrThrow();
    context_.reset();
}

bool GpuDirectContext::isDisposed()
{
    return !static_cast<bool>(context_);
}

v8::Local<v8::Value> GpuDirectContext::makeSurface(v8::Local<v8::Value> image_info,
                                                   bool budgeted,
                                                   int32_t aa_samples_per_pixel)
{
    CheckDisposedOrThrow();

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkImageInfo sk_image_info = ExtractCkImageInfo(isolate, image_info);

    if (aa_samples_per_pixel < 0)
        g_throw(RangeError, "Invalid antialias samples per pixel (argument `aaSamplesPerPixel`)");

    // GPU surface does not support unpremul alpha.
    // See `Device::CheckAlphaTypeAndGetFlags()` in `//third_party/skia/src/gpu/ganesh/Device.cpp`
    // for more details, including the supported alpha formats.
    SkAlphaType alpha_type = sk_image_info.alphaType();
    if (alpha_type == SkAlphaType::kUnknown_SkAlphaType ||
        alpha_type == SkAlphaType::kUnpremul_SkAlphaType)
    {
        g_throw(Error, "Provided alpha type is not supported by GPU surface");
    }

    GrDirectContext *direct_ctx = context_->GetSkiaGpuContext();
    int32_t max_aa_samples = direct_ctx->maxSurfaceSampleCountForColorType(
            sk_image_info.colorType());
    // Skia just rounds the sample count to the maximum supported value
    // if it is larger than the maximum value. But an exception still should
    // be thrown to inform users that they are using an invalid sample count.
    if (aa_samples_per_pixel > max_aa_samples)
    {
        g_throw(Error, fmt::format("Invalid antialias samples per pixel (maximum is {})",
                                   max_aa_samples));
    }

    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(
        direct_ctx,
        budgeted ? skgpu::Budgeted::kYes : skgpu::Budgeted::kNo,
        sk_image_info,
        aa_samples_per_pixel,
        GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
        nullptr,
        false
    );

    if (!surface)
        g_throw(Error, "Failed to create GPU surface");

    return binder::NewObject<CkSurface>(isolate, std::move(surface),
                                        GetObjectWeakReference().Get(isolate));
}

v8::Local<v8::Value> GpuDirectContext::makeBinarySemaphore()
{
    CheckDisposedOrThrow();
    VkDevice device = context_->GetDevice()->GetVkDevice();
    VkSemaphoreCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkSemaphore semaphore;
    VkResult result = vkCreateSemaphore(device, &create_info, nullptr, &semaphore);
    if (result != VK_SUCCESS)
        g_throw(Error, "Failed to create a binary semaphore");
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::NewObject<GpuBinarySemaphore>(
            isolate, GetObjectWeakReference().Get(isolate), semaphore);
}

v8::Local<v8::Value> GpuDirectContext::exportSemaphoreFd(v8::Local<v8::Value> semaphore)
{
    CheckDisposedOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *wrap = binder::UnwrapObject<GpuBinarySemaphore>(isolate, semaphore);
    if (!wrap || wrap->isDetachedOrDisposed())
        g_throw(TypeError, "Argument `semaphore` must be a valid GpuBinarySemaphore");
    int32_t fd = context_->ExportSemaphoreFd(wrap->GetVkSemaphore());
    if (fd < 0)
        g_throw(Error, "Failed to export semaphore as a file descriptor");
    return binder::NewObject<GpuExportedFd>(
            isolate, GetObjectWeakReference().Get(isolate), fd);
}

v8::Local<v8::Value>
GpuDirectContext::importSemaphoreFd(v8::Local<v8::Value> fd)
{
    CheckDisposedOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    GpuExportedFd *handle = binder::UnwrapObject<GpuExportedFd>(isolate, fd);
    if (!handle || handle->isImportedOrClosed())
        g_throw(TypeError, "Argument `fd` must be a valid GpuExportedFd");
    int32_t fd_value = handle->CheckAndTakeDescriptor();
    VkSemaphore imported = context_->ImportSemaphoreFromFd(fd_value);
    if (imported == VK_NULL_HANDLE)
    {
        // Fd only needs to be closed when an error is occurred,
        // which means the semaphore is not imported successfully.
        // If the semaphore is imported correctly, Vulkan will take over
        // the ownership of it, and we don't need to close it manually.
        close(fd_value);
        g_throw(Error, "Failed to import a semaphore from file descriptor");
    }
    return binder::NewObject<GpuBinarySemaphore>(
            isolate, GetObjectWeakReference().Get(isolate), imported);
}

int32_t GpuDirectContext::flush(v8::Local<v8::Value> info)
{
    CheckDisposedOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!info->IsObject())
        g_throw(TypeError, "Argument `info` must be an object");
    auto [flush_info, owned_semaphores] = ExtractGrFlushInfo(
            isolate, info.As<v8::Object>(), GetObjectWeakReference().Get(isolate));
    return static_cast<int32_t>(context_->GetSkiaGpuContext()->flush(flush_info));
}

bool GpuDirectContext::submit(bool wait_for_submit)
{
    CheckDisposedOrThrow();
    return context_->GetSkiaGpuContext()->submit(wait_for_submit);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
