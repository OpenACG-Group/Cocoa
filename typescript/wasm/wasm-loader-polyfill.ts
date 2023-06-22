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

import * as std from 'core';

function executeScript<T>(wasmBinary: ArrayBuffer, script: string): Promise<T> {
    let readyPromiseResolver: (v: any) => void = null;
    let readyPromiseReject: (v: any) => void = null;

    const readyPromise = new Promise<T>((resolve, reject) => {
        readyPromiseResolver = resolve;
        readyPromiseReject = reject;
    });

    // Emscripten uses `Module` object to interact with user's code,
    // and the exported objects will also be mounted on `Module`.
    const Module = {
        onRuntimeInitialized() {
            readyPromiseResolver(Module);
        }
    };

    // To emulate Web environment
    const window = {};
    const performance = {
        now() {
            return getMillisecondTimeCounter();
        }
    };

    // WebAssembly binary code will be loaded by this function.
    // It just provides a dummy implement of `fetch()` function
    // in Web environment.
    const fetch = () => {
        return Promise.resolve({
            ok: true,
            arrayBuffer() {
                return Promise.resolve(wasmBinary);
            }
        });
    };

    // Now executes the script to load and initialize the WebAssembly module.
    const f = new Function('Module', 'window', 'fetch', 'performance', script);
    f(Module, window, fetch, performance);

    return readyPromise;
}

export async function Load<T>(wasmBinary: ArrayBuffer, script: string): Promise<T> {
    return executeScript(wasmBinary, script);
}

export async function LoadFromFile<T>(wasmPath: string, scriptPath: string): Promise<T> {
    const wasmBuffer = std.File.ReadFileSync(wasmPath);
    const scriptBuffer = std.File.ReadFileSync(scriptPath);

    const scriptText = scriptBuffer.toString(std.Buffer.ENCODE_UTF8, scriptBuffer.length);

    return executeScript(wasmBuffer.byteArray.buffer, scriptText);
}

export async function LoadFromProjectThirdParty<T>(wasmName: string, scriptName: string): Promise<T> {
    const dir = await std.realpath(
        import.meta.url.replace(/(file:\/\/)|\/[^\/]+$/g, '')
        + '/../../../third_party/wasm-build/bin'
    );
    return LoadFromFile<T>(`${dir}/${wasmName}`, `${dir}/${scriptName}`);
}
