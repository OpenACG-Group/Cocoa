import {print} from 'core';

introspect.loadSharedObject('./libatri.so');
introspect.scheduleModuleUrlEvaluate('./use-example.js', (value) => {
	print('Finished\n');
}, (except) => {
	print('Rejected: ' + except.toString() + '\n');
});

