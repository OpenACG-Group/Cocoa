#include "Gsk/GskCursor.h"
GSK_NAMESPACE_BEGIN

Handle<GskCursor> GskCursor::MakeFromName(const std::string& name, const Handle<GskCursor>& fallback)
{
    if (name.empty())
        return nullptr;

    auto *ptr = new GskCursor(fallback,
                              name,
                              nullptr,
                              Vec2i(0, 0));
    return Handle<GskCursor>(ptr);
}

Handle<GskCursor> GskCursor::MakeFromTexture(const sk_sp<SkImage>& texture,
                                             const Vec2i& hotspot,
                                             const Handle<GskCursor>& fallback)
{
    if (hotspot[0] < 0 || hotspot[0] >= texture->width())
        return nullptr;
    if (hotspot[1] < 0 || hotspot[1] >= texture->height())
        return nullptr;

    auto *ptr = new GskCursor(fallback,
                              "",
                              texture,
                              hotspot);
    return Handle<GskCursor>(ptr);
}

uint64_t GskCursor::hash() const
{
    uint64_t result = fFallback ? fFallback->hash() : 0;

    if (!fName.empty())
        result ^= std::hash<std::string>()(fName);
    else
        result ^= sksp_hash(fTexture);

    result ^= (fHotspot[0] << 8) | fHotspot[1];

    return result;
}

bool GskCursor::equalTo(const Handle<GskCursor>& other) const
{
    if ((this->fFallback != nullptr) != (other->fFallback != nullptr))
        return false;
    if (this->fFallback != nullptr && !fFallback->equalTo(other->fFallback))
        return false;
    if (this->fName != other->fName)
        return false;
    if (this->fTexture != other->fTexture)
        return false;
    if (this->fHotspot != other->fHotspot)
        return false;

    return true;
}

GSK_NAMESPACE_END
