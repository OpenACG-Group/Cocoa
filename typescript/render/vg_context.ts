import {Platform} from "../platform";
import {VGRect} from "./vg_rect";

export enum VGWindowBackend
{
    kXcb = "XCB"
}

export class VGContext extends Platform.ResourceBase {
    constructor(backend: VGWindowBackend) {
        super();

        let rid = Platform.trap("op_va_ctx_create", {
            backend: backend
        });
        if (rid < 0) {
            throw Platform.errorByTrapsRetValue("Failed to create VGContext", rid);
        }
        this.setDescriptor(rid);
    }

    connect(id: number, device?: string): VGDisplay {
        if (id < 0) {
            throw Error("Illegal display ID");
        }

        let displayDevice: string = null;
        if (device) {
            displayDevice = device;
        }
        let ret = Platform.trap("op_va_ctx_connect", {
            rid: this.getDescriptor(),
            displayName: displayDevice,
            displayId: id
        });

        if (ret < 0) {
            throw Platform.errorByTrapsRetValue("Failed to connect to display server", ret);
        }

        return new VGDisplay(this, id);
    }
}

export class VGDisplay {
    private readonly context: VGContext;
    private readonly id: number;
    private closed: boolean;

    constructor(context: VGContext, id: number) {
        this.context = context;
        this.id = id;
        this.closed = false;
    }

    getContext(): VGContext {
        return this.context;
    }

    getID(): number {
        return this.id;
    }

    geometry(): VGRect {
        let out = <{width: number, height: number}>{};
        let ret = Platform.trap("op_va_dis_geometry", {
            rid: this.context.getDescriptor(),
            displayId: this.id,
            out: out
        });
        if (ret < 0) {
            throw Platform.errorByTrapsRetValue("Failed to get geometry of display", ret);
        }

        return VGRect.MakeXYWH(0, 0, out.width, out.height);
    }

    close(): void {
        if (this.closed)
            return;
        let ret = Platform.trap("op_va_dis_close", {
            rid: this.context.getDescriptor(),
            displayId: this.id
        });
        if (ret < 0) {
            throw Platform.errorByTrapsRetValue("Failed to close display", ret);
        }
        this.closed = true;
    }

    isClosed(): boolean {
        return this.closed;
    }
}
