import {delay, print} from 'core';

async function run() {
    await delay(1000);
    print('reached\n');
}

run();
print('end\n');

