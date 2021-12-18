#include <iostream>
#include <fstream>
#include "gtest/gtest.h"

#include "include/core/SkImageInfo.h"
#include "include/core/SkBitmap.h"

#include "Vanilla/Codec/AVDecoder.h"
#include "Core/CrpkgImage.h"
#include "Core/Filesystem.h"
#include "Core/Journal.h"
#include "Core/Data.h"
using namespace cocoa;
using namespace vanilla;

TEST(AVDecoderTest, OpenFile)
{
    Journal::New(LOG_LEVEL_DEBUG, Journal::OutputDevice::kStandardOut, true);
    ScopeEpilogue epilogue([]() -> void { Journal::Delete(); });
    std::string path = "/home/sora/Project/C++/Cocoa/res/TestSample.crpkg";
    auto file = CrpkgImage::Make(path)->openFile("/movie/gochiusa.flv");

    auto decoder = AVDecoder::MakeFromStream(path,
                                             Data::MakeFromPackage(file),
                                             AVStreamSelector());

    while (true)
    {
        std::vector<Handle<AVFrame>> frames;
        auto status = decoder->readFrame(frames);
        if (status == AVDecoder::ReadingStatus::kAgain)
            continue;
        else if (status == AVDecoder::ReadingStatus::kEof)
        {
            std::cout << "End of file" << std::endl;
            break;
        }
        else if (status == AVDecoder::ReadingStatus::kError)
        {
            std::cout << "Error" << std::endl;
            break;
        }

        if (frames.empty())
            continue;

        if (frames[0]->getType() == AVFrame::FrameType::kVideo)
        {
            std::cout << "Got one video frame" << std::endl;
            auto info = SkImageInfo::Make(960, 540, SkColorType::kBGRA_8888_SkColorType,
                                          SkAlphaType::kUnpremul_SkAlphaType);
            SkBitmap bitmap = AVVideoFrame::Cast(frames[0])->asBitmap(info);
            break;
        }
    }

    ASSERT_NE(decoder, nullptr);
}
