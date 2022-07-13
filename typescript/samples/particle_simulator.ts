/**
 * Cocoa Rendering Infrastructure - Particle Simulator Sample
 * 
 * Running this sample by:
 * <Cocoa executable> --startup=samples/particle_simulator.js
 *                    --cobalt-concurrent-workers=8
 *                    --cobalt-show-tile-boundaries
 *                    --log-level=quiet
 *                    <typescript compilation out dir.>
 */

import * as M from '../canvaskit/comath';
import * as canvaskit from '../canvaskit/canvaskit';
import * as std from 'core';
import * as Glamor from 'glamor';
import { Compositor, IEventListener, LayerBase } from '../canvaskit/compositor';
import { Canvas } from "../canvaskit/canvaskit";

const MAX_CATEGORY = 3;
const MAX_AGE = 650;
const MAX_PARTICLES = 5000;

let WIDTH = 1280;
let HEIGHT = 760;

class Particle {
    id: number;
    pos: M.Vector2;
    velocity: M.Vector2;
    radius: M.Scalar;
    age: number;
    category: number;
}

class ParticleSimulator {
    private particles: Array<Particle>;
    private counter: number;

    constructor() {
        this.particles = new Array<Particle>();
        this.counter = 0;
    }

    public evaluate(): void {
        let j = 0;
        for (let p of this.particles) {
            p.pos = p.pos.add(p.velocity);
            if (++p.age >= MAX_AGE) {
                continue;
            }
            this.particles[j++] = p;
        }
        while (this.particles.length > j) {
            this.particles.pop();
        }

        let count = (Math.random() * MAX_PARTICLES) | 0;
        for (let i = 0; i < count; i++) {
            if (this.particles.length >= MAX_PARTICLES) {
                break;
            }
            let angle = Math.random() * Math.PI * 2;
            let speed = Math.max(Math.random() * 6.0, 0.6);

            let p = new Particle();
            p.id = this.counter;
            p.pos = new M.Vector2(0, 0);
            p.velocity = new M.Vector2(speed * Math.cos(angle), speed * Math.sin(angle));
            p.age = (Math.min(Math.random(), 0.5) * MAX_AGE) | 0;
            p.category = (Math.random() * (MAX_CATEGORY - 1)) | 0;
            p.radius = 6;
            this.particles.push(p);

            this.counter++;
        }
    }

    public draw(canvas: canvaskit.Canvas, width: M.Scalar, height: M.Scalar): void {
        canvas.clear(canvaskit.U32ColorFromNorm(0, 0, 0, 0));

        let paths: Array<canvaskit.Path> = [];
        for (let i = 0; i < MAX_CATEGORY; i++) {
            paths.push(new canvaskit.Path(canvas));
        }

        for (let p of this.particles) {
            let x = width / 2 + p.pos.x;
            let y = height / 2 + p.pos.y;
            if (x >= width || y >= height || x < 0 || y < 0) {
                continue;
            }

            let radius = (MAX_AGE - p.age) / MAX_AGE * p.radius;
            paths[p.category].addCircle(x, y, radius);
        }

        let paint = new canvaskit.Paint(canvas);
        paint.setStyleStroke(false);
        paint.setAntialias(true);
        paint.setBlendMode(canvaskit.BlendMode.kPlus);

        let colors = [0xffff7f00, 0xffff3f9f, 0xff7f4fff];

        for (let i = 0; i < MAX_CATEGORY; i++) {
            paint.setColor(colors[i]);
            canvas.drawPath(paths[i], paint);
        }

        paint.unref();
        for (let path of paths) {
            path.unref();
        }
    }
}

class ParticleLayer extends LayerBase {
    simulator: ParticleSimulator;
    ctx: canvaskit.MemoryResourceGroup;
    
    constructor(ctx: canvaskit.MemoryResourceGroup, parent: LayerBase, w: number, h: number, pos: M.Vector2) {
        super(parent, w, h, pos);
        this.ctx = ctx;
        this.simulator = new ParticleSimulator();
    }

    public override onCommit(): Glamor.CkPicture {
        let prop = this.getProperties();
        let canvas = new canvaskit.Canvas(this.ctx, prop.width, prop.height);
        this.simulator.evaluate();
        this.simulator.draw(canvas, prop.width, prop.height);
        canvas.finish();
        let artifact = canvas.submit().artifact;
        canvas.dispose();

        return artifact;
    }
}

class ImageLayer extends LayerBase {
    ctx: canvaskit.MemoryResourceGroup;
    image: Glamor.CkImage;

    constructor(ctx: canvaskit.MemoryResourceGroup, parent: LayerBase, w: number, h: number, pos: M.Vector2) {
        super(parent, w, h, pos);
        this.ctx = ctx;
    }

    public loadImage(path: string): void {
        this.image = Glamor.CkImage.MakeFromEncodedFile(path);
    }

    public override onCommit(): Glamor.CkPicture {
        let prop = this.getProperties();
        let canvas = new canvaskit.Canvas(this.ctx, prop.width, prop.height);
        canvas.drawImage(this.image, 0, 0);
        canvas.finish();
        let artifact = canvas.submit().artifact;
        canvas.dispose();
        return artifact;
    }
}

let context = new canvaskit.MemoryResourceGroup();
let layer: ParticleLayer = null;

let count = 0;
let totalTime = 0;
function drawNextFrame(compositor: Compositor) {

    let prop = layer.getProperties();
    layer.setProperties({
        width: prop.width,
        height: prop.height,
        positionInParent: prop.positionInParent.add(new M.Vector2(0.5, 0.5))
    });
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

    let bgLayer = new ImageLayer(context, null, WIDTH, HEIGHT, new M.Vector2(0, 0));
    bgLayer.loadImage('/home/sora/Pictures/Library/ACG/猫耳メイドと路地裏さんぽ.jpg');

    (new ImageLayer(context, bgLayer, WIDTH, HEIGHT, new M.Vector2(0, 0)))
        .loadImage('/home/sora/Pictures/Library/Stand/Chara_chieri_std1-pc.webp');

    layer = new ParticleLayer(context, bgLayer, 500, 400, new M.Vector2(0, 0));

    compositor.setLayerTree(bgLayer);
}

await main();
