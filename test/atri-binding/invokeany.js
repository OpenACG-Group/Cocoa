import {args, print} from 'core';

async function main() {
    if (args.length != 3) {
        print('Usage: <library> <binding name> <func>\n');
        return;
    }

    introspect.loadSharedObject(args[0]);
    (await import(args[1]))[args[2]]();
}

main().catch(reason => {
    print(`promise rejection: ${reason}\n`);
});
