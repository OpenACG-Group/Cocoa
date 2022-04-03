interface RuntimeInfo {
    version: string;
    implementation: string;
    platform: string;
}

interface Global {
    /**
     * This field and the __global__ variable declared globally
     * refer to the same Global object, which means this field
     * in Global object refers to the object itself.
     */
    __global__: Global;

    __runtime__: RuntimeInfo;
    introspect: Introspect;
}

declare let __global__: Global;
declare let __runtime__: RuntimeInfo;

declare function setTimeout(callback: Function, timeout: number, ...argv: any[]): number;
declare function setInterval(callback: Function, interval: number, ...argv: any[]): number;
declare function clearTimeout(id: number);
declare function clearInterval(id: number);

declare function getMillisecondTimeCounter(): number;
