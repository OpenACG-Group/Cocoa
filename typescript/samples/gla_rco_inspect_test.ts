import * as GL from 'glamor';
import {print} from "../core/pretty_printer";

GL.RenderHost.Initialize({
    name: 'RCO Inspect Test',
    major: 1,
    minor: 0,
    patch: 0
});

let display = await GL.RenderHost.Connect();
display.connect('monitor-added', (monitor) => {
    print(monitor.inspectObject());
});
print(display.inspectObject());

let surface = await display.createRasterSurface(800, 600);
print(surface.inspectObject());

await surface.close();
await display.close();

GL.RenderHost.Dispose();
