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

#ifndef COCOA_GLAMOR_LAYERS_GPUSURFACEVIEWLAYER_H
#define COCOA_GLAMOR_LAYERS_GPUSURFACEVIEWLAYER_H

#include "Glamor/Layers/Layer.h"
GLAMOR_NAMESPACE_BEGIN

class GpuSurfaceViewLayer : public Layer
{
public:
    class ContentTracker
    {
    public:
        explicit ContentTracker(sk_sp<SkSurface> surface);
        ~ContentTracker() = default;

        /**
         * Copy constructor. When a ContentTracker is copied, the new object forks
         * the current state of original object. They have independent states that
         * do not affect each other.
         */
        ContentTracker(const ContentTracker& other) = default;

        /**
         * Compare the current surface content state with the recorded state, and then
         * update the recorded state to the current state. The result of the last
         * comparison can be got by `HasChangedSinceLastTrackPoint()` method.
         */
        void UpdateTrackPoint();

        g_nodiscard bool HasChangedSinceLastTrackPoint() const;

    private:
        sk_sp<SkSurface>    surface_;
        int64_t             last_track_point_;
        bool                has_changed_;
    };

    GpuSurfaceViewLayer(int64_t surface_id, SkRect dst_rect,
                        int64_t wait_semaphore_id, int64_t signal_semaphore_id,
                        ContentTracker *content_tracker);

    ~GpuSurfaceViewLayer() override = default;

    void Preroll(PrerollContext *context, const SkMatrix& matrix) override;
    void Paint(PaintContext *context) override;
    void DiffUpdate(const std::shared_ptr<Layer>& other) override;
    void ToString(std::ostream& out) override;

    const char *GetLayerTypeName() override {
        return "GpuSurfaceViewLayer";
    }

private:
    int64_t         surface_id_;
    SkRect          dst_rect_;
    int64_t         wait_semaphore_id_;
    int64_t         signal_semaphore_id_;
    bool            content_changed_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_LAYERS_GPUSURFACEVIEWLAYER_H
