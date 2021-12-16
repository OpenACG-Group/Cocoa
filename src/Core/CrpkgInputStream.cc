#include "Core/Errors.h"

#include "Core/Journal.h"
#include "Core/CrpkgImage.h"
#include "Core/CrpkgInputStream.h"
namespace cocoa {

CrpkgStreamBuffer::CrpkgStreamBuffer(std::shared_ptr<CrpkgFile> file, size_t bufSize)
    : fFile(std::move(file)),
      fBufferSize(bufSize),
      fBuffer(nullptr)
{
    CHECK(fFile != nullptr);
    CHECK(fBufferSize > 0);
    fBuffer = new char_type[fBufferSize];
    std::streambuf::setg(fBuffer, fBuffer, fBuffer);
}

CrpkgStreamBuffer::~CrpkgStreamBuffer()
{
    delete[] fBuffer;
}

std::streambuf::int_type CrpkgStreamBuffer::underflow()
{
    fmt::print("real_pos={}\n", fFile->seek(vfs::SeekWhence::kCurrent, 0));
    ssize_t ret = fFile->read(eback(), static_cast<ssize_t>(fBufferSize));
    if (ret <= 0)
        return traits_type::eof();
    else
    {
        setg(eback(), eback(), eback() + ret);
        return traits_type::to_int_type(*gptr());
    }
}

std::streambuf::pos_type CrpkgStreamBuffer::seekoff(off_type offset,
                                                    std::ios_base::seekdir dir,
                                                    std::ios_base::openmode mode)
{
    vfs::SeekWhence whence;
    switch (dir)
    {
    case std::ios_base::beg:
        whence = vfs::SeekWhence::kSet;
        break;
    case std::ios_base::cur:
        whence = vfs::SeekWhence::kCurrent;
        break;
    case std::ios_base::end:
        whence = vfs::SeekWhence::kEnd;
        break;
    default:
        throw std::invalid_argument("Invalid argument dir for CrpkgStreamBuffer::seekoff");
    }
    off_t ret = fFile->seek(whence, offset);
    if (ret < 0)
        return -1;
    this->underflow();

    return ret;
}

CrpkgInputStream::CrpkgInputStream(std::shared_ptr<CrpkgFile> file, size_t bufSize)
    : std::istream(new CrpkgStreamBuffer(std::move(file), bufSize))
{
}

CrpkgInputStream::~CrpkgInputStream()
{
}

} // namespace cocoa
