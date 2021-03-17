#include <iostream>
#include <mutex>
#include <thread>
#include <unistd.h>

std::mutex mtx;

void run()
{
    std::cout << "Thread started" << std::endl;
    sleep(1);
    mtx.unlock();
    std::cout << "Thread Acquired" << std::endl;
}

int main()
{
    mtx.lock();
    std::thread th(run);

    mtx.lock();
    std::cout << "Local acquired" << std::endl;

    if (th.joinable())
        th.join();
    return 0;
}
