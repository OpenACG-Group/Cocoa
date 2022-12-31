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

#include "Core/Exception.h"
#include "Core/EventLoop.h"
#include "Gallium/binder/ThrowExcept.h"
#include "Gallium/binder/Convert.h"
#include "Gallium/binder/Class.h"
#include "Gallium/binder/CallV8.h"
#include "Gallium/bindings/utau/Exports.h"
GALLIUM_BINDINGS_UTAU_NS_BEGIN

namespace {

void check_context_initialized()
{
    if (!AudioSinkContext::HasInstance())
        g_throw(Error, "AudioSinkContext has not been initialized yet");
}

} // namespace anonymous

void AudioSinkContext::Initialize()
{
    if (AudioSinkContext::HasInstance())
        g_throw(Error, "AudioSinkContext has already been initialized");

    // TODO(sora): Support other audio backends (like pulseaudio)
    auto sink = utau::AudioSink::MakePipeWire(EventLoop::Ref().handle());
    if (!sink)
        g_throw(Error, "Failed to connect to system audio backend (PipeWire)");

    AudioSinkContext::New();
    AudioSinkContext *context = AudioSinkContext::Instance();
    context->audio_sink_ = std::move(sink);
}

void AudioSinkContext::Dispose(bool call_from_listener)
{
    check_context_initialized();

    AudioSinkContext *context = AudioSinkContext::Instance();
    context->audio_sink_->Dispose(call_from_listener);

    AudioSinkContext::Delete();
}

int32_t AudioSinkContext::Enqueue(v8::Local<v8::Value> buffer)
{
    check_context_initialized();

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    AudioBufferWrap *wrap = binder::Class<AudioBufferWrap>::unwrap_object(isolate, buffer);
    if (!wrap)
        g_throw(TypeError, "Argument `buffer` must be an instance of `AudioBuffer`");

    return AudioSinkContext::Ref().audio_sink_->EnqueueBuffer(wrap->GetBuffer());
}

AudioSinkContext::JSBufferEventListener::JSBufferEventListener(v8::Isolate *i)
    : listener_id(0), isolate(i)
{
    static int32_t listener_counter = 0;
    listener_id = ++listener_counter;
}

void AudioSinkContext::JSBufferEventListener::InvokeJS(v8::Global<v8::Function>& func, int32_t id) const
{
    if (func.IsEmpty())
        return;

    v8::Local<v8::Function> func_local = func.Get(isolate);
    binder::Invoke(isolate, func_local,
                   isolate->GetCurrentContext()->Global(), id);
}

int32_t AudioSinkContext::AddBufferEventListener(v8::Local<v8::Value> listener)
{
    check_context_initialized();

    AudioSinkContext *self = AudioSinkContext::Instance();

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    if (!listener->IsObject())
        g_throw(Error, "Argument `listener` must be an object");

    auto listener_obj = v8::Local<v8::Object>::Cast(listener);

    self->js_buffer_listeners_.emplace_back(isolate);
    JSBufferEventListener& js_listener = self->js_buffer_listeners_.back();

    ScopeExitAutoInvoker back_listener_popper([self] {
        self->js_buffer_listeners_.pop_back();
    });

    int key_idx = 0;
    bool has_callback = false;
    for (const auto& key : {"playing", "cancelled", "consumed"})
    {
        if (!listener_obj->HasOwnProperty(
                context, binder::to_v8(isolate, key)).FromMaybe(false))
        {
            key_idx++;
            continue;
        }

        v8::Local<v8::Value> value = listener_obj->Get(
                context, binder::to_v8(isolate, key)).ToLocalChecked();
        if (!value->IsFunction())
            g_throw(TypeError, "Buffer event listeners must be functions");

        js_listener.callbacks[key_idx].Reset(
                isolate, v8::Local<v8::Function>::Cast(value));

        has_callback = true;
        key_idx++;
    }

    if (!has_callback)
        g_throw(Error, "No valid listener functions");

    self->audio_sink_->AppendBufferEventListener(&js_listener);
    back_listener_popper.cancel();

    return js_listener.listener_id;
}

void AudioSinkContext::RemoveBufferEventListener(int32_t listenerId)
{
    check_context_initialized();

    AudioSinkContext *self = AudioSinkContext::Instance();

    auto itr = std::find_if(self->js_buffer_listeners_.begin(),
                            self->js_buffer_listeners_.end(),
                            [listenerId](const JSBufferEventListener& entry) {
        return (listenerId == entry.listener_id);
    });

    if (itr == self->js_buffer_listeners_.end())
        g_throw(Error, "`listenerId` does not refer to a active listener");

    JSBufferEventListener *plistener = &(*itr);
    self->audio_sink_->RemoveBufferEventListener(plistener);

    self->js_buffer_listeners_.erase(itr);
}

GALLIUM_BINDINGS_UTAU_NS_END
