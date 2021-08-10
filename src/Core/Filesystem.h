#ifndef COCOA_FILESYSTEM_H
#define COCOA_FILESYSTEM_H

#include <cstdint>
#include <type_traits>
#include <vector>
#include <string>

#define VFS_NS_BEGIN    namespace cocoa::vfs {
#define VFS_NS_END      }

VFS_NS_BEGIN

template<typename E, typename = typename std::enable_if<std::is_enum_v<E>>::type>
class Bitfield
{
public:
    using T = typename std::underlying_type<E>::type;
    Bitfield() : fValue(0) {}
    explicit Bitfield(E value) : fValue(static_cast<T>(value)) {}

    explicit Bitfield(const std::vector<E>& values) : fValue(0) {
        for (E v : values)
            fValue |= static_cast<T>(v);
    }

    Bitfield(const std::initializer_list<E>& values) : fValue(0) {
        for (E v : values)
            fValue |= static_cast<T>(v);
    }

    Bitfield& operator|=(const E bit) {
        fValue |= static_cast<T>(bit);
        return *this;
    }

    Bitfield operator|(const E bit) {
        Bitfield result = *this;
        result.fValue |= static_cast<T>(bit);
        return result;
    }

    bool operator&(const E bit) {
        return (fValue & static_cast<T>(bit)) == static_cast<T>(bit);
    }

private:
    T   fValue;
};

enum class OpenFlags : uint8_t
{
    kReadonly       = (1 << 0),
    kWriteOnly      = (1 << 1),
    kReadWrite      = (1 << 2),
    kCreate         = (1 << 3),
    kTrunc          = (1 << 4),
    kAppend         = (1 << 5)
};

enum class Mode : uint16_t
{
    kNone       = 0,
    kUsrW       = (1 << 0),
    kUsrR       = (1 << 1),
    kUsrX       = (1 << 2),
    kOthW       = (1 << 3),
    kOthR       = (1 << 4),
    kOthX       = (1 << 5),
    kGrpW       = (1 << 6),
    kGrpR       = (1 << 7),
    kGrpX       = (1 << 8),
    kDir        = (1 << 9),
    kLink       = (1 << 10),
    kRegular    = (1 << 11),
    kChar       = (1 << 12),
    kBlock      = (1 << 13),
    kFifo       = (1 << 14),
    kSocket     = (1 << 15)
};

enum class MapProtection : uint8_t
{
    kNone       = (1 << 0),
    kRead       = (1 << 1),
    kWrite      = (1 << 2),
    kExec       = (1 << 3)
};

enum class MapFlags : uint8_t
{
    kFixed      = (1 << 0),
    kShared     = (1 << 1),
    kPrivate    = (1 << 2)
};

enum class SeekWhence
{
    kSet,
    kCurrent,
    kEnd
};

enum class AccessMode : uint8_t
{
    kReadable   = (1 << 0),
    kWritable   = (1 << 1),
    kExecutable = (1 << 2),
    kExist      = (1 << 3)
};

enum class AccessResult
{
    kOk,
    kFailed
};

struct Stat
{
    uint32_t        linkCount;
    Bitfield<Mode>  mode;
    uint32_t        uid;
    uint32_t        gid;
    size_t          size;
    struct timespec atime;
    struct timespec mtime;
    struct timespec ctime;
};

int32_t Open(const std::string& path, Bitfield<OpenFlags> flags,
             Bitfield<Mode> mode = Bitfield<Mode>());
int32_t Close(int32_t fd);

int32_t Chdir(const std::string& path);
std::string ReadLink(const std::string& path);
std::string Realpath(const std::string& path);
AccessResult Access(const std::string& path, Bitfield<AccessMode> mode);
int32_t Rename(const std::string& old, const std::string& _new);

ssize_t FileSize(int32_t fd);
ssize_t FileSize(const std::string& path);

ssize_t Read(int32_t fd, void *buffer, size_t size);
ssize_t Write(int32_t fd, const void *buffer, size_t size);
off_t Seek(int32_t fd, off_t offset, SeekWhence whence);
void *MemMap(int32_t fd, void *address, Bitfield<MapProtection> protection,
             Bitfield<MapFlags> flags, size_t size, off64_t offset);
int32_t MemUnmap(void *address, size_t size);

VFS_NS_END

#endif //COCOA_FILESYSTEM_H
