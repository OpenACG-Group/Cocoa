import {printf} from 'core/format';

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

printf('has core module: {}\n', introspect.hasSyntheticModule('core'));
introspect.writeToJournal('info', '%italic<>Hello, my first journal log!!!%reset');
introspect.loadSharedObject('/tmp/scratch/test.so');
