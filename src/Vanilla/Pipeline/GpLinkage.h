#ifndef COCOA_GPLINKAGE_H
#define COCOA_GPLINKAGE_H

#include <condition_variable>
#include <mutex>
#include <utility>

#include "include/core/SkPicture.h"
#include "include/core/SkImage.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkSurface.h"

#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class GpElement;

class GpLinkage
{
public:
    class Transfer
    {
    public:
        enum class Category
        {
            kPicture,
            kImage,
            kBitmap,
            kSurface,
            kEmpty
        };

        Transfer() : fCategory(Category::kEmpty) {}
        Transfer(const Transfer&) = default;
        Transfer& operator=(const Transfer&) = default;

        explicit Transfer(const sk_sp<SkPicture>& picture)
            : fCategory(Category::kPicture)
            , fPicture(picture) {}
        explicit Transfer(const sk_sp<SkImage>& image)
            : fCategory(Category::kImage)
            , fImage(image) {}
        explicit Transfer(SkBitmap bitmap)
            : fCategory(Category::kBitmap)
            , fBitmap(std::move(bitmap)) {}
        explicit Transfer(const sk_sp<SkSurface>& surface)
            : fCategory(Category::kSurface)
            , fSurface(surface) {}

        ~Transfer() = default;

        va_nodiscard inline bool isEmpty() const
        { return (fCategory == Category::kEmpty); }

        va_nodiscard Category getCategory() const
        { return fCategory; }

        va_nodiscard inline sk_sp<SkPicture> toPicture()
        { return fPicture; }
        va_nodiscard inline sk_sp<SkImage> toImage()
        { return fImage; }
        va_nodiscard inline SkBitmap& toBitmap()
        { return fBitmap; }
        va_nodiscard inline sk_sp<SkSurface> toSurface()
        { return fSurface; }

    private:
        Category            fCategory;
        sk_sp<SkPicture>    fPicture;
        sk_sp<SkImage>      fImage;
        SkBitmap            fBitmap;
        sk_sp<SkSurface>    fSurface;
    };

    GpLinkage(GpElement *src, int srcOutPad,
              GpElement *dst, int dstInPad)
        : fSrcElement(src)
        , fDstElement(dst)
        , fSrcOutPad(srcOutPad)
        , fDstInPad(dstInPad) {}

    ~GpLinkage() = default;

    va_nodiscard inline GpElement *etSrcElement()
    { return fSrcElement; }
    va_nodiscard inline GpElement *getDstElement()
    { return fDstElement; }
    va_nodiscard inline int getSrcOutPad() const
    { return fSrcOutPad; }
    va_nodiscard inline int getDstInPad() const
    { return fDstInPad; }

    inline void send(const Transfer& transfer)
    {
        {
            std::scoped_lock<std::mutex> lock(fTransferMutex);
            fTransfer = transfer;
        }
        fTransferredCond.notify_all();
    }

    va_nodiscard inline Transfer& waitForTransfer()
    {
        std::unique_lock<std::mutex> lock(fTransferMutex);
        while (fTransfer.isEmpty())
            fTransferredCond.wait(lock);
        return fTransfer;
    }

    va_nodiscard inline Transfer& getTransfer()
    {
        std::scoped_lock<std::mutex> lock(fTransferMutex);
        return fTransfer;
    }

    inline void reset()
    { fTransfer = Transfer(); }

private:
    GpElement                  *fSrcElement;
    GpElement                  *fDstElement;
    int                         fSrcOutPad;
    int                         fDstInPad;
    Transfer                    fTransfer;
    std::condition_variable     fTransferredCond;
    std::mutex                  fTransferMutex;
};

VANILLA_NS_END
#endif //COCOA_GPLINKAGE_H
