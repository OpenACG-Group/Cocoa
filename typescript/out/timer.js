var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
import { Platform } from "./platform";
var TimerOperations;
(function (TimerOperations) {
    TimerOperations[TimerOperations["SetTimeout"] = 1] = "SetTimeout";
    TimerOperations[TimerOperations["Stop"] = 2] = "Stop";
    TimerOperations[TimerOperations["SetInterval"] = 3] = "SetInterval";
})(TimerOperations || (TimerOperations = {}));
export class Timer extends Platform.ResourceBase {
    constructor(handler, _this) {
        super(0);
        let args = { callback: handler, _this: _this };
        let rid = Platform.trap("op_timer_create", args);
        if (rid < 0) {
            throw Platform.errorByTrapsRetValue("Failed to create timer", rid);
        }
        this.setDescriptor(rid);
    }
    setTimeout(timeout) {
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
    setInterval(interval) {
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
    stop() {
        let ret = Platform.trap("op_timer_ctl", {
            rid: this.getDescriptor(),
            verb: TimerOperations.Stop
        });
        if (ret < 0) {
            throw Platform.errorByTrapsRetValue("Failed to stop timer", ret);
        }
    }
}
export function delay(timeout) {
    return __awaiter(this, void 0, void 0, function* () {
        (yield (function () {
            return __awaiter(this, void 0, void 0, function* () {
                return new Promise(((resolve) => {
                    let timer = new Timer(() => {
                        resolve(timer);
                    });
                    timer.setTimeout(timeout);
                }));
            });
        })()).dispose();
    });
}
