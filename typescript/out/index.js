var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
import { delay } from "./timer";
import { VGContext, VGWindowBackend } from "./render/vg_context";
import { Platform } from "./platform";
(function () {
    return __awaiter(this, void 0, void 0, function* () {
        let context = new VGContext(VGWindowBackend.kXcb);
        let display = context.connect(1);
        let size = display.geometry();
        Platform.trap("op_print", { str: "width = " + size.width.toString() });
        Platform.trap("op_print", { str: "height = " + size.height.toString() });
        yield delay(1000);
        display.close();
        context.dispose();
    });
})();
