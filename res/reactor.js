import * as std from 'core';
import * as reactor from 'reactor';

std.print('Reactor testing\n');

let builder = new reactor.GShaderBuilder('layer0_shader');
builder.insertJSFunctionSymbol(() => {
    std.print('amazing! this is JS callback\n');
}, 'test');

builder.mainTestCodeGen();

let module = await reactor.GShaderModule.Compile(builder);
module.executeMain();
