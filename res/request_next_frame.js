import * as std from 'core';
import * as GL from 'glamor';

GL.RenderHost.Initialize({
    name: 'Test',
    major: 1,
    minor: 0,
    patch: 0
});

let display = await GL.RenderHost.Connect();
// let surface = await display.createRasterSurface(800, 600);

// surface.connect('frame', (sequence) => {
//    std.print(`${getMillisecondTimeCounter()}: Get frame signal, sequence=${sequence}\n`);
//    surface.requestNextFrame();
// });

// await surface.requestNextFrame();
// std.print('Requested\n');

let monitorList = await display.requestMonitorList();
for (let monitor of monitorList) {
    monitor.connect('properties-changed', (propertySet) => {
        std.print(`${JSON.stringify(propertySet)}\n`);
    });
    monitor.requestPropertySet();
    std.print('new monitor\n');
}

