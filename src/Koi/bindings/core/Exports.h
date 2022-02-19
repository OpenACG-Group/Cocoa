#ifndef COCOA_CORE_EXPORTS_H
#define COCOA_CORE_EXPORTS_H

#include "Core/EventSource.h"
#include "Core/Properties.h"
#include "Koi/bindings/Base.h"
#include "Koi/Runtime.h"
#include "Koi/binder/Class.h"

KOI_BINDINGS_NS_BEGIN

class FileWrap;
class PropertyWrap;
class Buffer;

class CoreBinding : public BindingBase
{
public:
    CoreBinding();
    ~CoreBinding() override;
    const char *onGetUniqueId() override;
    void onGetModule(binder::Module& self) override;
    void onSetInstanceProperties(v8::Local<v8::Object> instance) override;
    void onRegisterClasses(v8::Isolate *isolate) override;
    const char **onGetExports() override;

    binder::Class<PropertyWrap> *class_property_wrap_;
    binder::Class<Buffer>       *class_buffer_;
    binder::Class<FileWrap>     *class_file_wrap_;
};

void Print(const std::string& str);
void Dump(const std::string& what);
void Exit();

struct FsRequest;

/* JSDecl: class File */
class FileWrap
{
public:
    explicit FileWrap(uv_file fd);
    ~FileWrap();

    /* JSDecl: function close(): Promise<void> */
    koi_nodiscard v8::Local<v8::Value> close();

    /* JSDecl: function isClosed(): boolean */
    koi_nodiscard bool isClosed() const;

    /* JSDecl: function isClosing(): boolean */
    koi_nodiscard bool isClosing() const;

    /* JSDecl: function read(dst: Buffer, dstOffset: number, size: number, offset: number): Promise<number> */
    koi_nodiscard v8::Local<v8::Value> read(v8::Local<v8::Value> dst, int64_t dstOffset,
                                            size_t size, int64_t offset);

    /* JSDecl: function write(src: Buffer, srcOffset: number, size: number, offset: number): Promise<number> */
    koi_nodiscard v8::Local<v8::Value> write(v8::Local<v8::Value> src, int64_t srcOffset,
                                             size_t size, int64_t offset);

    /**
     * JSDecl:
     * interface Stat {
     *     dev: number;
     *     mode: number;
     *     nlink: number;
     *     uid: number;
     *     gid: number;
     *     rdev: number;
     *     blksize: number;
     *     ino: number;
     *     size: number;
     *     blocks: number;
     *     atimeMs: number;
     *     mtimeMs: number;
     *     ctimeMs: number;
     *     atime: Date;
     *     mtime: Date;
     *     ctime: Date;
     * }
     */

    /* JSDecl: function fstat(): Promise<Stat> */
    koi_nodiscard v8::Local<v8::Value> fstat();

    /* JSDecl: function fsync(): Promise<void> */
    koi_nodiscard v8::Local<v8::Value> fsync();

    /* JSDecl: function fdatasync(): Promise<void> */
    koi_nodiscard v8::Local<v8::Value> fdatasync();

    /* JSDecl: function ftruncate(length: number): Promise<void> */
    koi_nodiscard v8::Local<v8::Value> ftruncate(off_t length);

    /* JSDecl: function fchmod(mode: number): Promise<void> */
    koi_nodiscard v8::Local<v8::Value> fchmod(int32_t mode);

    /* JSDecl: function futime(atime: number, mtime: number): Promise<void> */
    v8::Local<v8::Value> futime(double atime, double mtime);

    /* JSDecl: function fchown(uid: number, gid: number): Promise<void> */
    v8::Local<v8::Value> fchown(uv_uid_t uid, uv_gid_t gid);

    bool        closed_;
    bool        is_closing_;
    uv_file     fd_;
    std::list<FsRequest*> pending_requests_;
};

/* JSDecl: function open(path: string, flags: number, mode: number): Promise<File> */
v8::Local<v8::Value> Open(const std::string& path, int32_t flags, int32_t mode);

/* JSDecl: function unlink(path: string): Promise<void> */
v8::Local<v8::Value> Unlink(const std::string& path);

/* JSDecl: function mkdir(path: string, mode: number): Promise<void> */
v8::Local<v8::Value> Mkdir(const std::string& path, int32_t mode);

/* JSDecl: function mkdtemp(tpl: string): Promise<string> */
v8::Local<v8::Value> Mkdtemp(const std::string& tpl);

/**
 * JSDecl:
 * interface FileWithPath {
 *     file: File;
 *     path: string;
 * }
 */

/* JSDecl: function mkstemp(tpl: string): Promise<FileWithPath> */
v8::Local<v8::Value> Mkstemp(const std::string& tpl);

/* JSDecl: function rmdir(path: string): Promise<void> */
v8::Local<v8::Value> Rmdir(const std::string& path);

/* JSDecl: function stat(path: string): Promise<Stat> */
v8::Local<v8::Value> Stat(const std::string& path);

/* JSDecl: function lstat(path: string): Promise<Stat> */
v8::Local<v8::Value> LStat(const std::string& path);

/* JSDecl: function rename(path: string, newPath: string): Promise<void> */
v8::Local<v8::Value> Rename(const std::string& path, const std::string& newPath);

/* JSDecl: function access(path: string, mode: number): Promise<void> */
v8::Local<v8::Value> Access(const std::string& path, int32_t mode);

/* JSDecl: function chmod(path: string, mode: number): Promise<void> */
v8::Local<v8::Value> Chmod(const std::string& path, int32_t mode);

/* JSDecl: function utime(path: string, atime: number, mtime: number): Promise<void> */
v8::Local<v8::Value> UTime(const std::string& path, double atime, double mtime);

/* JSDecl: function lutime(path: string, atime: number, mtime: number): Promise<void> */
v8::Local<v8::Value> LUTime(const std::string& path, double atime, double mtime);

/* JSDecl: function link(path: string, newPath: string): Promise<void> */
v8::Local<v8::Value> Link(const std::string& path, const std::string& newPath);

/* JSDecl: function symlink(path: string, newPath: string, flags: number): Promise<void> */
v8::Local<v8::Value> Symlink(const std::string& path, const std::string& newPath, int32_t flags);

/* JSDecl: function readlink(path: string): Promise<string> */
v8::Local<v8::Value> Readlink(const std::string& path);

/* JSDecl: function realpath(path: string): Promise<string> */
v8::Local<v8::Value> Realpath(const std::string& path);

/* JSDecL: function chown(path: string, uid: number, gid: number): Promise<void> */
v8::Local<v8::Value> Chown(const std::string& path, uv_uid_t uid, uv_gid_t gid);

/* JSDecL: function lchown(path: string, uid: number, gid: number): Promise<void> */
v8::Local<v8::Value> LChown(const std::string& path, uv_uid_t uid, uv_gid_t gid);

/* JSDecl: class Property */
class PropertyWrap
{
public:
    /* JSDecl: static const PROT_*: number */
    enum class Prot : uint32_t
    {
        /* JS and C++ readable/writable */
        kPublic     = 0,

        /* JS readable, C++ readable/writable */
        kPrivate    = 1,

        kLast       = kPrivate
    };

    /* JSDecl: static const TYPE_*: number */
    enum class Type : uint32_t
    {
        kObject     = 0,
        kArray      = 1,
        kData       = 2,
        kLast       = kData
    };

    explicit PropertyWrap(const std::shared_ptr<PropertyNode>& node);
    ~PropertyWrap();

    static void InstallProperties();
    static v8::Local<v8::Object> GetWrap(v8::Isolate *isolate, const std::shared_ptr<PropertyNode>& node);

    koi_nodiscard std::shared_ptr<PropertyNode> lockNode() const {
        return fNode.lock();
    }

    /* JSDecl: const type: number */
    koi_nodiscard v8::Local<v8::Value> getType() const;

    /* JSDecl: const parent: Property */
    koi_nodiscard v8::Local<v8::Value> getParent() const;

    /* JSDecl: name: string */
    koi_nodiscard v8::Local<v8::Value> getName() const;
    void setName(v8::Local<v8::Value> name) const;

    /* JSDecl: const protection: number */
    koi_nodiscard v8::Local<v8::Value> getProtection() const;

    /* JSDecl: const numberOfChildren: number */
    koi_nodiscard v8::Local<v8::Value> getNumberOfChildren() const;

    /* JSDecl: function foreachChild(func: (child: Property) => void): void */
    void foreachChild(v8::Local<v8::Value> callback) const;

    /* JSDecl: function findChild(name: string): Property */
    koi_nodiscard v8::Local<v8::Value> findChild(const std::string& name) const;

    /* JSDecl: function insertChild(type: number, name: string): Property */
    koi_nodiscard v8::Local<v8::Value> insertChild(int32_t type, const std::string& name) const;

    /* JSDecl: function pushbackChild(type: number): Property */
    koi_nodiscard v8::Local<v8::Value> pushbackChild(int32_t type) const;

    /* JSDecl: function detachFromParent(): void */
    void detachFromParent();

    /* JSDecl: function extract(): any (maybe undefined) */
    koi_nodiscard v8::Local<v8::Value> extract() const;

    /* JSDecl: function resetData(value?: any): void */
    void resetData(const v8::FunctionCallbackInfo<v8::Value>& value) const;

    /* JSDecl: function hasData(): boolean */
    koi_nodiscard bool hasData() const;

    /* JSDecl: function dataValueRTTI(): string */
    koi_nodiscard std::string dataTypeinfo() const;

private:
    void checkNodeProtectionForJSWriting() const;

    std::weak_ptr<PropertyNode>   fNode;
};

class Buffer
{
public:
    explicit Buffer(const v8::FunctionCallbackInfo<v8::Value>& info);
    ~Buffer();

    size_t length();
    uint8_t byteAt(int64_t idx);
    v8::Local<v8::Value> copy(const v8::FunctionCallbackInfo<v8::Value>& args);
    v8::Local<v8::Value> toDataView(const v8::FunctionCallbackInfo<v8::Value>& args);
    v8::Local<v8::Value> toString(const std::string& coding, int32_t length);

    uint8_t *getWriteableDataPointerByte();

private:
    v8::Global<v8::Uint8Array>  fArray;
    std::shared_ptr<v8::BackingStore> fBackingStore;
};

KOI_BINDINGS_NS_END
#endif //COCOA_CORE_EXPORTS_H
