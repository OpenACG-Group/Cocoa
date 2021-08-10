import {delay} from "./timer";
import {VGContext, VGDisplay, VGWindowBackend} from "./render/vg_context";
import {VGRect} from "./render/vg_rect";
import {Platform} from "./platform";

(async function() {
    let context: VGContext = new VGContext(VGWindowBackend.kXcb);
    let display: VGDisplay = context.connect(1);

    let size: VGRect = display.geometry();
    Platform.trap("op_print", {str: "width = " + size.width.toString()});
    Platform.trap("op_print", {str: "height = " + size.height.toString()});

    await delay(1000);
    display.close();
    context.dispose();
})();
