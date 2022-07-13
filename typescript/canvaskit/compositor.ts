import * as Glamor from 'glamor';
import * as std from 'core';
import { LinkedList } from '../core/linked_list';
import * as canvaskit from './canvaskit';
import * as M from './comath';

export interface WindowConfig {
    width: number;
    height: number;
    activated: boolean;
    maximized: boolean;
    fullscreen: boolean;
}

export interface IEventListener {
    /**
     * Emitted when the native surface has finished resizing.
     */
    onWindowResize(compositor: Compositor, width: number, height: number): void;

    /**
     * Emitted when window system notifies us that the window requests to be closed.
     * For example, user clicked the "close" button on the title bar.
     */
    onWindowCloseRequest(compositor: Compositor): void;

    /**
     * Emitted when the window has already been actually closed.
     * All the resources depending on the surface will not be available anymore.
     */
    onWindowClosed(compositor: Compositor): void;

    /**
     * Emitted when the window is activated or deactivated.
     */
    onWindowActivationChanged(compositor: Compositor, activated: boolean): void;

    /**
     * Emitted when the window is maximized or unmaximized.
     */
    onWindowMaximizationChanged(compositor: Compositor, maximize: boolean): void;

    /**
     * Emitted when the window has become into fullscreen or been cancelled from fullscreen.
     */
    onWindowFullScreenChanged(compositor: Compositor, fullscreen: boolean): void;

    /**
     * Emitted when the window's title has been changed.
     */
    onWindowTitleChanged(compositor: Compositor, title: string): void;

    /**
     * Emitted before the compositor being disposed by `dispose()` method.
     */
    onDispose(compositor: Compositor): void;

    // TODO: input events
}

export class Compositor {
    private surface: Glamor.Surface;
    private blender: Glamor.Blender;
    private surfaceCloseSlot: Glamor.SlotID;
    private surfaceConfigureSlot: Glamor.SlotID;
    private surfaceResizeSlot: Glamor.SlotID;
    private windowConfig: WindowConfig;
    private windowResizable: boolean;
    private eventListeners: LinkedList<IEventListener>;

    private canvasMemoryResourceGroup: canvaskit.MemoryResourceGroup;

    public static async MakeFromSurface(display: Glamor.Display, width: number, height: number): Promise<Compositor> {
        let surface: Glamor.Surface = null;
        try {
            surface = await display.createHWComposeSurface(width, height);
        } catch(e) {
            surface = await display.createRasterSurface(width, height);
        }
        let blender = await Glamor.RenderHost.MakeBlender(surface);
        return (new Compositor(surface, blender));
    }

    private constructor(surface: Glamor.Surface, blender: Glamor.Blender) {
        this.windowResizable = true;
        this.surface = surface;
        this.blender = blender;
        this.windowConfig = {
            width: surface.width,
            height: surface.height,
            activated: false,
            maximized: false,
            fullscreen: false
        };
        this.eventListeners = new LinkedList<IEventListener>();

        this.surfaceCloseSlot = surface.connect('close', () => {
            this.onSurfaceClose();
        });
        this.surfaceConfigureSlot = surface.connect('configure', (w: number, h: number, status: number) => {
            this.onSurfaceConfigure(w, h, status);
        });
        this.surfaceResizeSlot = surface.connect('resize', (width: number, height: number) => {
            this.onSurfaceResized(width, height);
        });

        this.canvasMemoryResourceGroup = new canvaskit.MemoryResourceGroup();
    }

    private forEachEventListener(pred: (listener: IEventListener) => void): void {
        for (let listener of this.eventListeners) {
            pred(listener);
        }
    }

    private onSurfaceClose(): void {
        this.forEachEventListener((listener) => {
            listener.onWindowCloseRequest(this);
        });
    }

    private onSurfaceConfigure(width: number, height: number, status: number): void {
        if ((status & Glamor.Surface.TOPLEVEL_RESIZING) && this.windowResizable) {
            this.surface.resize(width, height);
        }

        let activated = Boolean(status & Glamor.Surface.TOPLEVEL_ACTIVATED);
        if (activated != this.windowConfig.activated) {
            this.forEachEventListener((listener) => {
                listener.onWindowActivationChanged(this, activated);
            });
            this.windowConfig.activated = activated;
        }

        let maximized = Boolean(status & Glamor.Surface.TOPLEVEL_MAXIMZED);
        if (maximized != this.windowConfig.maximized) {
            this.forEachEventListener((listener) => {
                listener.onWindowMaximizationChanged(this, maximized);
            });
            this.windowConfig.maximized = maximized;
        }

        let fullscreen = Boolean(status & Glamor.Surface.TOPLEVEL_FULLSCREEN);
        if (fullscreen != this.windowConfig.fullscreen) {
            this.forEachEventListener((listener) => {
                listener.onWindowFullScreenChanged(this, fullscreen);
            });
            this.windowConfig.fullscreen = fullscreen;
        }
    }

    private onSurfaceResized(width: number, height: number): void {
        this.windowConfig.width = width;
        this.windowConfig.height = height;
        this.forEachEventListener((listener) => {
            listener.onWindowResize(this, width, height);
        });
    }

    public getNativeSurface(): Glamor.Surface {
        return this.surface;
    }

    public getNativeBlender(): Glamor.Blender {
        return this.blender;
    }

    public getWindowConfig(): WindowConfig {
        return this.windowConfig;
    }

    public setWindowTitle(title: string): void {
        this.surface.setTitle(title).then(() => {
            this.forEachEventListener((listener) => {
                listener.onWindowTitleChanged(this, title);
            });
        });
    }

    public setWindowResizable(resizable: boolean): void {
        this.windowResizable = resizable;
    }

    public dispose(): void {
        this.forEachEventListener((listener) => {
            listener.onDispose(this);
        });
        
        Promise.all([
            this.blender.dispose(),
            this.surface.close()
        ]).then(() => {
            this.forEachEventListener((listener) => {
                listener.onWindowClosed(this);
            });
        }).catch((reason) => {
            std.print(`[Compositor] Failed to dispose current compositor: ${reason}\n`);
        }).finally(() => {
            this.surface.disconnect(this.surfaceCloseSlot);
            this.surface.disconnect(this.surfaceConfigureSlot);
            this.surface.disconnect(this.surfaceResizeSlot);
        });
    }

    public appendEventListener(listener: IEventListener): void {
        if (this.eventListeners.hasElement(listener)) {
            throw new Error('Event listener has already been in the linked list');
        }
        this.eventListeners.push(listener);
    }

    public removeEventListener(listener: IEventListener): void {
        if (!this.eventListeners.hasElement(listener)) {
            throw new Error('Event listener has not been in the linked list');
        }
        this.eventListeners.remove(listener);
    }
}
