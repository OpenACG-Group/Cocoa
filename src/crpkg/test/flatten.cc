#include <iostream>
#include <cstring>

#include "crpkg/Serializer.h"
using namespace cocoa::crpkg;

char g_data11[] = "This is data11";
char g_data12[] = "This is data12";
char g_data21[] = "This is data21";
char g_data22[] = "This is data22";
char g_data0[] = "This is data0";

#define CT(val) reinterpret_cast<uint8_t*>(val)

int main()
{
    std::shared_ptr<uint8_t> data11(CT(strdup(g_data11)));
    std::shared_ptr<uint8_t> data12(CT(strdup(g_data12)));
    std::shared_ptr<uint8_t> data21(CT(strdup(g_data21)));
    std::shared_ptr<uint8_t> data22(CT(strdup(g_data22)));
    std::shared_ptr<uint8_t> data0(CT(strdup(g_data0)));

    INodeRoot root;
    auto dir1 = root.appendDir("dir1");
    dir1->appendFile("file11")->content()
        .setContent(data11)
        .setSize(sizeof(data11));

    dir1->appendFile("file12")->content()
            .setContent(data12)
            .setSize(sizeof(data12));

    auto dir2 = root.appendDir("dir2");
    dir2->appendFile("file21")->content()
            .setContent(data21)
            .setSize(sizeof(data21));

    dir2->appendFile("file22")->content()
            .setContent(data22)
            .setSize(sizeof(data22));

    root.appendFile("file0")->content()
            .setContent(data0)
            .setSize(sizeof(data0));

    Serializer serializer(&root);
    serializer.write("Test.crpkg.blob", "Test");
    return 0;
}
