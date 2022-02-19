import * as core from 'core';
introspect.setUnhandledPromiseRejectionHandler((promise, value) => {
    core.print(`${value}\n`);
});
core.print(`${await core.realpath(core.args[0])}\n`);
