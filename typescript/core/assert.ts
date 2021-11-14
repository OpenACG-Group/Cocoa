import * as core from 'core';

export type CallerT = Function;

export class CallTracker {
    #stacktrace: CallerT[];
    constructor() {
        let caller = CallTracker.caller;
        while (caller != null) {
            this.#stacktrace.push(caller);
            caller = caller.caller;
        }
    }

    depth(): number {
        return this.#stacktrace.length;
    }

    forEach(visitor: (caller: CallerT) => void): void {
        for (let caller of this.#stacktrace) {
            visitor(caller);
        }
    }

    toStringArray(): string[] {
        let result: string[] = [];
        let depth = 0;
        for (let caller of this.#stacktrace) {
            result.push(`#${depth} ${caller.name}`);
            depth++;
        }
        return result;
    }
}

export function assert(value: boolean, message?: string): void {
    core.exit();
}
