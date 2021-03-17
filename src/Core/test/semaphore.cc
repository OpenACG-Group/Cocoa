#include "../BlockableSemaphore.h"

int main()
{
    cocoa::BlockableSemaphore sem;
    sem.write(1);
    sem.read();

    return 0;
}
