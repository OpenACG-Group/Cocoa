#include <iostream>
#include "gtest/gtest.h"

#include "Core/CrpkgImage.h"
#include "Core/CrpkgInputStream.h"
#include "Core/Journal.h"
#include "Core/Filesystem.h"
#include "Core/Utils.h"
using namespace cocoa;

class CrpkgTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Journal::New(LOG_LEVEL_DISABLED,
                     Journal::OutputDevice::kStandardOut,
                     false);
        std::string execFile = utils::GetExecutablePath();
        std::string execPath = execFile.substr(0, execFile.find_last_of('/') + 1);
        // Realpath() to simplify the path.
        fSampleFile = vfs::Realpath(execPath + "/../../res/TestSample.crpkg");
        fImage = CrpkgImage::Make(fSampleFile);
    }

    void TearDown() override
    {
        Journal::Delete();
    }

    std::string     fSampleFile;
    std::shared_ptr<CrpkgImage> fImage;
};

TEST_F(CrpkgTest, ImageOpenAssertion)
{
    ASSERT_NE(fImage, nullptr);
}

TEST_F(CrpkgTest, FileRead)
{
    auto file = fImage->openFile("/signature.txt");
    EXPECT_NE(file, nullptr);
    auto stat = file->stat();
    EXPECT_TRUE(stat);

    auto *buffer = new uint8_t[stat->size + 1];
    EXPECT_GE(file->read(buffer, stat->size), 0);
    delete[] buffer;
}

TEST_F(CrpkgTest, FileInputStream)
{
    CrpkgInputStream in(fImage->openFile("/signature.txt"));
    std::string word1, word2;
    in >> word1 >> word2;
    EXPECT_EQ(word1, "#[[signature]]");
    EXPECT_EQ(word2, "cocoa::test_system:test-crpkg-package");
}
