#ifndef COCOA_AVDECODER_H
#define COCOA_AVDECODER_H

#include <vector>

#include "Core/Data.h"
#include "Vanilla/Base.h"
#include "Vanilla/Codec/AVFrame.h"
VANILLA_NS_BEGIN

enum class AVStreamType
{
    kAudio,
    kVideo,
    kSubtitle,
    kUnknown
};

class AVStreamSelector
{
public:
    AVStreamSelector() = default;
    ~AVStreamSelector() = default;

    void setAudio(const std::string& specifier);
    void setVideo(const std::string& specifier);
    void setSubtitle(const std::string& specifier);
    va_nodiscard const std::string& getSpecifier(AVStreamType type) const;

private:
    std::string     fAudio;
    std::string     fVideo;
    std::string     fSubtitle;
};

class AVDecoder
{
public:
    struct DecoderPrivate;
    enum class ReadingStatus
    {
        kOk,
        kEof,
        kAgain,
        kError
    };

    AVDecoder(std::string name, DecoderPrivate *pData);
    AVDecoder(const AVDecoder&) = delete;
    AVDecoder& operator=(const AVDecoder&) = delete;
    ~AVDecoder();

    static Handle<AVDecoder> MakeFromStream(std::string name,
                                            const std::shared_ptr<Data>& data,
                                            const AVStreamSelector& selector);

    va_nodiscard inline std::string getName() const
    { return fName; }

    va_nodiscard int32_t getStreamCount();
    va_nodiscard bool hasStream(AVStreamType type);

    /**
     * @param out   For video frame, this always returns a single frame.
     *              For audio frame, this may return multiple frames.
     */
    ReadingStatus readFrame(std::vector<Handle<AVFrame>>& out);

private:
    std::string          fName;
    DecoderPrivate      *fData;
};

VANILLA_NS_END
#endif //COCOA_AVDECODER_H
