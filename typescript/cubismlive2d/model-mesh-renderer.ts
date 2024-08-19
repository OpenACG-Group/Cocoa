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

import * as Render from 'renderer';
import { CubismModel } from "../third_party/CubismSdk/Framework/src/model/cubismmodel";
import { CubismBlendMode } from "../third_party/CubismSdk/Framework/src/rendering/cubismrenderer";

interface OrderedDrawableIndex {
    index: number;
    renderOrder: number;
}

interface TextureInfo {
    image: Render.Image;
    shader: Render.Shader;
    matrix: Render.Mat3x3;
}

const kAdditiveBlenderSkSL = `
vec4 main(vec4 src, vec4 dst) {
    vec4 c = dst + src;
    return vec4(c.r, c.g, c.b, dst.a);
}
`;

const kMultBlenderSkSL = `
vec4 main(vec4 src, vec4 dst) {
    vec4 c = dst * (1.0 - src.a) + src * dst;
    return vec4(c.r, c.g, c.b, dst.a);
}
`;

enum SkSLProgramSelector {
    kAdditiveBlender = 0,
    kMultiplicationBlender = 1,

    kLast = kMultiplicationBlender
}

const SkSLProgramSources = [
    kAdditiveBlenderSkSL,
    kMultBlenderSkSL
];

interface TransformedDrawableInfo {
    vertices: Render.Vertices;
    shader: Render.Shader;
}

export class ModelMeshRenderer {
    private fTextures: Map<number, TextureInfo>;
    private fModel: CubismModel;
    private fSkSLPrograms: Array<Render.Blender>;
    private fModelCoordCTM: Render.Mat3x3;
    private fMaskSurface: Render.Surface;
    private fCanvasWidth: number;
    private fCanvasHeight: number;

    constructor(model: CubismModel) {
        this.fTextures = new Map<number, TextureInfo>();
        this.fModel = model;
        this.fSkSLPrograms = new Array<Render.Blender>(SkSLProgramSelector.kLast + 1);
        this.fMaskSurface = null;
        this.fCanvasWidth = model.getCanvasWidth() * model.getPixelsPerUnit();
        this.fCanvasHeight = model.getCanvasHeight() * model.getPixelsPerUnit();

        const ppu = model.getPixelsPerUnit();
        const ox = model.getModel().canvasinfo.CanvasOriginX;
        const oy = model.getModel().canvasinfo.CanvasOriginY;
        this.fModelCoordCTM = Render.Mat3x3.Concat(Render.Mat3x3.Translate(ox, oy), Render.Mat3x3.Scale(ppu, -ppu));
    }

    private compileRuntimeBlender(source: string): Render.Blender {
        const [effect, error] = Render.RuntimeEffect.CompileBlender(source);
        if (!effect) {
            throw Error(`Failed to compile runtime blender: ${error}`);
        }
        return effect.makeBlender(null, []);
    }

    private getSkSLProgram(selector: SkSLProgramSelector): Render.Blender {
        if (this.fSkSLPrograms[selector]) {
            return this.fSkSLPrograms[selector];
        }
        this.fSkSLPrograms[selector] = this.compileRuntimeBlender(SkSLProgramSources[selector]);
        return this.fSkSLPrograms[selector];
    }

    public bindTexture(index: number, image: Render.Image): void {
        const matrix = Render.Mat3x3.Concat(Render.Mat3x3.Translate(0, image.height),
            Render.Mat3x3.Scale(image.width, -image.height));
        this.fTextures.set(index, {
            image: image,
            shader: image.makeShader(Render.TileMode.Clamp, Render.TileMode.Clamp,
                { useCubic: false, filter: Render.FilterMode.Linear }, null),
            matrix: matrix
        });
    }

    private sortDrawableRenderOrders(): Array<OrderedDrawableIndex> {
        const count = this.fModel.getDrawableCount();
        const result = new Array<OrderedDrawableIndex>(count);
        const renderOrders = this.fModel.getDrawableRenderOrders();
        for (let i = 0; i < count; i++) {
            result[i] = { index: i, renderOrder: renderOrders[i] };
        }

        result.sort((a, b) => a.renderOrder - b.renderOrder);
        return result;
    }

    private transformDrawable(dwIndex: number): TransformedDrawableInfo {
        const textureIndex = this.fModel.getDrawableTextureIndex(dwIndex);
        const texture = this.fTextures.get(textureIndex);
        if (texture === undefined) {
            throw Error(`Invalid texture index #${textureIndex}: texture is not bound`);
        }

        const vertNorm = this.fModel.getDrawableVertexPositions(dwIndex);
        const uvNorm = this.fModel.getDrawableVertexUvs(dwIndex);

        const verts = new Float32Array(vertNorm.length);
        const uvs = new Float32Array(uvNorm.length);
        for (let v = 0; v < vertNorm.length / 2; v++) {
            const coord = this.fModelCoordCTM.map(vertNorm[v*2], vertNorm[v*2+1], 1);
            verts[v * 2] = coord.x;
            verts[v * 2 + 1] = coord.y;

            const uvCoord = texture.matrix.map(uvNorm[v * 2], uvNorm[v * 2 + 1], 1);
            uvs[v * 2] = uvCoord.x;
            uvs[v * 2 + 1] = uvCoord.y;
        }

        return {
            vertices: Render.Vertices.MakeCopy(
                Render.VertexMode.Triangles,
                verts,
                uvs,
                null,
                this.fModel.getDrawableVertexIndices(dwIndex)
            ),
            shader: texture.shader
        };
    }

    public renderMaskShader(dwIndex: number, dwInfo: TransformedDrawableInfo[]): Render.Shader {
        const maskCount = this.fModel.getDrawableMaskCounts()[dwIndex];
        if (maskCount == 0) {
            return null;
        }

        const masks = this.fModel.getDrawableMasks()[dwIndex];

        if (this.fMaskSurface == null) {
            this.fMaskSurface = Render.Surface.MakeRaster(
                Render.ImageInfo.MakeA8(this.fCanvasWidth, this.fCanvasHeight),
                null
            );
        }
        const canvas = this.fMaskSurface.canvas;

        const paint = new Render.Paint();
        paint.alphaf = 1.0;
        paint.blendMode = Render.BlendMode.SrcOver;
        for (let i = 0; i < maskCount; i++) {
            if (masks[i] < 0 || !this.fModel.getDrawableDynamicFlagVertexPositionsDidChange(masks[i])) {
                continue;
            }
            canvas.drawVertices(dwInfo[masks[i]].vertices, Render.BlendMode.SrcOver, paint);
        }

        const image = this.fMaskSurface.makeImageSnapshot(null);
        const shader = image.makeShader(
            Render.TileMode.Clamp, Render.TileMode.Clamp,
            { useCubic: false, filter: Render.FilterMode.Linear },
            null
        );
        image.dispose();

        return shader;
    }

    public renderToCanvas(canvas: Render.Canvas): void {
        const drawableCount = this.fModel.getDrawableCount();

        // Build drawable info using transformed positions
        const dwInfo: TransformedDrawableInfo[] = [];
        for (let d = 0; d < drawableCount; d++) {
            dwInfo.push(this.transformDrawable(d));
        }

        // Draw
        const sortedDrawables = this.sortDrawableRenderOrders();
        for (let i = 0; i < drawableCount; i++) {
            const dwIndex = sortedDrawables[i].index;

            const opacity = this.fModel.getDrawableOpacity(dwIndex);
            // fastpath: discard completely opaque drawables
            if (opacity == 0) {
                continue;
            }

            const paint = new Render.Paint();
            paint.shader = dwInfo[dwIndex].shader;
            paint.alphaf = opacity;

            const blendMode = this.fModel.getDrawableBlendMode(dwIndex);
            if (blendMode == CubismBlendMode.CubismBlendMode_Normal) {
                paint.blendMode = Render.BlendMode.SrcOver;
            } else if (blendMode == CubismBlendMode.CubismBlendMode_Additive) {
                paint.blender = this.getSkSLProgram(SkSLProgramSelector.kAdditiveBlender);
            } else if (blendMode == CubismBlendMode.CubismBlendMode_Multiplicative) {
                paint.blender = this.getSkSLProgram(SkSLProgramSelector.kMultiplicationBlender);
            }

            canvas.save();

            const mask = this.renderMaskShader(dwIndex, dwInfo);
            if (mask != null) {
                const inverted = this.fModel.getDrawableInvertedMaskBit(dwIndex);
                canvas.clipShader(mask, inverted ? Render.ClipOp.Difference : Render.ClipOp.Intersect);
                mask.dispose();
            }

            // TODO(sora): process culling

            canvas.drawVertices(dwInfo[dwIndex].vertices, Render.BlendMode.SrcOver, paint);
            canvas.restore();
        }
    }
}
