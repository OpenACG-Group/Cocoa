import * as std from 'core';
import * as reactor from 'reactor';

// std.print('Reactor testing\n');

let builder = new reactor.GShaderBuilder('layer0_shader');

builder.insertJSFunctionSymbol(() => {
    std.print('amazing! this is JS callback\n');
}, 'test');

// builder.mainTestCodeGen();

builder.userMainEntrypointBasicBlock((BB) => {
    BB.createJSFunctionCall('test');
    BB.createReturn();
});

try {
    let mod = await reactor.GShaderModule.Compile(builder);
    mod.executeMain();
} catch (e) {
    std.print(`Exception: ${e}\n`);
}

