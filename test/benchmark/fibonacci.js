import {print} from 'core';

introspect.setUnhandledPromiseRejectionHandler((promise, reason) => {
    introspect.print(`${reason}\n`);
});

function fib(x) {
    if (x == 1 || x == 2) {
        return 1;
    }
    return fib(x - 1) + fib(x - 2);
}

let st = getMillisecondTimeCounter();

let result = fib(40);

let et = getMillisecondTimeCounter();

print(`result = ${result} in ${et - st} ms\n`);
