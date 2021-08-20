try {
    let value: string = Cocoa.core.getProperty("persistent.event-loop");
    Cocoa.core.print(value + "\n");
} catch (e) {
    Cocoa.core.print(e + "\n");
}

(async function() {
    await Cocoa.core.delay(1000);
})();

Cocoa.core.print("done\n");