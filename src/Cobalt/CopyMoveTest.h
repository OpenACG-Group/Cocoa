#ifndef COCOA_COBALT_COPYMOVETEST_H
#define COCOA_COBALT_COPYMOVETEST_H

#include "fmt/format.h"

class CopyMoveTest
{
public:
    CopyMoveTest() {
        fmt::print("constructor\n");
    }

    ~CopyMoveTest() {
        fmt::print("destructor\n");
    }

    CopyMoveTest(CopyMoveTest&& rhs) noexcept {
        fmt::print("move-constructor\n");
    }

    CopyMoveTest(const CopyMoveTest& lhs) {
        fmt::print("copy-constructor\n");
    }

    void foo() {
        fmt::print("foo()\n");
    }
};

#endif //COCOA_COBALT_COPYMOVETEST_H
