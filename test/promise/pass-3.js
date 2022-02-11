import {print} from 'core';

setTimeout(() => { print('B\n'); }, 0);

Promise.resolve().then(() => { print('A\n'); });

