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

#ifndef COCOA_GLAMOR_LAYERS_EXTERNALTEXTURELAYER_H
#define COCOA_GLAMOR_LAYERS_EXTERNALTEXTURELAYER_H

#include "Glamor/Layers/Layer.h"
GLAMOR_NAMESPACE_BEGIN

/**
 * External texture accessor.
 */
class ExternalTextureAccessor
{
public:
    virtual ~ExternalTextureAccessor() = default;

    /**
     * Called on rendering thread to determine whether the generated
     * texture is stored in GPU memory.
     */
    virtual bool IsGpuBackedTexture(bool has_gpu_context) = 0;

    /**
     * Called on rendering thread to notify implementor that
     * `Acquire` function will be called soon (in several milliseconds or shorter).
     * For better performance, implementor can check pending asynchronous
     * preprocessing tasks and prepare datas in this call.
     * It is also acceptable to ignore this call and perform all the things
     * in `Acquire()` call.
     */
    virtual void Prefetch() = 0;

    /**
     * Called on rendering thread when the texture is required
     * for drawing. Once an `SkImage` texture is returned, implementor
     * should not own it or modify it anymore, assuming the ownership
     * of `SkImage` is transferred to caller.
     *
     * @note Implementor should make sure the overhead on this call
     * as small as possible (e.g. perform some preprocessing on other threads
     * before this call).
     *
     * @param direct_context    A GPU context which can be used to create GPU backed
     *                          texture. Maybe nullptr if GPU rendering is unavailable.
     *                          This context should only be used for creating texture,
     *                          and must NOT be shared with any other threads.
     */
    virtual sk_sp<SkImage> Acquire(GrDirectContext *direct_context) = 0;

    /**
     * Called on rendering thread, following `Acquire()`,
     * when the texture is not needed anymore. Implementor should
     * release all the dynamically allocated resources in `Acquire()`,
     * and free all the references to GPU resources (if implementor
     * have referenced them since `Acquire` was called).
     *
     * @note This method will NOT be called if the frame is dropped.
     *       Implementor should consider this to avoid latent resources
     *       leaking.
     */
    virtual void Release() = 0;
};

class ExternalTextureLayer : public Layer
{
public:
    using Accessor = ExternalTextureAccessor;

    ExternalTextureLayer(std::unique_ptr<Accessor> frame_accessor,
                         const SkPoint& offset,
                         const SkISize& size,
                         const SkSamplingOptions& sampling);
    ~ExternalTextureLayer() override = default;

    void DiffUpdate(const std::shared_ptr<Layer>& other) override;

    void Preroll(PrerollContext *context, const SkMatrix& matrix) override;
    void Paint(PaintContext *context) override;
    void ToString(std::ostream& out) override;

    const char * GetLayerTypeName() override {
        return "ExternalTextureLayer";
    }

private:
    std::unique_ptr<Accessor> frame_accessor_;

    SkPoint             offset_;
    SkISize             scale_size_;
    SkSamplingOptions   scale_sampling_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_LAYERS_EXTERNALTEXTURELAYER_H
