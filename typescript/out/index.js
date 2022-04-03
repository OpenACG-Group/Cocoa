import * as std from 'core';
introspect.setUnhandledPromiseRejectionHandler(((promise, value) => {
    std.print(`Error: ${value}\n`);
}));
const ARTIFACT_DIR = `/tmp/in-place-load-artifact`;
const ARTIFACT_SO_FILE = `${ARTIFACT_DIR}/libArtifact.so`;
async function cleanup() {
    if (await std.access(ARTIFACT_SO_FILE, std.File.F_OK) == 0)
        await std.unlink(ARTIFACT_SO_FILE);
    if (await std.access(ARTIFACT_DIR, std.File.F_OK) == 0)
        await std.rmdir(ARTIFACT_DIR);
}
async function main() {
    if (std.args.length != 2) {
        std.print('Usage: [Target.o file] [cpp source file]');
        return;
    }
    await std.mkdir(ARTIFACT_DIR, std.File.S_IWUSR | std.File.S_IRUSR | std.File.S_IXUSR);
    let targetCxaRT = std.args[0], cppSource = std.args[1];
    let status = await std.fork({
        file: '/usr/bin/clang',
        args: ['-std=c++17', targetCxaRT, cppSource, '-fPIC', '-shared', '-o', ARTIFACT_SO_FILE]
    }).promiseOnExit();
    if (status.status != 0) {
        std.print('Failed in compilation\n');
        await cleanup();
        return;
    }
    introspect.loadSharedObject(ARTIFACT_SO_FILE);
    let module = await import('artifact');
    module.__trampoline();
    await cleanup();
}
await main();
