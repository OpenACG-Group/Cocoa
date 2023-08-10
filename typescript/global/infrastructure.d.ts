interface RuntimeInfo {
    version: string;
    implementation: string;
    platform: string;
}

interface GlobalScope {
    global: GlobalScope;

    __runtime__: RuntimeInfo;
    introspect: Introspect;
}

interface WorkerGlobalScope {
    self: WorkerGlobalScope;
    __runtime__: RuntimeInfo;
}

// TODO(sora): make this available for workers
// declare const self: WorkerGlobalScope;
declare const global: GlobalScope;
declare const __runtime__: RuntimeInfo;

declare function setTimeout(callback: Function, timeout: number, ...argv: any[]): number;
declare function setInterval(callback: Function, interval: number, ...argv: any[]): number;
declare function clearTimeout(id: number);
declare function clearInterval(id: number);

declare function getMillisecondTimeCounter(): number;
