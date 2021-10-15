Cocoa.core.print("Script started\n");
let timer = new Cocoa.core.Timer();
timer.setTimeout(3000, () => {
	Cocoa.core.print("Timer reached\n");
});

Cocoa.core.exit();
Cocoa.core.print("Script end\n");
