import * as std from 'core'

std.print('User hello!\n');

function foo() {
    throw Error('Fuck error');
}

std.print(`${JSON.stringify(getGalliumRuntimeInfo())}\n`);

foo();

std.print('continue?\n');
