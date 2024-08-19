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

#include "Gallium/bindings/multimedia/Packet.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

Packet::~Packet()
{
    if (packet_)
        av_packet_free(&packet_);
}

ffi::RetLocal<v8::Value> Packet::clone()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<Packet>(isolate, av_packet_clone(packet_));
}

ffi::Ret<void> Packet::dispose()
{
    av_packet_free(&packet_);
    NotifyDisposeState(DisposeState::kDisposed);
    return {};
}

ffi::Ret<void> Packet::disposeReusable()
{
    av_packet_unref(packet_);
    NotifyDisposeState(DisposeState::kDisposed);
    return {};
}

ffi::RetLocal<v8::Value> Packet::getPts()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (packet_->pts == AV_NOPTS_VALUE)
        return v8::Null(isolate);
    return ffi::Cast<int64_t>::ToChecked(isolate, packet_->pts);
}

ffi::RetLocal<v8::Value> Packet::getDts()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (packet_->dts == AV_NOPTS_VALUE)
        return v8::Null(isolate);
    return ffi::Cast<int64_t>::ToChecked(isolate, packet_->dts);
}

ffi::RetLocal<v8::Value> Packet::getDuration()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (packet_->duration == 0)
        return v8::Null(isolate);
    return ffi::Cast<int64_t>::ToChecked(isolate, packet_->duration);
}

ffi::Ret<int32_t> Packet::getStreamIndex()
{
    return packet_->stream_index;
}

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
