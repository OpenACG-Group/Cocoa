#include <iostream>

#include "crpkg/crpkg.h"
#include "crpkg/Compression.h"
#include "crpkg/Serializer.h"
#include "crpkg/Deserializer.h"
using namespace cocoa::crpkg;

int main(int argc, char const **argv)
{
    std::ofstream fs(argv[2], std::ios::trunc | std::ios::binary);
    std::ifstream input(argv[1], std::ios::binary);

    auto ctx = CompressContext::Make();
    auto compressor = ctx->compress(std::move(input));

    std::cout << "size: " << compressor.compressedSize() << std::endl;
    compressor.write([&](const void *ptr, size_t size) -> void {
        fs.write(reinterpret_cast<const char*>(ptr), size);
    });
    return 0;
}
