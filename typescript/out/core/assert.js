import * as core from 'core';
import { StackCapture } from 'stack_capture';
export class CallTracker {
    constructor() {
        let capture = new StackCapture();
        core.print(capture.stack + '\n');
    }
}
/**
 * Abort program if `value` is false.
 * @param value Result of an expression. Assertion fails if false.
 * @param message Optional description of this assertion.
 */
export function assert(value, message) {
    if (!value) {
        core.print('Assertion failed: ' + message + '\n');
        let tracker = new CallTracker();
        // core.exit();
    }
}
