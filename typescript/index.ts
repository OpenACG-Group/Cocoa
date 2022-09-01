import * as std from 'core';
import * as canvaskit from 'canvaskit/canvaskit';
import * as M from 'canvaskit/comath';
import * as render from "cobalt";


let context = new canvaskit.MemoryResourceGroup();

function paint(): render.GskPicture {
    let canvas = new canvaskit.Canvas(context, 800, 600);

    let paint = new canvaskit.Paint(canvas);
    let path = new canvaskit.Path(canvas);

    paint.setAntialias(true);
    paint.setStyleStroke(true);
    paint.setStrokeCap(canvaskit.PaintCap.kRound);
    paint.setStrokeWidth(2);
    paint.setColor(canvaskit.U32ColorFromNorm(1, 1, 0, 0));

    path.addOval(M.Rect.MakeXYWH(50, 50, 200, 120));

    canvas.clear(canvaskit.U32ColorFromNorm(1, 1, 1, 1));
    canvas.drawPath(path, paint);
    paint.unref();
    path.unref();

    canvas.finish();

    std.print('Generated IR code:\n');
    std.print(canvas.toString());
    let artifact = canvas.submit().artifact;
    canvas.dispose();

    return artifact;
}


async function main() {
    render.RenderHost.Initialize({
        name: 'Cocoa Rendering Infrastructure Test',
        major: 1,
        minor: 0,
        patch: 0
    });

    let display = await render.RenderHost.Connect();
    display.connect('closed', render.RenderHost.Dispose);

    let surface = await display.createHWComposeSurface(800, 600);
    surface.setTitle('Cocoa Rendering Infrastructure Test');

    let blender = await render.RenderHost.MakeBlender(surface);

    let picture = paint();
    let timer = setInterval(() => {
        std.print('updated\n');
        blender.paint(picture, { left: 0, top: 0, right: 800, bottom: 600 });
    }, 500);

    surface.connect('close', () => {
        std.print('Window is closed by user...\n');
        clearInterval(timer);
        blender.dispose();
        surface.close();
        display.close();
    });
}

await main();
