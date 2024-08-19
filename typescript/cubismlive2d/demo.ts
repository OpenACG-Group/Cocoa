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

/// <reference path='../third_party/CubismSdk/Core/live2dcubismcore.d.ts'/>

import * as fs from 'fs';
import * as Render from 'renderer';
import * as Present from "present";
import { decodeText, TextCodec } from 'utils';
import { CubismModelSettingJson } from '../third_party/CubismSdk/Framework/src/cubismmodelsettingjson';
import { CubismFramework } from '../third_party/CubismSdk/Framework/src/live2dcubismframework';
import { CubismMoc } from '../third_party/CubismSdk/Framework/src/model/cubismmoc';
import { CubismModel } from '../third_party/CubismSdk/Framework/src/model/cubismmodel';
import { CubismPose } from '../third_party/CubismSdk/Framework/src/effect/cubismpose';
import { CubismPhysics } from '../third_party/CubismSdk/Framework/src/physics/cubismphysics';
import { ModelMeshRenderer } from './model-mesh-renderer';


// Specify where to find our CubismSdk, relative to cwd (`//typescript/out` by default)
const SDK_PATH = '../third_party/CubismSdk';

function LoadJsonSheet<T>(path: string, fn: (ab: ArrayBuffer, size: number) => T): T {
    const readResult = fs.ReadFile(path, 0, null);
    return fn(readResult.buffer.buffer, readResult.readSize);
}

class ModelContext {

    public static Make(modelJsonPath: string): ModelContext {
        const modelPath = modelJsonPath.split('/').slice(0, -1).join('/');

        // Load `.model3.json` file which contains model settings
        const modelSettings = LoadJsonSheet(modelJsonPath, (ab, size) => {
            return new CubismModelSettingJson(ab, size);
        });

        // Load moc file
        let mocPath = modelSettings.getModelFileName();
        if (mocPath.length == 0) {
            throw Error('model settings cannot provide a model file name');
        }
        mocPath = `${modelPath}/${mocPath}`;
        introspect.print(`Load moc from ${mocPath}\n`);

        const moc = CubismMoc.create(fs.ReadFile(mocPath, 0, null).buffer.buffer);
        const model = moc.createModel();

        // Load texture images
        const textures = new Array<Render.Image>();
        for (let i = 0; i < modelSettings.getTextureCount(); i++) {
            const texImageFile = `${modelPath}/${modelSettings.getTextureFileName(i)}`;
            introspect.print(`Load texture #${i} from ${texImageFile}\n`);
            textures.push(Render.Image.DeferredFromEncodedFile(texImageFile, null));
        }

        // Load pose file
        let pose: CubismPose = null;
        let poseJsonFile = modelSettings.getPoseFileName();
        if (poseJsonFile.length != 0) {
            poseJsonFile = `${modelPath}/${poseJsonFile}`;
            introspect.print(`Load pose file from ${poseJsonFile}\n`);
            pose = LoadJsonSheet(poseJsonFile, (ab, size) => {
                return CubismPose.create(ab, size);
            });
        }

        // Load physics file
        let physics: CubismPhysics = null;
        let physicsJsonFile = modelSettings.getPhysicsFileName();
        if (physicsJsonFile.length != 0) {
            physicsJsonFile = `${modelPath}/${physicsJsonFile}`;
            introspect.print(`Load physics file from ${physicsJsonFile}\n`);
            physics = LoadJsonSheet(physicsJsonFile, (ab, size) => {
                return CubismPhysics.create(ab, size);
            });
            physics.initialize();
        }


        // Create mesh renderer
        const meshRenderer = new ModelMeshRenderer(model);
        for (let i = 0; i < textures.length; i++) {
            meshRenderer.bindTexture(i, textures[i]);
        }

        return new ModelContext(modelSettings, moc, model, pose, physics, meshRenderer,
            getMillisecondTimeCounter());
    }

    private constructor(private fSettings: CubismModelSettingJson,
                        private fMoc: CubismMoc,
                        private fModel: CubismModel,
                        private fPose: CubismPose,
                        private fPhysics: CubismPhysics,
                        private fMeshRenderer: ModelMeshRenderer,
                        private fLastTimeMs: number)
    {
    }

    public get width(): number {
        return this.fModel.getCanvasWidth() * this.fModel.getPixelsPerUnit();
    }

    public get height(): number {
        return this.fModel.getCanvasHeight() * this.fModel.getPixelsPerUnit();
    }

    public dispose(): void {
        this.fMoc.deleteModel(this.fModel);
    }

    public drawMesh(canvas: Render.Canvas, scalar: number): void {
        const timeMs = getMillisecondTimeCounter();
        const deltaTimeMs = timeMs - this.fLastTimeMs;
        this.fLastTimeMs = timeMs;

        // Update model
        if (this.fPose != null) {
            this.fPose.updateParameters(this.fModel, deltaTimeMs / 1000);
        }
        if (this.fPhysics != null) {
            this.fPhysics.evaluate(this.fModel, deltaTimeMs / 1000);
        }
        this.fModel.update();

        // Draw model
        const restoreCount = canvas.save();
        canvas.scale(scalar, scalar);
        this.fMeshRenderer.renderToCanvas(canvas);
        canvas.restoreToCount(restoreCount);
    }
}

async function main(modelJsonPath: string): Promise<void> {
    // Load CubismCore (a wasm module) and inject to global scope
    await introspect.scheduleScriptEval(
        decodeText(fs.ReadFile(`${SDK_PATH}/Core/live2dcubismcore.js`, 0, null).buffer, TextCodec.UTF8));

    // Initialize framework
    CubismFramework.startUp();
    CubismFramework.initialize();

    const modelContext = ModelContext.Make(modelJsonPath);

    const bounds = Render.Rect.MakeWH(720, 1280);

    // Create window
    const thread = Present.PresentThread.Start();
    const display = await thread.createDisplay();
    const window = await display.createSurface(bounds.width, bounds.height, true);

    window.addListener('frame', () => {
        const recorder = new Render.PictureRecorder();
        const canvas = recorder.beginRecording(bounds);

        canvas.clear([1, 1, 1, 1]);
        modelContext.drawMesh(canvas, bounds.height / modelContext.height);

        window.contentAggregator.update(
            new Present.SceneBuilder(bounds)
                .addPicture(recorder.finishRecordingAsPicture(), true)
                .build()
        );
    });

    window.addListener('close', () => {
        modelContext.dispose();
        window.removeAllListeners('frame');
        window.close().then(() => {
            return display.close();
        }).then(() => {
            thread.dispose();
        });
    });

    // Start draw
    await window.requestNextFrame();
}

await main(`${SDK_PATH}/Samples/Resources/Hiyori/Hiyori.model3.json`);
