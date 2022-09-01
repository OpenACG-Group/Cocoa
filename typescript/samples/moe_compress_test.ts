import * as std from 'core';
import * as canvaskit from '../canvaskit/canvaskit';

let rGroup = new canvaskit.MemoryResourceGroup();
let canvas = new canvaskit.Canvas(rGroup, 800, 600);

const R = 115.2, C = 128.0;
let path = new canvaskit.Path(canvas);
path.moveTo(C + R, C);
for (let i = 1; i < 8; i++) {
    let a = 2.6927937 * i;
    path.lineTo(C + R * Math.cos(a), C + R * Math.sin(a));
}

let paint = new canvaskit.Paint(canvas);
paint.setStyleStroke(true);
paint.setStrokeWidth(2.0);
paint.setAntialias(true);
paint.setColor(0xff4285F4);

canvas.clear(canvaskit.U32ColorFromNorm(1, 1, 1, 1));
canvas.drawPath(path, paint);
paint.unref();
path.unref();

canvas.finish();
std.print(`Disassembled:\n${canvas.toString()}\n`);
canvas.compressToBuffer();
canvas.dispose();
