#include "include/v8.h"

#include "Koi/binder/Convert.h"
#include "Koi/bindings/Base.h"
#include "Koi/bindings/core/Exports.h"
#include "Koi/bindings/core/FdRandomize.h"

KOI_BINDINGS_NS_BEGIN

CoreBinding::CoreBinding()
        : BindingBase("core", "Basic language features for Cocoa JavaScript")
        , class_property_wrap_(nullptr)
        , class_buffer_(nullptr)
        , class_file_wrap_(nullptr)
{
    FDLRInitialize();
    PropertyWrap::InstallProperties();
}

CoreBinding::~CoreBinding()
{
    delete class_file_wrap_;
    delete class_property_wrap_;
    delete class_buffer_;
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
    class_file_wrap_ = new binder::Class<FileWrap>(isolate);
    (*class_file_wrap_)
        .constructor<uv_file>()
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
        .set("close", &FileWrap::close)
        .set("isClosed", &FileWrap::isClosed)
        .set("isClosing", &FileWrap::isClosing)
        .set("read", &FileWrap::read)
        .set("write", &FileWrap::write)
        .set("fstat", &FileWrap::fstat)
        .set("fsync", &FileWrap::fsync)
        .set("fdatasync", &FileWrap::fdatasync)
        .set("ftruncate", &FileWrap::ftruncate);

    class_property_wrap_ = new binder::Class<PropertyWrap>(isolate);
    (*class_property_wrap_)
        .constructor<const std::shared_ptr<PropertyNode>&>()
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

    class_buffer_ = new binder::Class<Buffer>(isolate);
    (*class_buffer_)
        .constructor<const v8::FunctionCallbackInfo<v8::Value>&>()
        .set("length", binder::Property(&Buffer::length))
        .set("byteAt", &Buffer::byteAt)
        .set("copy", &Buffer::copy)
        .set("toDataView", &Buffer::toDataView)
        .set("toString", &Buffer::toString);
}

KOI_BINDINGS_NS_END
