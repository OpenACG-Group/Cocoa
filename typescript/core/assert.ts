import * as core from 'core';
import {StackCapture} from 'stack_capture';

export type CallerT = Function;

export class CallTracker {
    constructor() {
        let capture: any = new StackCapture();
        core.print(capture.stack + '\n');
    }
}

/**
 * Abort program if `value` is false.
 * @param value Result of an expression. Assertion fails if false.
 * @param message Optional description of this assertion.
 */
export function assert(value: boolean, message?: string): void {
    if (!value) {
        core.print('Assertion failed: ' + message + '\n');
        let tracker = new CallTracker();
        // core.exit();
    }
}
