export interface ScripterInfo {
    version: number[];
    manufacture: string;
    capabilities: string[];
}

interface Core {
    /**
     * Invoke an Op (enter native code) synchronously or asynchronously.
     *
     * @param op Name of Op that will be invoked.
     * @param args A JSON object that contains the arguments.
     * @return number >=0 if success, <0 if failed.
     */
    opCall(op: string, args: object): number|Promise<number>;

    /**
     * Get information and capabilities of JavaScript engine.
     */
    getScripterInfo(): ScripterInfo;
}

export let Core: Core;
