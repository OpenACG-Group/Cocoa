import * as Glamor from 'glamor';
import * as std from 'core';
import { LinkedList } from '../core/linked_list';
import * as canvaskit from './canvaskit';
export class Compositor {
    constructor(surface, blender) {
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
        this.eventListeners = new LinkedList();
        this.surfaceCloseSlot = surface.connect('close', () => {
            this.onSurfaceClose();
        });
        this.surfaceConfigureSlot = surface.connect('configure', (w, h, status) => {
            this.onSurfaceConfigure(w, h, status);
        });
        this.surfaceResizeSlot = surface.connect('resize', (width, height) => {
            this.onSurfaceResized(width, height);
        });
        this.canvasMemoryResourceGroup = new canvaskit.MemoryResourceGroup();
    }
    static async MakeFromSurface(display, width, height) {
        let surface = null;
        try {
            surface = await display.createHWComposeSurface(width, height);
        }
        catch (e) {
            surface = await display.createRasterSurface(width, height);
        }
        let blender = await Glamor.RenderHost.MakeBlender(surface);
        return (new Compositor(surface, blender));
    }
    forEachEventListener(pred) {
        for (let listener of this.eventListeners) {
            pred(listener);
        }
    }
    onSurfaceClose() {
        this.forEachEventListener((listener) => {
            listener.onWindowCloseRequest(this);
        });
    }
    onSurfaceConfigure(width, height, status) {
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
    onSurfaceResized(width, height) {
        this.windowConfig.width = width;
        this.windowConfig.height = height;
        this.forEachEventListener((listener) => {
            listener.onWindowResize(this, width, height);
        });
    }
    getNativeSurface() {
        return this.surface;
    }
    getNativeBlender() {
        return this.blender;
    }
    getWindowConfig() {
        return this.windowConfig;
    }
    setWindowTitle(title) {
        this.surface.setTitle(title).then(() => {
            this.forEachEventListener((listener) => {
                listener.onWindowTitleChanged(this, title);
            });
        });
    }
    setWindowResizable(resizable) {
        this.windowResizable = resizable;
    }
    dispose() {
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
    appendEventListener(listener) {
        if (this.eventListeners.hasElement(listener)) {
            throw new Error('Event listener has already been in the linked list');
        }
        this.eventListeners.push(listener);
    }
    removeEventListener(listener) {
        if (!this.eventListeners.hasElement(listener)) {
            throw new Error('Event listener has not been in the linked list');
        }
        this.eventListeners.remove(listener);
    }
}
