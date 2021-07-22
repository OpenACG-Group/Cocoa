var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
import { Core } from "internal:core";
export var Platform;
(function (Platform) {
    let ErrorNumber;
    (function (ErrorNumber) {
        ErrorNumber[ErrorNumber["Success"] = 0] = "Success";
        ErrorNumber[ErrorNumber["ErrType"] = 1] = "ErrType";
        ErrorNumber[ErrorNumber["ErrArgc"] = 2] = "ErrArgc";
        ErrorNumber[ErrorNumber["ErrOpNum"] = 3] = "ErrOpNum";
        ErrorNumber[ErrorNumber["ErrInternal"] = 4] = "ErrInternal";
        ErrorNumber[ErrorNumber["ErrInvArg"] = 5] = "ErrInvArg";
        ErrorNumber[ErrorNumber["ErrNoMem"] = 6] = "ErrNoMem";
        ErrorNumber[ErrorNumber["ErrRadRID"] = 7] = "ErrRadRID";
        ErrorNumber[ErrorNumber["ErrBusy"] = 8] = "ErrBusy";
        ErrorNumber[ErrorNumber["ErrAsync"] = 9] = "ErrAsync";
        ErrorNumber[ErrorNumber["LastEnum"] = 10] = "LastEnum";
    })(ErrorNumber = Platform.ErrorNumber || (Platform.ErrorNumber = {}));
    let Capability;
    (function (Capability) {
        Capability[Capability["None"] = 0] = "None";
        Capability[Capability["Lang"] = 1] = "Lang";
        Capability[Capability["OpCall"] = 2] = "OpCall";
        Capability[Capability["AsyncOpCall"] = 4] = "AsyncOpCall";
    })(Capability = Platform.Capability || (Platform.Capability = {}));
    Platform.version = null;
    Platform.manufacture = null;
    Platform.capabilities = Capability.None;
    function getErrno(opRet) {
        if (opRet >= 0)
            return ErrorNumber.Success;
        else if (opRet <= -ErrorNumber.LastEnum)
            throw RangeError("Bad op return value");
        else
            return -opRet;
    }
    Platform.getErrno = getErrno;
    function strerror(num) {
        switch (num) {
            case Platform.ErrorNumber.Success:
                return "Success";
            case Platform.ErrorNumber.ErrType:
                return "Bad argument type";
            case Platform.ErrorNumber.ErrArgc:
                return "Unexpected number of arguments";
            case Platform.ErrorNumber.ErrOpNum:
                return "Bad Op name or number";
            case Platform.ErrorNumber.ErrInternal:
                return "Internal error";
            case Platform.ErrorNumber.ErrNoMem:
                return "Out of memory";
            case Platform.ErrorNumber.ErrRadRID:
                return "Bad resource descriptor";
            case Platform.ErrorNumber.ErrInvArg:
                return "Invalid arguments";
            case Platform.ErrorNumber.ErrBusy:
                return "Device or resource busy";
            case Platform.ErrorNumber.ErrAsync:
                return "Asynchrony mismatched";
        }
        return "Unknown error";
    }
    Platform.strerror = strerror;
    function trap(name, args) {
        if (name.endsWith("_async"))
            return -ErrorNumber.ErrAsync;
        return Core.opCall(name, args);
    }
    Platform.trap = trap;
    function trapAsync(name, args) {
        return __awaiter(this, void 0, void 0, function* () {
            if (!name.endsWith("_async"))
                return -ErrorNumber.ErrAsync;
            return Core.opCall(name, args);
        });
    }
    Platform.trapAsync = trapAsync;
    class ResourceBase {
        constructor(rid) {
            this.rid = rid;
            this.disposed = false;
        }
        getDescriptor() {
            return this.rid;
        }
        isDisposed() {
            return this.disposed;
        }
        dispose() {
            if (this.disposed)
                return;
            let result = trap("op_dispose", { rid: this.rid });
            if (result < 0)
                throw Error("Failed to dispose resource #"
                    + String(this.rid) + ": " + strerror(getErrno(result)));
            this.disposed = true;
        }
    }
    Platform.ResourceBase = ResourceBase;
})(Platform || (Platform = {}));
(function () {
    let info = Core.getScripterInfo();
    Platform.version = {
        minor: info.version[0],
        major: info.version[1],
        patch: info.version[2]
    };
    Platform.manufacture = info.manufacture;
    for (const cap in info.capabilities) {
        if (cap == "capabilities::lang")
            Platform.capabilities &= Platform.Capability.Lang;
        else if (cap == "capabilities::opCall")
            Platform.capabilities &= Platform.Capability.OpCall;
        else if (cap == "capabilities::asyncOpCall")
            Platform.capabilities &= Platform.Capability.AsyncOpCall;
    }
})();
