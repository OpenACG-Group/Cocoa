var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
try {
    let value = Cocoa.core.getProperty("persistent.event-loop");
    Cocoa.core.print(value + "\n");
}
catch (e) {
    Cocoa.core.print(e + "\n");
}
(function () {
    return __awaiter(this, void 0, void 0, function* () {
        yield Cocoa.core.delay(1000);
    });
})();
Cocoa.core.print("done\n");
