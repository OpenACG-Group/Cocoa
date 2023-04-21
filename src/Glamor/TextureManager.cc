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

#include <utility>

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Glamor/TextureManager.h"
#include "Glamor/TextureFactory.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.TextureManager)

TextureManager::TextureManager(Unique<TextureFactory> factory)
    : factory_(std::move(factory))
{
}

TextureManager::~TextureManager()
{
    // Delete textures
    for (auto& wrapper_pair : wrappers_)
    {
        CHECK(wrapper_pair.second->acquired_count == 0);
        for (const auto& callback : wrapper_pair.second->del_callbacks)
            callback();

        CHECK(wrapper_pair.second->texture.unique());
        wrapper_pair.second->texture.reset();
        wrapper_pair.second.reset();
    }
    wrappers_.clear();
}

std::optional<Texture::TextureId> TextureManager::Create(const CreateCallback& callback,
                                                         const std::string_view& annotation)
{
    CHECK(callback);

    Shared<Texture> shared_texture = callback(factory_);
    if (!shared_texture)
    {
        // Factory could not create a texture
        return {};
    }

    Texture::TextureId id = shared_texture->GetUniqueId();

    QLOG(LOG_DEBUG, "Created texture \"{}\", object={} id={}", annotation,
         fmt::ptr(shared_texture.get()), id);

    auto wrapper = std::make_unique<TextureWrapper>();
    wrapper->texture = shared_texture;
    wrapper->acquired_count = 0;
    wrapper->annotation = annotation;
    wrapper->del_callbacks = {};

    wrappers_[id] = std::move(wrapper);

    return id;
}

bool TextureManager::Delete(Texture::TextureId id)
{
    if (wrappers_.count(id) == 0)
    {
        QLOG(LOG_WARNING, "Try deleting texture object id={}, "
                          "which refers to an invalid texture", id);
        return false;
    }

    auto& wrapper = wrappers_[id];

    if (wrapper->acquired_count > 0)
    {
        QLOG(LOG_WARNING, "Try deleting an acquired texture object id={}, refused", id);
        return false;
    }

    for (const auto& callback : wrapper->del_callbacks)
        callback();

    CHECK(wrapper->texture.unique());
    wrapper->texture.reset();

    wrappers_.erase(id);
    return true;
}

Texture *TextureManager::AcquireTexture(Texture::TextureId id)
{
    if (wrappers_.count(id) == 0)
    {
        QLOG(LOG_WARNING, "Try acquiring an invalid texture object id={}", id);
        return nullptr;
    }

    auto& wrapper = wrappers_[id];
    wrapper->acquired_count++;

    return wrapper->texture.get();
}

void TextureManager::ReleaseTexture(Texture *texture)
{
    Texture::TextureId id = texture->GetUniqueId();
    if (wrappers_.count(texture->GetUniqueId()) == 0)
    {
        QLOG(LOG_WARNING, "Try releasing an invalid texture object id={}", id);
        return;
    }

    auto& wrapper = wrappers_[id];
    if (wrapper->acquired_count == 0)
    {
        QLOG(LOG_WARNING, "Try releasing a not acquired texture object id={}, refused", id);
        return;
    }

    wrapper->acquired_count--;
}

void TextureManager::SubscribeTextureDeletion(Texture::TextureId id,
                                              const DeletionCallback& callback)
{
    if (wrappers_.count(id) == 0)
        return;
    wrappers_[id]->del_callbacks.push_back(callback);
}

void TextureManager::Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept
{
    int32_t index = 0;
    for (const auto& wrapper : wrappers_)
    {
        auto note = fmt::format("Texture#{} [{}]", index++, wrapper.second->annotation);
        tracer->TraceMember(note, wrapper.second->texture.get());
    }
}

GLAMOR_NAMESPACE_END
