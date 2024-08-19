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

#include <cstring>
#include <algorithm>
#include <unordered_map>

#include "Gallium/bindings/multimedia/AChannelLayout.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

namespace {

const std::unordered_map<std::string, AVChannelLayout> g_named_layouts{
    { "mono", AV_CHANNEL_LAYOUT_MONO },
    { "stereo", AV_CHANNEL_LAYOUT_STEREO },
    { "2.1", AV_CHANNEL_LAYOUT_2POINT1 },
    { "2-1", AV_CHANNEL_LAYOUT_2_1 },
    { "surround", AV_CHANNEL_LAYOUT_SURROUND },
    { "3.1", AV_CHANNEL_LAYOUT_3POINT1 },
    { "4.0", AV_CHANNEL_LAYOUT_4POINT0 },
    { "4.1", AV_CHANNEL_LAYOUT_4POINT1 },
    { "2-2", AV_CHANNEL_LAYOUT_2_2 },
    { "quad", AV_CHANNEL_LAYOUT_QUAD },
    { "5.0", AV_CHANNEL_LAYOUT_5POINT0 },
    { "5.1", AV_CHANNEL_LAYOUT_5POINT1 },
    { "5.0-back", AV_CHANNEL_LAYOUT_5POINT0_BACK },
    { "5.1-back", AV_CHANNEL_LAYOUT_5POINT1_BACK },
    { "6.0", AV_CHANNEL_LAYOUT_6POINT0 },
    { "6.0-front", AV_CHANNEL_LAYOUT_6POINT0_FRONT },
    { "hexagonal", AV_CHANNEL_LAYOUT_HEXAGONAL },
    { "6.1", AV_CHANNEL_LAYOUT_6POINT1 },
    { "6.1-back", AV_CHANNEL_LAYOUT_6POINT1_BACK },
    { "6.1-front", AV_CHANNEL_LAYOUT_6POINT1_FRONT },
    { "7.0", AV_CHANNEL_LAYOUT_7POINT0 },
    { "7.0-front", AV_CHANNEL_LAYOUT_7POINT0_FRONT },
    { "7.1", AV_CHANNEL_LAYOUT_7POINT1 },
    { "7.1-wide", AV_CHANNEL_LAYOUT_7POINT1_WIDE },
    { "7.1-wide-back", AV_CHANNEL_LAYOUT_7POINT1_WIDE_BACK },
    { "7.1-top-back", AV_CHANNEL_LAYOUT_7POINT1_TOP_BACK },
    { "octagonal", AV_CHANNEL_LAYOUT_OCTAGONAL },
    { "cube", AV_CHANNEL_LAYOUT_CUBE },
    { "hexadecagonal", AV_CHANNEL_LAYOUT_HEXADECAGONAL },
    { "stereo-downmix", AV_CHANNEL_LAYOUT_STEREO_DOWNMIX },
    { "22.2", AV_CHANNEL_LAYOUT_22POINT2 },
};

struct NamedChannel
{
    std::string name;
    AVChannel ch;
};

const std::vector<NamedChannel> g_channel_names{
    // Note that channel names are case-insensitive.
    // The following definition must be in the same order to definitions in `AVChannel`.
    { "FrontL", AV_CHAN_FRONT_LEFT },
    { "FrontR", AV_CHAN_FRONT_RIGHT },
    { "FrontC", AV_CHAN_FRONT_CENTER },
    { "LowFreq", AV_CHAN_LOW_FREQUENCY },
    { "BackL", AV_CHAN_BACK_LEFT },
    { "BackR", AV_CHAN_BACK_RIGHT },
    { "FrontLc", AV_CHAN_FRONT_LEFT_OF_CENTER },
    { "FrontRc", AV_CHAN_FRONT_RIGHT_OF_CENTER },
    { "BackC", AV_CHAN_BACK_CENTER },
    { "SideL", AV_CHAN_SIDE_LEFT },
    { "SideR", AV_CHAN_SIDE_RIGHT },
    { "TopC", AV_CHAN_TOP_CENTER },
    { "TopFrontL", AV_CHAN_TOP_FRONT_LEFT },
    { "TopFrontC", AV_CHAN_TOP_FRONT_CENTER },
    { "TopFrontR", AV_CHAN_TOP_FRONT_RIGHT },
    { "TopBackL", AV_CHAN_TOP_BACK_LEFT },
    { "TopBackC", AV_CHAN_TOP_BACK_CENTER },
    { "TopBackR", AV_CHAN_TOP_BACK_RIGHT },
    { "L", AV_CHAN_STEREO_LEFT },
    { "R", AV_CHAN_STEREO_RIGHT },
    { "WideL", AV_CHAN_WIDE_LEFT },
    { "WideR", AV_CHAN_WIDE_RIGHT },
    { "SurDirectL", AV_CHAN_SURROUND_DIRECT_LEFT },
    { "SurDirectR", AV_CHAN_SURROUND_DIRECT_RIGHT },
    { "LowFreq2", AV_CHAN_LOW_FREQUENCY_2 },
    { "TopSideL", AV_CHAN_TOP_SIDE_LEFT },
    { "TopSideR", AV_CHAN_TOP_SIDE_RIGHT },
    { "BottomFrontC", AV_CHAN_BOTTOM_FRONT_CENTER },
    { "BottomFrontL", AV_CHAN_BOTTOM_FRONT_LEFT },
    { "BottomFrontR", AV_CHAN_BOTTOM_FRONT_RIGHT }
};

const NamedChannel *find_channel_by_id(AVChannel id)
{
    auto itr = std::find_if(g_channel_names.begin(), g_channel_names.end(),
                            [id](const NamedChannel& ch) { return (id == ch.ch); });
    return itr == g_channel_names.end() ? nullptr : &*itr;
}

const NamedChannel *find_channel_by_name(const std::string& name)
{
    auto itr = std::find_if(g_channel_names.begin(), g_channel_names.end(),
                            [name](const NamedChannel& ch) {
        return strcasecmp(ch.name.c_str(), name.c_str()) == 0;
    });
    return itr == g_channel_names.end() ? nullptr : &*itr;
}

v8::Local<v8::Array> channel_mask_to_names(v8::Isolate *isolate, uint64_t mask)
{
    std::vector<v8::Local<v8::Value>> names;
    for (const NamedChannel& ch : g_channel_names)
    {
        if (mask & (1ULL << ch.ch))
        {
            names.emplace_back(v8::String::NewFromUtf8(
                    isolate, ch.name.c_str()).ToLocalChecked());
        }
    }
    return v8::Array::New(isolate, names.data(), names.size());
}

} // namespace anonymous

ffi::RetLocal<v8::Value> AChannelLayout::Predefined(const std::string& name)
{
    auto itr = g_named_layouts.find(name);
    if (itr == g_named_layouts.end())
        return ffi::Fail(ffi::kErr, "required channel layout name was not found");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<AChannelLayout>(
            isolate, const_cast<AVChannelLayout&>(itr->second), kNotFreeOriginal);
}

AChannelLayout::AChannelLayout(AVChannelLayout& ch, int flags)
    : layout_{}
{
    av_channel_layout_copy(&layout_, &ch);
    if (!(flags & kNotFreeOriginal))
        av_channel_layout_uninit(&ch);
}

AChannelLayout::~AChannelLayout()
{
    av_channel_layout_uninit(&layout_);
}

void AChannelLayout::CopyLayoutTo(AVChannelLayout &dst)
{
    av_channel_layout_copy(&dst, &layout_);
}

ffi::RetLocal<v8::Value> AChannelLayout::getOrderedChannels()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    // Returns an empty array for unspecified order
    if (layout_.order == AV_CHANNEL_ORDER_UNSPEC)
        return v8::Array::New(isolate);

    if (layout_.order == AV_CHANNEL_ORDER_NATIVE)
        return channel_mask_to_names(isolate, layout_.u.mask);

    if (layout_.order == AV_CHANNEL_ORDER_CUSTOM)
    {
        std::vector<v8::Local<v8::Value>> names;
        for (int i = 0; i < layout_.nb_channels; i++)
        {
            AVChannel id = layout_.u.map[i].id;
            const NamedChannel* info = find_channel_by_id(id);
            if (!info)
                return ffi::Fail(ffi::kErr, fmt::format("channel id #{} not found", static_cast<int>(id)));
            names.emplace_back(v8::String::NewFromUtf8(
                    isolate, info->name.c_str()).ToLocalChecked());
        }
        return v8::Array::New(isolate, names.data(), names.size());
    }

    return ffi::Fail(ffi::kErr, "unsupported channel layout");
}

ffi::RetLocal<v8::Value> AChannelLayout::clone()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<AChannelLayout>(isolate, layout_, kNotFreeOriginal);
}

ffi::Ret<bool> AChannelLayout::equalTo(const ffi::Class<AChannelLayout>& other)
{
    int res = av_channel_layout_compare(&layout_, &other->layout_);
    if (res < 0)
        return ffi::Fail(ffi::kErr, fmt::format("failed to compare: {}", av_err2str(res)));
    return (res == 0);
}

ffi::RetLocal<v8::Value> AChannelLayout::intersect(const std::vector<std::string>& ch)
{
    uint64_t mask = 0;
    for (const std::string& name : ch)
    {
        const NamedChannel *chinfo = find_channel_by_name(name);
        if (!chinfo)
            return ffi::Fail(ffi::kErr, fmt::format("invalid channel name `{}`", name));
        mask |= 1ULL << chinfo->ch;
    }
    mask = av_channel_layout_subset(&layout_, mask);
    return channel_mask_to_names(v8::Isolate::GetCurrent(), mask);
}

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
