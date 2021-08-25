let i = 0;
while (true) {
    let prop = "runtime.script.lbp-preloads.#" + i;
    if (!Cocoa.core.hasProperty(prop))
        break;
    Cocoa.core.print(Cocoa.core.getProperty(prop) + "\n");
    i++;
}
(async function () {
    await Cocoa.core.delay(1000);
})();
Cocoa.core.print("done\n");
