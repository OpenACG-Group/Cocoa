import * as M from '../canvaskit/comath';
import * as canvaskit from '../canvaskit/canvaskit';
import * as std from 'core';
import * as Glamor from 'glamor';
import { Compositor, IEventListener, LayerBase, LayerEffectType } from '../canvaskit/compositor';
import { Canvas } from "../canvaskit/canvaskit";

let WIDTH = 1280;
let HEIGHT = 760;

class ImageLayer extends LayerBase {
    ctx: canvaskit.MemoryResourceGroup;
    image: Glamor.CkImage;
    picture: Glamor.CkPicture;

    constructor(ctx: canvaskit.MemoryResourceGroup, parent: LayerBase, w: number, h: number, pos: M.Vector2) {
        super(parent, w, h, pos);
        this.ctx = ctx;
    }

    public loadImage(path: string): void {
        this.image = Glamor.CkImage.MakeFromEncodedFile(path);

        let prop = this.getProperties();
        let canvas = new canvaskit.Canvas(this.ctx, prop.width, prop.height);
        canvas.drawImageRect(this.image, M.Rect.MakeWH(prop.width, prop.height));
        canvas.finish();
        this.picture = canvas.submit().artifact;
        canvas.dispose();
    }

    public override onCommit(): Glamor.CkPicture {
        return this.picture;
    }
}

let context = new canvaskit.MemoryResourceGroup();

function drawNextFrame(compositor: Compositor) {
    compositor.representNextFrame();
}

class EventListener implements IEventListener {
    constructor(public timer: number, public display: Glamor.Display) {}

    onWindowResize(compositor: Compositor, width: number, height: number): void {
        std.print(`Window resize: ${width}x${height}\n`);
        WIDTH = width;
        HEIGHT = height;
    }
    onWindowCloseRequest(compositor: Compositor): void {
        std.print(`Window requests close\n`);

        clearInterval(this.timer);
        compositor.dispose();
        this.display.close();
    }
    onWindowClosed(compositor: Compositor): void {
        std.print(`Window closed\n`);
    }
    onWindowActivationChanged(compositor: Compositor, activated: boolean): void {
        std.print(`Window activation changed: ${activated}\n`);
    }
    onWindowMaximizationChanged(compositor: Compositor, maximize: boolean): void {
        std.print(`Window maximazition changed: ${maximize}\n`);
    }
    onWindowFullScreenChanged(compositor: Compositor, fullscreen: boolean): void {
        std.print(`Window fullscreen changed: ${fullscreen}\n`);
    }
    onWindowTitleChanged(compositor: Compositor, title: string): void {
        std.print(`Window title changed: ${title}\n`);
    }
    onDispose(compositor: Compositor): void {
        std.print('Compositor dispose\n');
    }
}

async function main() {
    Glamor.RenderHost.Initialize({
        name: 'Cocoa Rendering Infrastructure Test',
        major: 1,
        minor: 0,
        patch: 0
    });

    let display = await Glamor.RenderHost.Connect();
    display.connect('closed', Glamor.RenderHost.Dispose);

    let compositor = await Compositor.MakeFromSurface(display, WIDTH, HEIGHT);

    let timer = setInterval(drawNextFrame, 16, compositor);
    compositor.appendEventListener(new EventListener(timer, display));
    compositor.setWindowTitle('Amairo Chocolate [Cocoa Rendering Infrastructure Test]');

    let bgLayer = new ImageLayer(context, null, WIDTH, HEIGHT, new M.Vector2(0, 0));
    bgLayer.loadImage('/home/sora/Pictures/Library/ACG/猫耳メイドと路地裏さんぽ.jpg');
    bgLayer.getEffectStack().push({
        type: LayerEffectType.kImageFilter,
        effect: canvaskit.ImageFilter.MakeBlur(4, 4, canvaskit.TileMode.kClamp)
    });

    let charLayer = new ImageLayer(context, bgLayer, 700, 1200, new M.Vector2(300, 0));
    charLayer.loadImage('/home/sora/Pictures/Library/Stand/Chara_chieri_std1-pc.webp');

    compositor.setLayerTree(bgLayer);
}

await main();
