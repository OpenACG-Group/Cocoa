#include <string>
#include <iostream>
#include "shell.h"
KOI_BINDINGS_NS_BEGIN

extern "C" BindingBase *__g_cocoa_hook()
{
    return new ShellBinding();
}

uint8_t execute(const std::string& path)
{
    std::cout << "execute(" << path << ")\n";
    return 0;
}

KOI_BINDINGS_NS_END
