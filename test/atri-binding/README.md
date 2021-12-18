### The Simplest Language Binding Example

Generating files:
```shell
../../script/generate-binding-exports-getter.py exports.mds >generated.cc
```

Compiling:
```shell
clang++ -I../../src -I../../third_party/v8 \
        -I../../third_party/v8/include     \
        atri.cc generated.cc               \
        -std=c++2a -shared -fPIC -o libatri.so
```

Now, a dynamic library file named __libatri.so__ should be generated.

To use the dynamic library as a language binding, 2 ways are available:

- __Loading shared object by an external JS file:__

```javascript
// loader.js:
import {print} from 'core';

introspect.loadSharedObject('./libatri.so');
introspect.scheduleModuleUrlEvaluate('./use-example.js', (value) => {
    print('Finished\n');
}, (except) => {
    print('Rejected: ' + except.toString() + '\n');
});

// use-example.js:
import {print} from 'core';
import * as atri from 'Atri';

print(`fma(1, 2, 3) = ${atri.fma(1, 2, 3)}\n`);
```

Then launch Cocoa:
```shell
/path/to/Cocoa --log-level=debug                              \
               --introspect-policy=AllowLoadingSharedObject   \
               --startup=loader.js
```

- __Loading shared object directly by Cocoa:__

Launch Cocoa directly (use-example.js file is required):
```shell
/path/to/Cocoa --log-level=debug          \
               --rt-preload=./libatri.so  \
               --startup=use-example.js
```
