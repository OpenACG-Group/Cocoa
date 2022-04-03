import { print, exit } from "core";
let kStackTraceFrameLimitation = 16;
export function adjustStackTraceFramesLimitation(value) {
    if (value < 0 || !Number.isInteger(value)) {
        throw TypeError('Stacktrace limitation should be a non-negative integer');
    }
    kStackTraceFrameLimitation = value;
}
function printStacktrace(stacktrace) {
    let depth = 1;
    for (let frame of stacktrace) {
        print(`  #${depth++} at `);
        if (frame.isConstructor) {
            print('new ');
        }
        print(`${frame.functionName} (${frame.scriptName}`);
        if (frame.line > 0 && frame.column > 0) {
            print(`:${frame.line}:${frame.column}`);
        }
        print(')\n');
    }
}
export function assert(expr, message) {
    if (expr)
        return;
    print('Assertion Failed: ');
    if (message != undefined) {
        print(`${message}:`);
    }
    print('\n');
    let stacktrace = introspect.inspectStackTrace(kStackTraceFrameLimitation);
    printStacktrace(stacktrace);
    /* Exit immediately without waiting for event loop */
    exit();
}
