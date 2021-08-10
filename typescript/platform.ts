import {Core} from "internal:core"

export namespace Platform {
    export enum ErrorNumber {
        Success,
        ErrType,
        ErrArgc,
        ErrOpNum,
        ErrInternal,
        ErrInvArg,
        ErrNoMem,
        ErrRadRID,
        ErrBusy,
        ErrAsync,

        LastEnum
    }

    export enum Capability
    {
        None            = 0,
        Lang            = (1 << 0),
        OpCall          = (1 << 1),
        AsyncOpCall     = (1 << 2)
    }

    export let version: {
        major,
        minor,
        patch
    } = null;
    export let manufacture: string = null;
    export let capabilities: Capability = Capability.None;

    export function getErrno(opRet: number): ErrorNumber {
        if (opRet >= 0)
            return ErrorNumber.Success;
        else if (opRet <= -ErrorNumber.LastEnum)
            throw RangeError("Bad op return value");
        else
            return -opRet;
    }

    export function strerror(num: ErrorNumber): string {
        switch (num)
        {
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

    export function errorByTrapsRetValue(desc, ret): Error {
        return Error(desc + ": " + strerror(getErrno(ret)));
    }

    export function trap(name: string, args: object): number {
        if (name.endsWith("_async"))
            return -ErrorNumber.ErrAsync;
        return <number>Core.opCall(name, args);
    }

    export async function trapAsync(name: string, args: object): Promise<number> {
        if (!name.endsWith("_async"))
            return -ErrorNumber.ErrAsync;
        return <Promise<number>>Core.opCall(name, args);
    }

    export class ResourceBase {
        private rid: number;
        private disposed: boolean;

        constructor(rid?: number) {
            if (rid)
                this.rid = rid;
            this.disposed = false;
        }

        protected setDescriptor(rid: number) {
            this.rid = rid;
        }

        getDescriptor(): number {
            return this.rid;
        }

        isDisposed(): boolean {
            return this.disposed;
        }

        dispose(): void {
            if (this.disposed)
                return;

            let result = trap("op_dispose", {rid: this.rid});
            if (result < 0)
                throw Error("Failed to dispose resource #"
                    + String(this.rid) + ": " + strerror(getErrno(result)));
            this.disposed = true;
        }
    }
}

(function (){
    let info = Core.getScripterInfo();
    Platform.version = {
        minor: info.version[0],
        major: info.version[1],
        patch: info.version[2]
    };
    Platform.manufacture = info.manufacture;

    for (const cap in info.capabilities)
    {
        if (cap == "capabilities::lang")
            Platform.capabilities &= Platform.Capability.Lang;
        else if (cap == "capabilities::opCall")
            Platform.capabilities &= Platform.Capability.OpCall;
        else if (cap == "capabilities::asyncOpCall")
            Platform.capabilities &= Platform.Capability.AsyncOpCall;
    }
})();
