#ifndef COCOA_BLOCKABLEIO_H
#define COCOA_BLOCKABLEIO_H

namespace cocoa {

class BlockableIO
{
public:
    virtual ~BlockableIO() = default;

    virtual int nativeFd() const = 0;
};

} // namespace cocoa
#endif //COCOA_BLOCKABLEIO_H
