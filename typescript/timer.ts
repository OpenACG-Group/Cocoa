import {Platform} from "./platform";

enum TimerOperations
{
    SetTimeout = 1,
    Stop = 2,
    SetInterval = 3
}

export class Timer extends Platform.ResourceBase {
    constructor(handler: () => void, _this?: object) {
        super(0);

        let args = {callback: handler, _this: _this};
        let rid = Platform.trap("op_timer_create", args);

        if (rid < 0) {
            throw Platform.errorByTrapsRetValue("Failed to create timer", rid);
        }

        this.setDescriptor(rid);
    }

    setTimeout(timeout: number): void {
        if (timeout < 0)
            throw Error("Invalid timeout");

        let ret = Platform.trap("op_timer_ctl", {
            rid: this.getDescriptor(),
            verb: TimerOperations.SetTimeout,
            timeout: timeout
        });
        if (ret < 0) {
            throw Platform.errorByTrapsRetValue("Failed to set timeout", ret);
        }
    }

    setInterval(interval: number): void {
        if (interval < 0)
            throw Error("Invalid timeout");

        let ret = Platform.trap("op_timer_ctl", {
            rid: this.getDescriptor(),
            verb: TimerOperations.SetInterval,
            interval: interval
        });
        if (ret < 0) {
            throw Platform.errorByTrapsRetValue("Failed to set interval", ret);
        }
    }

    stop(): void {
        let ret = Platform.trap("op_timer_ctl", {
            rid: this.getDescriptor(),
            verb: TimerOperations.Stop
        });
        if (ret < 0) {
            throw Platform.errorByTrapsRetValue("Failed to stop timer", ret);
        }
    }
}

export async function delay(timeout: number) {
    (await (async function() {
        return new Promise<Timer>(((resolve) => {
            let timer = new Timer((): void => {
                resolve(timer);
            });
            timer.setTimeout(timeout);
        }))
    })()).dispose();
}
