import * as core from 'core';
function stringifyReplacer(key, value) {
    if (typeof value == 'function') {
        if (value.name.length == 0) {
            return '[Function: <anonymous>]';
        }
        return `[Function: ${value.name}]`;
    }
    return value;
}
export function print(value) {
    core.print(`${JSON.stringify(value, stringifyReplacer, 2)}\n`);
}
