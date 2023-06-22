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

export type PropertyFilter = number;
export type PromiseState = number;

interface Constants {
    PROPERTY_FILTER_ALL_PROPERTIES: PropertyFilter;
    PROPERTY_FILTER_ONLY_WRITABLE: PropertyFilter;
    PROPERTY_FILTER_ONLY_ENUMERABLE: PropertyFilter;
    PROPERTY_FILTER_ONLY_CONFIGURABLE: PropertyFilter;
    PROPERTY_FILTER_SKIP_STRINGS: PropertyFilter;
    PROPERTY_FILTER_SKIP_SYMBOLS: PropertyFilter;

    PROMISE_STATE_FULFILLED: PromiseState;
    PROMISE_STATE_PENDING: PromiseState;
    PROMISE_STATE_REJECTED: PromiseState;
}

export const Constants: Constants;

type Enum<T> = T;
type Bitfield<T> = T;

export function IsExternal(value: any): boolean;
export function IsTypedArray(value: any): boolean;
export function IsDate(value: any): boolean;
export function IsArgumentsObject(value: any): boolean;
export function IsBigIntObject(value: any): boolean;
export function IsBooleanObject(value: any): boolean;
export function IsNumberObject(value: any): boolean;
export function IsStringObject(value: any): boolean;
export function IsSymbolObject(value: any): boolean;
export function IsNativeError(value: any): boolean;
export function IsRegExp(value: any): boolean;
export function IsAsyncFunction(value: any): boolean;
export function IsGeneratorFunction(value: any): boolean;
export function IsGeneratorObject(value: any): boolean;
export function IsPromise(value: any): boolean;
export function IsMap(value: any): boolean;
export function IsSet(value: any): boolean;
export function IsMapIterator(value: any): boolean;
export function IsSetIterator(value: any): boolean;
export function IsWeakMap(value: any): boolean;
export function IsWeakSet(value: any): boolean;
export function IsArrayBuffer(value: any): boolean;
export function IsDataView(value: any): boolean;
export function IsSharedArrayBuffer(value: any): boolean;
export function IsProxy(value: any): boolean;
export function IsModuleNamespaceObject(value: any): boolean;
export function IsAnyArrayBuffer(value: any): boolean;
export function IsBoxedPrimitive(value: any): boolean;

export function GetOwnNonIndexProperties(obj: object, filter: Bitfield<PropertyFilter>): string[];
export function GetConstructorName(obj: object): string | null;
export function GetPromiseDetails(promise: Promise<any>): {state: Enum<PromiseState>, result?: any};
export function GetProxyDetails(proxy: object): {target: any, handler: any};
export function PreviewEntries(obj: object): {entries: any[], isKeyValue: boolean};
