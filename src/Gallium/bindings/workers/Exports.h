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

#ifndef COCOA_GALLIUM_BINDINGS_WORKERS_EXPORTS_H
#define COCOA_GALLIUM_BINDINGS_WORKERS_EXPORTS_H

#include "include/v8.h"
#include "uv.h"

#include "Gallium/Gallium.h"
#include "Gallium/bindings/workers/Types.h"
#include "Gallium/bindings/Base.h"
#include "Gallium/bindings/EventEmitter.h"
GALLIUM_BINDINGS_WORKERS_NS_BEGIN

class MessagePort;

//! TSDecl: class Worker
class WorkerWrap : public ExportableObjectBase
{
public:
    //! TSDecl: function MakeFromURL(url: string): Worker
    static v8::Local<v8::Value> MakeFromURL(const std::string& url);

    explicit WorkerWrap(std::shared_ptr<MessagePort> port);
    ~WorkerWrap();

    //! TSDecl: readonly port: MessagePort
    v8::Local<v8::Value> getPort() {
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        return message_port_.Get(isolate);
    }

private:
    v8::Global<v8::Object> message_port_;
};

//! TSDecl: class MessagePort
class MessagePortWrap : public ExportableObjectBase,
                        public EventEmitterBase
{
public:
    //! TSDecl: function MakeConnectedPair(): [MessagePort, MessagePort]
    static v8::Local<v8::Value> MakeConnectedPair();

    explicit MessagePortWrap(std::shared_ptr<MessagePort> port);
    ~MessagePortWrap() override = default;

    g_nodiscard g_inline std::shared_ptr<MessagePort> GetPort() const {
        return port_;
    }

    //! TSDecl: function close(): void
    void close();

    //! TSDecl: function postMessage(message: any, transfers?: Array<any>): void
    void postMessage(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
    void CheckClosedPort();
    v8::Local<v8::Object> OnGetObjectSelf(v8::Isolate *isolate) override;

    std::shared_ptr<MessagePort> port_;
};

GALLIUM_BINDINGS_WORKERS_NS_END
#endif //COCOA_GALLIUM_BINDINGS_WORKERS_EXPORTS_H
