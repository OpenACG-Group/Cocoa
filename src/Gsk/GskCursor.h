#ifndef COCOA_GSKCURSOR_H
#define COCOA_GSKCURSOR_H

#include <utility>

#include "include/core/SkImage.h"

#include "Gsk/Gsk.h"
GSK_NAMESPACE_BEGIN

class GskCursor
{
public:
    struct HashFunctor
    {
        g_inline size_t operator()(const Handle<GskCursor>& cursor) const {
            return cursor->hash();
        }
    };

    struct EqualFunctor
    {
        bool operator()(const Handle<GskCursor>& a, const Handle<GskCursor>& b) const {
            return a->equalTo(b);
        }
    };

    ~GskCursor() = default;

    static Handle<GskCursor> MakeFromTexture(const sk_sp<SkImage>& texture,
                                             const Vec2i& hotspot,
                                             const Handle<GskCursor>& fallback);

    static Handle<GskCursor> MakeFromName(const std::string& name,
                                          const Handle<GskCursor>& fallback);

    g_nodiscard g_inline Handle<GskCursor> getFallback() const {
        return fFallback;
    }

    g_nodiscard g_inline const std::string& getName() const {
        return fName;
    }

    g_nodiscard g_inline sk_sp<SkImage> getTexture() const {
        return fTexture;
    }

    g_nodiscard g_inline Vec2i getHotspot() const {
        return fHotspot;
    }

    g_nodiscard uint64_t hash() const;
    g_nodiscard bool equalTo(const Handle<GskCursor>& cursor) const;

private:
    GskCursor(const Handle<GskCursor>& fallback,
              std::string name,
              const sk_sp<SkImage>& texture,
              const Vec2i& hotspot)
        : fFallback(fallback)
        , fName(std::move(name))
        , fTexture(texture)
        , fHotspot(hotspot) {}

    Handle<GskCursor>   fFallback;
    std::string         fName;
    sk_sp<SkImage>      fTexture;
    Vec2i               fHotspot;
};

GSK_NAMESPACE_END
#endif //COCOA_GSKCURSOR_H
