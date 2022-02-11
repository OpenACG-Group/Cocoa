/*
import {printf} from 'core/format';
import {delay, print} from 'core';
import * as Gsk from 'gsk';

introspect.setUnhandledPromiseRejectionHandler((promise, value) => {
    printf('Reject: {}\n', value.toString());
});

async function run() {
    let display: Gsk.Display = Gsk.Connect();
    print('opened\n');
    if (display.monitorsCount >= 1) {
        let monitor: Gsk.Monitor = display.getMonitor(0);
        printf('monitor[0]: manufacturer={}, model={}, connector={}, {}x{}\n',
            monitor.manufacturer, monitor.model, monitor.connector,
            monitor.geometryWidth, monitor.geometryHeight);
    }

    await delay(3000);
    display.close();
}

run();
*/

import {File, Buffer, print, open, args} from "core";

const kBufferSize: number = 4096;

async function main() {
    if (args.length != 2) {
        throw Error('Need an option to specify a file to read');
    }

    let st = getMillisecondTimeCounter();

    let src = await open(args[0], File.O_RDONLY, 0);
    let srcStat = await src.fstat();

    let dst = await open(args[1], File.O_WRONLY | File.O_CREAT, srcStat.mode);

    let buf = [new Buffer(kBufferSize), new Buffer(kBufferSize)];

    let p = 0;
    let readSize = await src.read(buf[p], 0, kBufferSize, 0);
    let pos_r = readSize, pos_w = 0;
    while (readSize > 0)
    {
        let pw = dst.write(buf[p], 0, readSize, pos_w);
        let pr = src.read(buf[p ^ 1], 0, kBufferSize, pos_r);

        readSize = await pr;
        pos_r += readSize;
        pos_w += await pw;
        p ^= 1;
    }

    await src.close();
    await dst.close();

    let et = getMillisecondTimeCounter();
    print(`Read ${srcStat.size} bytes in ${et - st} ms\n`);
}

main().catch(reason => {
    print(`error: ${reason}\n`)
});
