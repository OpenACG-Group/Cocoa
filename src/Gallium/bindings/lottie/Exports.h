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

#ifndef COCOA_GALLIUM_BINDINGS_LOTTIE_EXPORTS_H
#define COCOA_GALLIUM_BINDINGS_LOTTIE_EXPORTS_H

#include "include/v8.h"
#include "modules/skottie/include/Skottie.h"

#include "Core/Project.h"

#define GALLIUM_BINDINGS_LOTTIE_NS_BEGIN    namespace cocoa::gallium::bindings::lottie_wrap {
#define GALLIUM_BINDINGS_LOTTIE_NS_END      }


GALLIUM_BINDINGS_LOTTIE_NS_BEGIN

void SetInstanceProperties(v8::Local<v8::Object> instance);

//! TSDecl: class AnimationBuilder
class AnimationBuilderWrap
{
public:
    //! TSDecl: constructor(flags: Enum<AnimationBuilderFlags>)
    explicit AnimationBuilderWrap(uint32_t flags);

    ~AnimationBuilderWrap() = default;

    //! TSDecl: function setResourceProvider(rp: ResourceProvider): AnimationBuilder
    v8::Local<v8::Value> setResourceProvider(v8::Local<v8::Value> rp);

    //! TSDecl: function setFontManager(mgr: GL.CkFontMgr): AnimationBuilder
    v8::Local<v8::Value> setFontManager(v8::Local<v8::Value> mgr);

    //! TSDecl: type Logger = (level: Enum<LoggerLevel>, message: string, json: string | null) => void

    //! TSDecl: function setLogger(func: Logger): AnimationBuilder
    v8::Local<v8::Value> setLogger(v8::Local<v8::Value> func);

    //! TSDecl: type MarkerObserver = (name: string, t0: number, t1: number) => void

    //! TSDecl: function setMarkerObserver(func: MarkerObserver): AnimationBuilder
    v8::Local<v8::Value> setMarkerObserver(v8::Local<v8::Value> func);

    //! TSDecl: type ExternalLayerRenderFunc = (canvas: GL.CkCanvas, t: number) => void
    //! TSDecl: type PrecompInterceptorFunc = (id: string, name: string,
    //!                                        width: number, height: number) => ExternalLayerRenderFunc

    //! TSDecl: function setPrecompInterceptor(func: PrecompInterceptorFunc): AnimationBuilder
    v8::Local<v8::Value> setPrecompInterceptor(v8::Local<v8::Value> func);

    //! TSDecl: interface IExpressionManager {
    //!   createNumberExpressionEvaluator(expr: string): (t: number) => number;
    //!   createStringExpressionEvaluator(expr: string): (t: number) => string;
    //!   createArrayExpressionEvaluator(expr: string): (t: number) => Array<number>;
    //! }

    //! TSDecl: function setExpressionManager(manager: IExpressionManager): AnimationBuilder
    v8::Local<v8::Value> setExpressionManager(v8::Local<v8::Value> manager);

    //! TSDecl: function make(json: string): Animation
    v8::Local<v8::Value> make(v8::Local<v8::Value> json);

    //! TSDecl: function makeFromFile(path: string): Animation
    v8::Local<v8::Value> makeFromFile(const std::string& path);

private:
    v8::Local<v8::Object> ReturnThis();

    skottie::Animation::Builder builder_;
};

//! TSDecl: class Animation
class AnimationWrap
{
public:
    explicit AnimationWrap(sk_sp<skottie::Animation> animation)
        : animation_(std::move(animation)) {}

    ~AnimationWrap() = default;

    //! TSDecl: function render(canvas: GL.CkCanvas, dst: GL.CkRect | null,
    //!                         flags: Bitfield<AnimationRenderFlag>): void
    void render(v8::Local<v8::Value> canvas, v8::Local<v8::Value> dst, uint32_t flags);

    //! TSDecl: function seekFrame(t: number): void
    void seekFrame(double t);

    //! TSDecl: function seekFrameTime(t: number): void
    void seekFrameTime(double t);

    //! TSDecl: readonly duration: number
    g_nodiscard g_inline double getDuration() const {
        return animation_->duration();
    }

    //! TSDecl: readonly fps: number
    g_nodiscard g_inline double getFps() const {
        return animation_->fps();
    }

    //! TSDecl: readonly inPoint: number
    g_nodiscard g_inline double getInPoint() const {
        return animation_->inPoint();
    }

    //! TSDecl: readonly outPoint: number
    g_nodiscard g_inline double getOutPoint() const {
        return animation_->outPoint();
    }

private:
    sk_sp<skottie::Animation> animation_;
};

GALLIUM_BINDINGS_LOTTIE_NS_END
#endif //COCOA_GALLIUM_BINDINGS_LOTTIE_EXPORTS_H
