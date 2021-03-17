#include <iostream>
#include <unistd.h>

#include "Ciallo/RendererThread.h"
CIALLO_BEGIN_NS

class MyWorker : public Worker
{
public:
    void init() override
    {
        std::cout << "Thread initialize" << std::endl;
    }

    void final() override
    {
        std::cout << "Thread finalize" << std::endl;
    }

    Thread::CmdExecuteResult execute(const Thread::Command& task) override
    {
        std::cout << "Thread task, opcode = " << task.opcode()
                  << ", data = " << task.userdata().extract<int>() << std::endl;
        return Thread::CmdExecuteResult::kNormal;
    }
};

void test()
{
    MyWorker myWorker;
    Thread thread("Worker", &myWorker);

    for (int i = 0; i < 100000000; i++)
    {
        auto fence = thread.enqueueCmd(Thread::Command(i, Poco::Dynamic::Var(2233)));
        fence->wait();
        std::cout << "Done" << std::endl;
    }
}

CIALLO_END_NS

int main(int argc, char const **argv)
{
    cocoa::ciallo::test();
    return 0;
}
