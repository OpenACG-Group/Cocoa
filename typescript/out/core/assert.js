var __classPrivateFieldGet = (this && this.__classPrivateFieldGet) || function (receiver, state, kind, f) {
    if (kind === "a" && !f) throw new TypeError("Private accessor was defined without a getter");
    if (typeof state === "function" ? receiver !== state || !f : !state.has(receiver)) throw new TypeError("Cannot read private member from an object whose class did not declare it");
    return kind === "m" ? f : kind === "a" ? f.call(receiver) : f ? f.value : state.get(receiver);
};
var _CallTracker_stacktrace;
import * as core from 'core';
export class CallTracker {
    constructor() {
        _CallTracker_stacktrace.set(this, void 0);
        let caller = CallTracker.caller;
        while (caller != null) {
            __classPrivateFieldGet(this, _CallTracker_stacktrace, "f").push(caller);
            caller = caller.caller;
        }
    }
    depth() {
        return __classPrivateFieldGet(this, _CallTracker_stacktrace, "f").length;
    }
    forEach(visitor) {
        for (let caller of __classPrivateFieldGet(this, _CallTracker_stacktrace, "f")) {
            visitor(caller);
        }
    }
    toStringArray() {
        let result = [];
        let depth = 0;
        for (let caller of __classPrivateFieldGet(this, _CallTracker_stacktrace, "f")) {
            result.push(`#${depth} ${caller.name}`);
            depth++;
        }
        return result;
    }
}
_CallTracker_stacktrace = new WeakMap();
export function assert(value, message) {
    core.exit();
}
