import {Signal} from './viz_component_base';
import {print} from 'core';

const sig1 = new Signal<number>('sig1', 10);

const slot1 = sig1.connect((a) => {
    print('Signal 1, slot 1 called\n');
});

const slot2 = sig1.connect(() => {
    print('Signal 1, slot 2 called\n');
});

const slot3 = sig1.connect(() => {
    print('Signal 1, slot 3 called\n')
});

print(`slot1 = ${slot1.toString(16)}, slot2 = ${slot2.toString(16)}\n`);

print('Emitting signal 1:\n');
sig1.emit(3322);

const slot4 = sig1.connect(() => {
    print('Signal 1, slot 4 called\n');
});

sig1.connect((a) => {
    print(`Signal 1, test slot called, a=${a}\n`);
});

sig1.disconnect(slot2);
sig1.disconnect(slot3);

const slot5 = sig1.connect(() => {
    print('Signal 1, slot 5 called\n');
});

print(`slot4 = ${slot4.toString(16)}, slot5 = ${slot5.toString(16)}\n`);

print('Emitting signal 1:\n');
sig1.emit(2233);

