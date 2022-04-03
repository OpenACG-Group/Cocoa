import * as std from 'core';
import * as cobalt from 'cobalt';

introspect.setUnhandledPromiseRejectionHandler((promise, value) => {
    std.print(`Reject: ${value}\n`);
});

cobalt.RenderHost.Initialize({
    name: "White Eternity",
    major: 1,
    minor: 0,
    patch: 0
});

let display = await cobalt.RenderHost.Connect();
display.connect('closed', cobalt.RenderHost.Dispose);

let surface = await display.createHWComposeSurface(1280, 720);

surface.setTitle('GPU surface');

surface.connect('configure', (w, h, state) => {});
surface.connect('close', () => {
    std.print('window closed by user...\n');
    surface.close();
    display.close();
});
