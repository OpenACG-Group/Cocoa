function print(str)
{
    Cocoa.core.opCall(Cocoa.core.OP_PRINT, {str: str});
}

/*
(async function() {
    const delay = () => {
        return new Promise((resolve) => {
            Cocoa.core.opCall("op_create_timer", {callback: () => {
                    print("Timer dispatched");
                    resolve();
            }});
        });
    };

    await delay();
    print("Promise resolved");
})();
 */

try {
    let context = Cocoa.core.opCall("op_va_ctx_create", {backend: "XCB"});
    Cocoa.core.opCall("op_va_ctx_connect", {rid: context, displayName: null, displayId: 1});
    Cocoa.core.opCall("op_va_ctx_dispose", {rid: context});
} catch (e) {
    print(String(e));
}

