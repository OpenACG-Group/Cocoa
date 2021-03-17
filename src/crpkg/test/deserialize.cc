#include <iostream>

#include "crpkg/Deserializer.h"
using namespace cocoa::crpkg;

int main(int argc, const char **argv)
{
    Deserializer deserializer(argv[1]);

    deserializer.extractTo(nullptr);
    return 0;
}