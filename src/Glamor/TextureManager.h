/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef COCOA_GLAMOR_TEXTUREMANAGER_H
#define COCOA_GLAMOR_TEXTUREMANAGER_H

#include <utility>
#include <unordered_map>
#include <string_view>

#include "Core/Exception.h"
#include "Glamor/Glamor.h"
#include "Glamor/Texture.h"
#include "Glamor/GraphicsResourcesTrackable.h"
GLAMOR_NAMESPACE_BEGIN

class TextureFactory;

class TextureManager : public GraphicsResourcesTrackable
{
public:
    class ScopedTextureAcquire
    {
        CO_NONASSIGNABLE(ScopedTextureAcquire)
        CO_NONCOPYABLE(ScopedTextureAcquire)

    public:
        void* operator new(size_t size) = delete;
        void* operator new[](size_t size) = delete;
        void operator delete(void*, size_t) = delete;
        void operator delete[](void*, size_t) = delete;

        ScopedTextureAcquire(TextureManager& manager, Texture::TextureId id)
                : manager_(manager) {
            texture_ = manager_.AcquireTexture(id);
            if (!texture_)
                throw RuntimeException(__func__, "Could not acquire a texture object from ID");
        }
        ~ScopedTextureAcquire() {
            manager_.ReleaseTexture(texture_);
        }

        g_nodiscard g_inline Texture* Get() {
            return texture_;
        }

    private:
        TextureManager&           manager_;
        Texture                  *texture_;
    };

    using CreateCallback = std::function<Shared<Texture>(const Unique<TextureFactory>& factory)>;
    using DeletionCallback = std::function<void()>;

    explicit TextureManager(Unique<TextureFactory> factory);
    ~TextureManager();

    std::optional<Texture::TextureId> Create(const CreateCallback& callback,
                                             const std::string_view& annotation = "");
    bool Delete(Texture::TextureId id);

    Texture *AcquireTexture(Texture::TextureId id);
    void ReleaseTexture(Texture *texture);

    void SubscribeTextureDeletion(Texture::TextureId id, const DeletionCallback& callback);

    void Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept override;

private:
    struct TextureWrapper
    {
        Shared<Texture>  texture;
        int32_t          acquired_count;
        std::string      annotation;
        std::vector<DeletionCallback> del_callbacks;
    };

    Unique<TextureFactory>                          factory_;
    std::unordered_map<Texture::TextureId,
                       Unique<TextureWrapper>>      wrappers_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_TEXTUREMANAGER_H
