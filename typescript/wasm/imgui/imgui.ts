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


type f32 = number;
type i32 = number;
type u32 = number;

type TypedArray = Float32Array
                | Int32Array
                | Uint32Array
                | Int16Array
                | Uint16Array
                | Int8Array
                | Uint8Array;

type TypedArrayConstructor<T> =
    T extends Float32Array ?  Float32ArrayConstructor :
    T extends Int32Array ? Int32ArrayConstructor :
    T extends Uint32Array ? Uint32ArrayConstructor :
    T extends Int16Array ? Int16ArrayConstructor :
    T extends Uint16Array ? Uint16ArrayConstructor :
    T extends Int8Array ? Int8ArrayConstructor :
    T extends Uint8Array ? Uint8ArrayConstructor : unknown;

type DrawVerticesFunc = (clipL: f32, clipT: f32, clipR: f32, clipB: f32, pos: Float32Array,
                         uv: Float32Array, colors: Uint32Array, indices: Uint16Array, textureId: number) => void;

export interface ImGuiModule {
    InputEventType: {
        MousePos: InputEventTypeValue<0>;
        MouseButtonLeft: InputEventTypeValue<1>;
        MouseButtonRight: InputEventTypeValue<2>;
        MouseWheel: InputEventTypeValue<3>;
    };

    malloc<T extends TypedArray>(ctor: TypedArrayConstructor<T>, byteSize: number): MallocMemory<T>;
    free<T extends TypedArray>(memory: MallocMemory<T>): void;

    CreateContext(config: JSConfiguration): void;
    QueueInputEvent(eventType: InputEventType, args: Array<any>): void;
    NewFrame(): void;
    Render(drawVerticesCallback: DrawVerticesFunc): void;

    ShowDemoWindow(closable: boolean): boolean;
}

interface InputEventTypeValue<T extends i32> {
    value: T
}
export type InputEventType =
    InputEventTypeValue<0> |
    InputEventTypeValue<1> |
    InputEventTypeValue<2> |
    InputEventTypeValue<3>;

export interface JSConfiguration {
    width: i32;
    height: i32;
    fontSizePx: f32;
    ttfFontData: MallocMemory<Uint8Array>;
    onBuildFontAtlas: (pixels: Uint8Array, w: i32, h: i32) => u32;
}

export interface MallocMemory<T> {
    readonly length: number;
    readonly byteOffset: number;
    subarray(start: number, end: number): T;
    toTypedArray(): T;
}
