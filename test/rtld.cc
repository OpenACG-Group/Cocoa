#include <iostream>
#include <dlfcn.h>

int main(int argc, char const *argv[])
{
    void *handle = dlopen("/usr/lib/libvulkan_radeon.so", RTLD_LAZY | RTLD_LOCAL);
    std::cout << handle << ": " << dlerror() << std::endl;
    return 0;
}

