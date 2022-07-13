#include "Glamor/Monitor.h"
GLAMOR_NAMESPACE_BEGIN

namespace {
uint32_t g_monitor_unique_id_counter = 0;
} // namespace anonymous

GLAMOR_TRAMPOLINE_IMPL(Monitor, RequestProperties)
{
    info.GetThis()->As<Monitor>()->RequestProperties();
    info.SetReturnStatus(RenderClientCallInfo::Status::kOpSuccess);
}

Monitor::Monitor(const Weak<Display>& display)
    : RenderClientObject(RealType::kMonitor)
    , display_(display)
    , unique_id_(0)
    , logical_x_(0)
    , logical_y_(0)
    , physical_width_(0)
    , physical_height_(0)
    , subpixel_(MonitorSubpixel::kUnknown)
    , manufacture_name_(GLAMOR_MONITOR_DEFAULT_MANUFACTURE)
    , model_name_(GLAMOR_MONITOR_DEFAULT_MODEL)
    , transform_(MonitorTransform::kNormal)
    , mode_flags_({MonitorMode::kPreferred})
    , mode_width_(0)
    , mode_height_(0)
    , refresh_rate_mhz_(0)
    , scale_factor_(1)
    , connector_name_(GLAMOR_MONITOR_DEFAULT_CONNECTOR)
    , description_(GLAMOR_MONITOR_DEFAULT_DESCRIPTION)
{
    unique_id_ = ++g_monitor_unique_id_counter;

    SetMethodTrampoline(CROP_MONITOR_REQUEST_PROPERTIES, Monitor_RequestProperties_Trampoline);
}

void Monitor::RequestProperties()
{
    NotifyPropertiesChanged();
}

void Monitor::NotifyPropertiesChanged()
{
    auto prop = std::make_shared<PropertySet>();
    prop->logical_position.set(logical_x_, logical_y_);
    prop->physical_metrics.set(physical_width_, physical_height_);
    prop->subpixel = subpixel_;
    prop->manufacture_name = manufacture_name_;
    prop->model_name = model_name_;
    prop->transform = transform_;
    prop->mode_flags = mode_flags_;
    prop->mode_size.set(mode_width_, mode_height_);
    prop->refresh_rate_mhz = refresh_rate_mhz_;
    prop->scale_factor = scale_factor_;
    prop->connector_name = connector_name_;
    prop->description = description_;

    RenderClientEmitterInfo info;
    info.EmplaceBack<Shared<PropertySet>>(std::move(prop));

    Emit(CRSI_MONITOR_PROPERTIES_CHANGED, std::move(info));
}

GLAMOR_NAMESPACE_END
