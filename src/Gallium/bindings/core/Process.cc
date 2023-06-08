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

#include "Core/Journal.h"
#include "Core/EventLoop.h"
#include "Gallium/bindings/core/Exports.h"

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.bindings.core)

GALLIUM_BINDINGS_NS_BEGIN

void Print(const std::string& str)
{
    if (str.empty())
        return;
    std::fwrite(str.c_str(), str.size(), 1, stdout);
}

void Dump(const std::string& what)
{
    // TODO: implement this.
}

ProcessWrap::ProcessWrap(uv_process_t *handle, v8::Local<v8::Object> streams[3])
    : PreventGCObject(v8::Isolate::GetCurrent())
    , isolate_(v8::Isolate::GetCurrent())
    , process_handle_(handle)
    , has_stopped_(false)
{
    v8::Local<v8::Context> ctx = isolate_->GetCurrentContext();
    uv_handle_set_data(reinterpret_cast<uv_handle_t*>(process_handle_), this);
    v8::Local<v8::Promise::Resolver> promise = v8::Promise::Resolver::New(ctx).ToLocalChecked();
    on_exit_promise_.Reset(isolate_, promise);

    for (int i = 0; i < 3; i++)
    {
        if (!streams[i].IsEmpty())
            redirected_streams_[i].Reset(isolate_, streams[i]);
    }
}

ProcessWrap::~ProcessWrap()
{
    for (v8::Global<v8::Object>& p : redirected_streams_)
        p.Reset();
    on_exit_promise_.Reset();
    detachHandle();
}

void ProcessWrap::detachHandle()
{
    if (process_handle_)
    {
        uv_unref(reinterpret_cast<uv_handle_t *>(process_handle_));
        delete process_handle_;
        process_handle_ = nullptr;
    }
}

void ProcessWrap::kill(int32_t signum) const
{
    if (!has_stopped_)
    {
        uv_process_kill(process_handle_, signum);
    }
}

v8::Local<v8::Value> ProcessWrap::promiseOnExit()
{
    return on_exit_promise_.Get(v8::Isolate::GetCurrent());
}

int32_t ProcessWrap::getPid() const
{
    if (!has_stopped_ && process_handle_)
        return uv_process_get_pid(process_handle_);
    return -1;
}

v8::Local<v8::Object> ProcessWrap::getPipeStream(uint32_t stream)
{
    if (stream >= 3)
        g_throw(RangeError, "Bad stream index (stdin 0, stdout 1, stderr 2)");
    if (redirected_streams_[stream].IsEmpty())
        g_throw(Error, fmt::format("Stream {} is not redirected when spawn child process", stream));
    return redirected_streams_[stream].Get(isolate_);
}

namespace {

v8::MaybeLocal<v8::Value> check_object_field(v8::Local<v8::Object> obj, const char *field,
                                             const char *type, bool required)
{
    v8::Isolate *i = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = i->GetCurrentContext();
    bool has = obj->Has(ctx, binder::to_v8(i, field)).FromMaybe(false);
    if (!has && required)
        g_throw(TypeError, fmt::format("Options missing field \'{}\'", field));

    v8::Local<v8::Value> value;
    if (!has || !obj->Get(ctx, binder::to_v8(i, field)).ToLocal(&value))
        return value;

    auto realType = binder::from_v8<std::string>(i, value->TypeOf(i));
    if (realType != type)
        g_throw(TypeError, fmt::format("Options require property '{}' with {} type", field, type));

    return value;
}

template<typename T>
T extract_object_field(v8::Local<v8::Object> obj, const char *field,
                       const char *type, const std::optional<T>& default_)
{
    auto maybe = check_object_field(obj, field, type, !default_.has_value());
    if (maybe.IsEmpty())
        return default_.value();
    return binder::from_v8<T>(v8::Isolate::GetCurrent(), maybe.ToLocalChecked());
}

std::vector<std::string> extract_string_array(v8::Local<v8::Object> obj, const char *field)
{
    auto maybe = check_object_field(obj, field, GALLIUM_JS_TYPEOF_OBJECT, false);
    if (maybe.IsEmpty())
        return {};

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

    auto prop = maybe.ToLocalChecked()->ToObject(ctx).ToLocalChecked();
    if (!prop->IsArray())
    {
        g_throw(TypeError, fmt::format("Options require property '{}' with array type", field));
    }

    v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(prop);
    uint32_t size = array->Length();

    std::vector<std::string> result;
    for (uint32_t i = 0; i < size; i++)
    {
        v8::Local<v8::Value> v = array->Get(ctx, i).ToLocalChecked();
        if (!v->IsString())
        {
            g_throw(TypeError, fmt::format("Options require property '{}' to be a string array", field));
        }
        result.emplace_back(binder::from_v8<std::string>(isolate, v));
    }

    return result;
}

} // namespace anonymous

void on_process_exit_callback(uv_process_t *process, int64_t status, int32_t signal)
{
    void *closure = uv_handle_get_data(reinterpret_cast<uv_handle_t*>(process));
    auto *wrap = reinterpret_cast<ProcessWrap*>(closure);
    v8::HandleScope scope(wrap->isolate_);

    v8::Local<v8::Context> ctx = wrap->isolate_->GetCurrentContext();

    v8::Local<v8::Object> obj = v8::Object::New(wrap->isolate_);
    obj->Set(ctx, binder::to_v8(wrap->isolate_, "status"),
             binder::to_v8(wrap->isolate_, status)).Check();
    obj->Set(ctx, binder::to_v8(wrap->isolate_, "signal"),
             binder::to_v8(wrap->isolate_, signal)).Check();

    wrap->has_stopped_ = true;
    wrap->on_exit_promise_.Get(wrap->isolate_)->Resolve(ctx, obj).Check();
    wrap->markCanBeGarbageCollected();

    wrap->detachHandle();
}

v8::Local<v8::Value> ProcessWrap::Fork(v8::Local<v8::Object> options)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    uv_process_options_t proc_opts{};

    proc_opts.exit_cb = on_process_exit_callback;

    auto file = extract_object_field<std::string>(options, "file", GALLIUM_JS_TYPEOF_STRING, {});
    proc_opts.file = file.c_str();

    std::vector<std::string> argv = extract_string_array(options, "args");
    proc_opts.args = reinterpret_cast<char**>(malloc((argv.size() + 2) * sizeof(char*)));
    proc_opts.args[0] = const_cast<char *>(file.c_str());
    proc_opts.args[argv.size() + 1] = nullptr;
    for (size_t i = 0; i < argv.size(); i++)
        proc_opts.args[i + 1] = const_cast<char *>(argv[i].c_str());

    std::vector<std::string> envs = extract_string_array(options, "env");
    if (!envs.empty())
    {
        proc_opts.env = reinterpret_cast<char**>(malloc((envs.size() + 2) * sizeof(char*)));
        proc_opts.env[envs.size()] = nullptr;
        for (size_t i = 0; i < envs.size(); i++)
            proc_opts.env[i] = const_cast<char *>(envs[i].c_str());
    }
    else
        proc_opts.env = nullptr;

    ScopeExitAutoInvoker epi([&proc_opts] {
        std::free(proc_opts.env);
        std::free(proc_opts.args);
    });

    auto cwd = extract_object_field<std::string>(options, "cwd", GALLIUM_JS_TYPEOF_STRING, "");
    proc_opts.cwd = cwd.empty() ? nullptr : cwd.c_str();

    auto maybe = check_object_field(options, "uid", GALLIUM_JS_TYPEOF_NUMBER, false);
    if (!maybe.IsEmpty())
    {
        proc_opts.uid = extract_object_field<uv_uid_t>(options, "uid", GALLIUM_JS_TYPEOF_NUMBER, {});
        proc_opts.flags |= UV_PROCESS_SETUID;
    }

    maybe = check_object_field(options, "gid", GALLIUM_JS_TYPEOF_NUMBER, false);
    if (!maybe.IsEmpty())
    {
        proc_opts.gid = extract_object_field<uv_gid_t>(options, "gid", GALLIUM_JS_TYPEOF_NUMBER, {});
        proc_opts.flags |= UV_PROCESS_SETGID;
    }

    proc_opts.stdio_count = 3;
    proc_opts.stdio = new uv_stdio_container_t[3];
    proc_opts.stdio[0].flags = UV_IGNORE;
    proc_opts.stdio[1].flags = UV_IGNORE;
    proc_opts.stdio[2].flags = UV_IGNORE;
    ScopeExitAutoInvoker stdio_epi([&proc_opts] {
        delete[] proc_opts.stdio;
    });
#define Tp std::make_pair
    v8::Local<v8::Object> streams[3];
    for (auto& field : {Tp("stdin", 0), Tp("stdout", 1), Tp("stderr", 2)})
    {
        maybe = check_object_field(options, field.first, GALLIUM_JS_TYPEOF_NUMBER, false);
        if (!maybe.IsEmpty())
        {
            auto bits = extract_object_field<uint32_t>(options, field.first, GALLIUM_JS_TYPEOF_NUMBER, {});
            if (bits == GAL_PROC_STREAM_INHERIT)
            {
                proc_opts.stdio[field.second].flags = UV_INHERIT_FD;
                proc_opts.stdio[field.second].data.fd = field.second;
            }
            else if (bits == GAL_PROC_STREAM_REDIRECT)
            {
                /*
                auto *pipe = StreamWrap::Allocate<uv_pipe_t>();
                uv_pipe_init(EventLoop::Ref().handle(), pipe, false);

                int flags = UV_CREATE_PIPE;
                flags |= field.second == 0 ? UV_READABLE_PIPE : UV_WRITABLE_PIPE;

                proc_opts.stdio[field.second].flags = static_cast<uv_stdio_flags>(flags);
                proc_opts.stdio[field.second].data.stream = reinterpret_cast<uv_stream_t *>(pipe);

                streams[field.second] = binder::NewObject<StreamWrap>(isolate,
                        reinterpret_cast<uv_stream_t *>(pipe));
                */

                // TODO(sora): ! Implement this as soon as possible

                CHECK_FAILED("Not implemented yet!");
            }
            else if (bits)
            {
                g_throw(Error, fmt::format("Unrecognized bitfield in {} flags", field.first));
            }
        }
    }
#undef Tp

    auto *proc = new uv_process_t;
    v8::Local<v8::Object> resultObject = binder::NewObject<ProcessWrap>(isolate, proc, streams);
    binder::UnwrapObject<ProcessWrap>(isolate, resultObject)->setGCObjectSelfHandle(resultObject);

    int ret = uv_spawn(EventLoop::Ref().handle(), proc, &proc_opts);
    if (ret < 0)
        g_throw(Error, fmt::format("Failed in spawn: {}", uv_strerror(ret)));

    return resultObject;
}

v8::Local<v8::Value> GetEnviron()
{
    uv_env_item_t *envItems;
    int count;
    int ret = uv_os_environ(&envItems, &count);
    if (ret < 0)
        g_throw(Error, fmt::format("Failed in getting environments: {}", uv_strerror(ret)));

    ScopeExitAutoInvoker epi([envItems, count] {
        uv_os_free_environ(envItems, count);
    });

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    v8::Local<v8::Map> result = v8::Map::New(isolate);
    for (int i = 0; i < count; i++)
    {
        v8::Local<v8::String> name = binder::to_v8(isolate, envItems[i].name,
                                                   std::strlen(envItems[i].name));
        v8::Local<v8::String> value = binder::to_v8(isolate, envItems[i].value,
                                                    std::strlen(envItems[i].value));
        if (result->Set(ctx, name, value).IsEmpty())
        {
            g_throw(Error, fmt::format("Failed in resolving environment: name={} value={}",
                                       envItems[i].name, envItems[i].value));
        }
    }

    return result;
}

void DumpNativeHeapProfile()
{
    // mallctl("prof.dump", nullptr, nullptr, nullptr, 0);
    // TODO(sora): deprecate this
}

GALLIUM_BINDINGS_NS_END
