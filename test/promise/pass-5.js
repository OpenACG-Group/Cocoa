let dev = global.introspect;

dev.print('PassA\n');

global.run = () => {
    introspect.print('PassC\n');
};

introspect.print('PassB\n');
run();

global = {};
introspect.print('PassD\n');

