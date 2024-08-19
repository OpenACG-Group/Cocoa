type i8 = number;
type u8 = number;
type i16 = number;
type u16 = number;
type i32 = number;
type u32 = number;
type i64 = number;
type u64 = number;
type f32 = number;
type f64 = number;
export declare enum Constants {
    PROPERTY_FILTER_ALL_PROPERTIES,
    PROPERTY_FILTER_ONLY_WRITABLE,
    PROPERTY_FILTER_ONLY_ENUMERABLE,
    PROPERTY_FILTER_ONLY_CONFIGURABLE,
    PROPERTY_FILTER_SKIP_STRINGS,
    PROPERTY_FILTER_SKIP_SYMBOLS,
    PROMISE_STATE_FULFILLED,
    PROMISE_STATE_PENDING,
    PROMISE_STATE_REJECTED,
}
export interface PromiseDetails {
    state: u32;
    result?: any;
}
export interface ProxyDetails {
    target: any;
    handler: any;
}
export interface EntriesInfo {
    entries: any[];
    isKeyValue: boolean;
}
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
export function IsAnyArrayBuffer(value: any): boolean;
export function GetOwnNonIndexProperties(obj: object, filter: u32): string[];
export function GetConstructorName(obj: object): string;
export function GetPromiseDetails(promise: Promise<any>): PromiseDetails;
export function GetProxyDetails(proxy: object): ProxyDetails;
export function PreviewEntries(obj: object): EntriesInfo;
