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

import * as gl from 'glamor';
import { print } from 'core';

const presentThread = await gl.PresentThread.Start();
const display = await presentThread.createDisplay();
const surface = await display.createHWComposeSurface(800, 600);
const blender = await surface.createBlender();

const context = gl.GpuDirectContext.Make();
const semaphore = context.makeBinarySemaphore();

const id = await blender.importGpuSemaphoreFd(context.exportSemaphoreFd(semaphore));
print(`Exported semaphore to onscreen context, ID=${id}\n`);

await blender.deleteImportedGpuSemaphore(id);
semaphore.dispose();
context.dispose();

await blender.dispose();
await surface.close();
await display.close();

presentThread.dispose();