import * as Glamor from 'glamor';
import * as std from 'core';
import * as canvaskit from '../canvaskit/canvaskit';

Glamor.RenderHost.Initialize({
    name: 'Scene Test',
    major: 1,
    minor: 0,
    patch: 0
});

const WIDTH = 800;
const HEIGHT = 600;

let rgroup = new canvaskit.MemoryResourceGroup();
let canvas = new canvaskit.Canvas(rgroup, WIDTH, HEIGHT);

let path = new canvaskit.Path(canvas);
let paint = new canvaskit.Paint(canvas);

path.addCircle(100, 100, 100);
paint.setColor(canvaskit.U32ColorFromNorm(1, 0, 1, 0));
paint.setAntialias(true);
paint.setStyleStroke(false);
canvas.clear(canvaskit.U32ColorFromNorm(1, 1, 1, 1));
canvas.drawPath(path, paint);
path.unref();
paint.unref();
canvas.finish();
let picture = canvas.submit().artifact;

let scene = new Glamor.SceneBuilder(WIDTH, HEIGHT)
    .pushOffset(50, 100)
    .addPicture(picture, 0, 0)
    .build();

let display = await Glamor.RenderHost.Connect();
let surface = await display.createHWComposeSurface(800, 600);
let blender = await Glamor.RenderHost.MakeBlender(surface);

surface.setTitle('Test Window');

await blender.update(scene);

surface.requestNextFrame();

// Glamor.RenderHost.Dispose();
std.print('done!\n');
