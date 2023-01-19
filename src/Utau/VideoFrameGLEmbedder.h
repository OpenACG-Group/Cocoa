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

#ifndef COCOA_UTAU_GLEXTENSIONVIDEOLAYER_H
#define COCOA_UTAU_GLEXTENSIONVIDEOLAYER_H

#define FFWRAP_AVUTIL_USE_HWCONTEXT_VAAPI

#include <unordered_map>

#include "Glamor/Layers/ExternalTextureLayer.h"
#include "Utau/Utau.h"
#include "Utau/ffwrappers/libavutil.h"
UTAU_NAMESPACE_BEGIN

class VideoBuffer;

class VideoFrameGLEmbedder
{
public:
    struct SwscaleContextCache;

    VideoFrameGLEmbedder();
    ~VideoFrameGLEmbedder();

    /**
     * Called from rendering thread to wrap a video buffer into an
     * SkImage with possible GPU context associated with it.
     * Images are cached if they are VAAPI frames.
     */
    std::unique_ptr<gl::ExternalTextureLayer> Commit(const std::shared_ptr<VideoBuffer>& buffer,
                                                     const SkPoint& offset,
                                                     const SkISize& size,
                                                     const SkSamplingOptions& sampling);

    sk_sp<SkImage> ConvertToRasterImage(const std::shared_ptr<VideoBuffer>& buffer);

    g_private_api g_nodiscard g_inline
    SwscaleContextCache *GetSwsContextCache() const {
        return sws_context_cache_.get();
    }

private:
    std::unique_ptr<SwscaleContextCache> sws_context_cache_;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_GLEXTENSIONVIDEOLAYER_H
