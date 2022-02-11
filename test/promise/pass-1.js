Promise.resolve().then(() => {
    introspect.print('B\n');
});

introspect.print('A\n');

// Expected output:
// A
// B

