import * as std from 'core';
import * as cobalt from 'cobalt';

const source = `
[[param(0), volatile]] let vertex_a: float2;
[[layout_triple(1UL, 32UL, 1.25f)]] let vertex_b: float2;
let vertex_c: float2;
`;

try {
    let artifact = cobalt.LapsCompiler.Compile(source);
    std.print('Artifact:\n');
    std.print(`${artifact.output}\n`);
} catch (e) {
    std.print(`Error: ${e}\n`);
}
