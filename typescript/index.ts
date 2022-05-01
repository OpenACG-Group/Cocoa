import * as std from 'core';
import * as cocanvas from 'core/cocanvas';
import {print} from "core";

let ctx = new cocanvas.DrawingContext();
let canvas = new cocanvas.Canvas(ctx, 800, 600);

canvas.test();
canvas.finish();
// std.print(canvas.disassemble());

let s = getMillisecondTimeCounter();
canvas.submit();
let e = getMillisecondTimeCounter();
print(`time: ${e - s} ms\n`);

std.print('Done\n');
