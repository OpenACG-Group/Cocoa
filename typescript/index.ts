import {printf} from 'core/format';
import * as core from 'core';

/*
let source = 'function foo(a, b) { return (a + b) * (a - b); }; foo;';

introspect.scheduleScriptEvaluate(source, (value) => {
    let castValue = <(a: number, b: number) => number> value;
    try {
        printf('result = {}\n', castValue(10, 22));
    } catch (e) {
        printf('Exception: {}\n', e.toString());
    }
}, (except) => {
    printf('Exception: {}\n', except.toString());
});
printf('Control follow end\n');
*/

let buffer = new core.Buffer('惑わす心 苛々彼女', 'utf8');
let view = buffer.toDataView();

let dwordSize = Math.floor(view.byteLength / 4);

for (let i = 0; i < dwordSize; i++) {
    printf('{08x} ', view.getUint32(i * 4, true));
}
printf('\n');

for (let i = 0; i < view.byteLength; i++) {
    printf('{02x} ', view.getUint8(i));
}
printf('\n');

