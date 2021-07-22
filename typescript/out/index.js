import { Platform } from "./platform";
class VaContext extends Platform.ResourceBase {
    constructor(backend) {
        let rid = Platform.trap("op_va_ctx_create", { backend: backend });
        if (rid < 0)
            throw Error(Platform.strerror(Platform.getErrno(rid)));
        super(rid);
    }
}
let ctx = new VaContext("XCB");
ctx.dispose();
