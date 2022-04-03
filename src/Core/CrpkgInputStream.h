#ifndef COCOA_CORE_CRPKGINPUTSTREAM_H
#define COCOA_CORE_CRPKGINPUTSTREAM_H

#include <streambuf>
#include <istream>

#include "Core/CrpkgImage.h"
namespace cocoa {

class CrpkgStreamBuffer : public std::streambuf
{
public:
    CrpkgStreamBuffer(std::shared_ptr<CrpkgFile> file, size_t bufSize);
    ~CrpkgStreamBuffer() override;

    int_type underflow() override;
    pos_type seekoff(off_type offset,
                     std::ios_base::seekdir dir,
                     std::ios_base::openmode mode) override;

private:
    std::shared_ptr<CrpkgFile>   fFile;
    const size_t                 fBufferSize;
    char_type                   *fBuffer;
};

class CrpkgInputStream : public std::istream
{
public:
    explicit CrpkgInputStream(std::shared_ptr<CrpkgFile> file,
                              size_t bufSize = 4096);
    ~CrpkgInputStream() override;
};

} // namespace cocoa
#endif //COCOA_CORE_CRPKGINPUTSTREAM_H
