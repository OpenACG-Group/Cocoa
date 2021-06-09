/**
 * CocoaJs Core Object, Javascript ES6 Standard.
 * Copyright(C) 2021, Jingheng Luo (masshiro.io@qq.com)
 */
let Cocoa;
(function (Cocoa) {
    "use strict";

    /* Available Ops */
    Cocoa.VMC_PRINT = "vmc_print";
    Cocoa.VMC_OPEN = "vmc_open";
    Cocoa.VMC_CLOSE = "vmc_close";
    Cocoa.VMC_READ = "vmc_read";
    Cocoa.VMC_WRITE = "vmc_write";

    /* System errors.
       Note that the error number that Op returns is negative. */
    Cocoa.ETYPE       = 1;
    Cocoa.EARGC       = 2;
    Cocoa.EVMCALLNUM  = 3;
    Cocoa.EINTERNAL   = 4;
    Cocoa.EINVARG     = 5;
    Cocoa.EASYNC      = 6;
    Cocoa.ENOMEM      = 7;

    /* Default/Eternal resource descriptors */
    Cocoa.RD_STDIN  = 0;
    Cocoa.RD_STDOUT = 1;
    Cocoa.RD_STDERR = 2;

    /* Wrapper functions for performing Op */
    function vmcall(name, ...args) {
        return __do_vmcall(name, ...args);
    }
    Cocoa.vmcall = vmcall;

    function vmcallAsync(name, ...args) {
        return __do_vmcall_async(name + "_async", ...args);
    }
    Cocoa.vmcallAsync = vmcallAsync;
})(Cocoa || (Cocoa = {}));
