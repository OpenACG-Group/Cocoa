#include <iostream>
#include "Core/Journal.h"
using namespace cocoa;

int main(int argc, const char **argv)
{
    Journal::New(LOG_LEVEL_DEBUG, Journal::OutputDevice::kStandardOut, true);

    char str[256];
    Journal::Ref()(LOG_INFO, "Hello, World\nline2 wewde\nline3 dsaihwda");

    Journal::Delete();
    return 0;
}
