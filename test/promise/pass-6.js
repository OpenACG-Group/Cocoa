import { print } from 'core';

const wait = timeout => new Promise(resolve => setTimeout(resolve, timeout));

let i = 0;
(async () => {
    while (i < 4) {
        await wait(2000);
        print(`Hello World ${i}!\n`)
        i++;
    }
})();

