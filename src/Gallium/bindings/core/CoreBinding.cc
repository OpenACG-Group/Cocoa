#include "include/v8.h"

#include "Gallium/binder/Convert.h"
#include "Gallium/bindings/Base.h"
#include "Gallium/bindings/core/Exports.h"
#include "Gallium/bindings/core/FdRandomize.h"

GALLIUM_BINDINGS_NS_BEGIN

CoreBinding::CoreBinding()
        : BindingBase("core", "Basic language features for Cocoa JavaScript")
{
    FDLRInitialize();
    PropertyWrap::InstallProperties();
}

CoreBinding::~CoreBinding()
{
    FDLRCollectAndSweep();
}

void CoreBinding::onSetInstanceProperties(v8::Local<v8::Object> instance)
{
    v8::Isolate *i = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = i->GetCurrentContext();

    v8::Local<v8::Array> argv = v8::Array::New(i);
    instance->Set(ctx, binder::to_v8(i, "args"), argv).Check();
    auto pass = prop::Get()->next("Runtime")->next("Script")
            ->next("Pass")->as<PropertyArrayNode>();
    int32_t index = 0;
    for (const auto& node : *pass)
    {
        auto str = node->as<PropertyDataNode>()->extract<std::string>();
        argv->Set(ctx, index++, binder::to_v8(i, str)).Check();
    }

    v8::Local<v8::Object> property = PropertyWrap::GetWrap(i, prop::Get());
    instance->Set(ctx, binder::to_v8(i, "property"), property).Check();
}

void CoreBinding::onRegisterClasses(v8::Isolate *isolate)
{
    class_stream_weak_buffer_ = NewClassExport<StreamWeakBuffer>(isolate);
    (*class_stream_weak_buffer_)
        .set("toStrongOwnership", &StreamWeakBuffer::toStrongOwnership)
        .set("expired", binder::Property(&StreamWeakBuffer::isExpired));

    class_stream_read_iterator_ = NewClassExport<StreamReadIterator>(isolate);
    (*class_stream_read_iterator_)
        .set("next", &StreamReadIterator::next)
        .set("return", &StreamReadIterator::return_)
        .set("throw", &StreamReadIterator::throw_);

    class_stream_wrap_ = NewClassExport<StreamWrap>(isolate);
    (*class_stream_wrap_)
        .set_static_func("OpenTTYStdin", StreamWrap::OpenTTYStdin)
        .set("writable", binder::Property(&StreamWrap::isWritable))
        .set("readable", binder::Property(&StreamWrap::isReadable))
        .set(v8::Symbol::GetAsyncIterator(isolate), &StreamWrap::asyncIterator);

    class_file_wrap_ = NewClassExport<FileWrap>(isolate);
    (*class_file_wrap_)
        .set_static("O_FILEMAP", UV_FS_O_FILEMAP)
        .set_static("O_RANDOM", UV_FS_O_RANDOM)
        .set_static("O_SHORT_LIVED", UV_FS_O_SHORT_LIVED)
        .set_static("O_SEQUENTIAL", UV_FS_O_SEQUENTIAL)
        .set_static("O_TEMPORARY", UV_FS_O_TEMPORARY)
        .set_static("O_APPEND", UV_FS_O_APPEND)
        .set_static("O_CREAT", UV_FS_O_CREAT)
        .set_static("O_DIRECT", UV_FS_O_DIRECT)
        .set_static("O_DIRECTORY", UV_FS_O_DIRECTORY)
        .set_static("O_DSYNC", UV_FS_O_DSYNC)
        .set_static("O_EXCL", UV_FS_O_EXCL)
        .set_static("O_EXLOCK", UV_FS_O_EXLOCK)
        .set_static("O_NOATIME", UV_FS_O_NOATIME)
        .set_static("O_NOCTTY", UV_FS_O_NOCTTY)
        .set_static("O_NOFOLLOW", UV_FS_O_NOFOLLOW)
        .set_static("O_NONBLOCK", UV_FS_O_NONBLOCK)
        .set_static("O_RDONLY", UV_FS_O_RDONLY)
        .set_static("O_RDWR", UV_FS_O_RDWR)
        .set_static("O_SYMLINK", UV_FS_O_SYMLINK)
        .set_static("O_SYNC", UV_FS_O_SYNC)
        .set_static("O_TRUNC", UV_FS_O_TRUNC)
        .set_static("O_WRONLY", UV_FS_O_WRONLY)
        .set_static("S_IRWXU", S_IRWXU)
        .set_static("S_IRUSR", S_IRUSR)
        .set_static("S_IWUSR", S_IWUSR)
        .set_static("S_IXUSR", S_IXUSR)
        .set_static("S_IRWXG", S_IRWXG)
        .set_static("S_IRGRP", S_IRGRP)
        .set_static("S_IWGRP", S_IWGRP)
        .set_static("S_IXGRP", S_IXGRP)
        .set_static("S_IRWXO", S_IRWXO)
        .set_static("S_IROTH", S_IROTH)
        .set_static("S_IWOTH", S_IWOTH)
        .set_static("S_IXOTH", S_IXOTH)
        .set_static("S_IFMT", S_IFMT)
        .set_static("S_IFREG", S_IFREG)
        .set_static("S_IFDIR", S_IFDIR)
        .set_static("S_IFCHR", S_IFCHR)
        .set_static("S_IFBLK", S_IFBLK)
        .set_static("S_IFIFO", S_IFIFO)
        .set_static("S_IFLNK", S_IFLNK)
        .set_static("S_IFSOCK", S_IFSOCK)
        .set_static("F_OK", F_OK)
        .set_static("R_OK", R_OK)
        .set_static("W_OK", W_OK)
        .set_static("X_OK", X_OK)
        .set_static("SYMLINK_DIR", UV_FS_SYMLINK_DIR)
        .set_static("SYMLINK_JUNCTION", UV_FS_SYMLINK_JUNCTION)
        .set_static_func("Open", FileWrap::Open)
        .set("close", &FileWrap::close)
        .set("isClosed", &FileWrap::isClosed)
        .set("isClosing", &FileWrap::isClosing)
        .set("read", &FileWrap::read)
        .set("write", &FileWrap::write)
        .set("fstat", &FileWrap::fstat)
        .set("fsync", &FileWrap::fsync)
        .set("fdatasync", &FileWrap::fdatasync)
        .set("ftruncate", &FileWrap::ftruncate)
        .set("fchmod", &FileWrap::fchmod)
        .set("futime", &FileWrap::futime)
        .set("fchown", &FileWrap::fchown);


    class_property_wrap_ = NewClassExport<PropertyWrap>(isolate);
    (*class_property_wrap_)
        .set_static("PROT_PUBLIC", static_cast<uint32_t>(PropertyWrap::Prot::kPublic))
        .set_static("PROT_PRIVATE", static_cast<uint32_t>(PropertyWrap::Prot::kPrivate))
        .set_static("TYPE_OBJECT", static_cast<uint32_t>(PropertyWrap::Type::kObject))
        .set_static("TYPE_ARRAY", static_cast<uint32_t>(PropertyWrap::Type::kArray))
        .set_static("TYPE_DATA", static_cast<uint32_t>(PropertyWrap::Type::kData))
        .set("type", binder::Property(&PropertyWrap::getType))
        .set("parent", binder::Property(&PropertyWrap::getParent))
        .set("name", binder::Property(&PropertyWrap::getName, &PropertyWrap::setName))
        .set("protection", binder::Property(&PropertyWrap::getProtection))
        .set("numberOfChildren", binder::Property(&PropertyWrap::getNumberOfChildren))
        .set("foreachChild", &PropertyWrap::foreachChild)
        .set("findChild", &PropertyWrap::findChild)
        .set("insertChild", &PropertyWrap::insertChild)
        .set("pushbackChild", &PropertyWrap::pushbackChild)
        .set("detachFromParent", &PropertyWrap::detachFromParent)
        .set("extract", &PropertyWrap::extract)
        .set("hasData", &PropertyWrap::hasData)
        .set("resetData", &PropertyWrap::resetData)
        .set("dataTypeinfo", &PropertyWrap::dataTypeinfo);

    class_buffer_ = NewClassExport<Buffer>(isolate);
    (*class_buffer_)
        .constructor<const v8::FunctionCallbackInfo<v8::Value>&>()
        .set_static("ENCODE_LATIN1", static_cast<uint32_t>(Buffer::Encoding::kLatin1))
        .set_static("ENCODE_ASCII", static_cast<uint32_t>(Buffer::Encoding::kLatin1))
        .set_static("ENCODE_UTF8", static_cast<uint32_t>(Buffer::Encoding::kUtf8))
        .set_static("ENCODE_UCS2", static_cast<uint32_t>(Buffer::Encoding::kUcs2))
        .set_static("ENCODE_HEX", static_cast<uint32_t>(Buffer::Encoding::kHex))
        .set("length", binder::Property(&Buffer::length))
        .set("byteAt", &Buffer::byteAt)
        .set("copy", &Buffer::copy)
        .set("toDataView", &Buffer::toDataView)
        .set("toString", &Buffer::toString);

    class_process_wrap_ = NewClassExport<ProcessWrap>(isolate);
    (*class_process_wrap_)
        .set_static("STREAM_INHERIT", GAL_PROC_STREAM_INHERIT)
        .set_static("STREAM_REDIRECT", GAL_PROC_STREAM_REDIRECT)
        .set_static("SIGINT", SIGINT)
        .set_static("SIGILL", SIGILL)
        .set_static("SIGABRT", SIGABRT)
        .set_static("SIGFPE", SIGFPE)
        .set_static("SIGSEGV", SIGSEGV)
        .set_static("SIGTERM", SIGTERM)
        .set_static("SIGHUP", SIGHUP)
        .set_static("SIGQUIT", SIGQUIT)
        .set_static("SIGTRAP", SIGTRAP)
        .set_static("SIGKILL", SIGKILL)
        .set_static("SIGPIPE", SIGPIPE)
        .set_static("SIGALRM", SIGALRM)
        .set_static("SIGSTKFLT", SIGSTKFLT)
        .set_static("SIGPWR", SIGPWR)
        .set_static("SIGBUS", SIGBUS)
        .set_static("SIGSYS", SIGSYS)
        .set_static("SIGURG", SIGURG)
        .set_static("SIGSTOP", SIGSTOP)
        .set_static("SIGTSTP", SIGTSTP)
        .set_static("SIGCONT", SIGCONT)
        .set_static("SIGCHLD", SIGCHLD)
        .set_static("SIGTTIN", SIGTTIN)
        .set_static("SIGTTOU", SIGTTOU)
        .set_static("SIGPOLL", SIGPOLL)
        .set_static("SIGXFSZ", SIGXFSZ)
        .set_static("SIGXCPU", SIGXCPU)
        .set_static("SIGVTALRM", SIGVTALRM)
        .set_static("SIGPROF", SIGPROF)
        .set_static("SIGUSR1", SIGUSR1)
        .set_static("SIGUSR2", SIGUSR2)
        .set_static_func("Fork", ProcessWrap::Fork)
        .set("pid", binder::Property(&ProcessWrap::getPid))
        .set("kill", &ProcessWrap::kill)
        .set("getPipeStream", &ProcessWrap::getPipeStream)
        .set("promiseOnExit", &ProcessWrap::promiseOnExit);
}

GALLIUM_BINDINGS_NS_END
