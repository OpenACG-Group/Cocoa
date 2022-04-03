let itr = {
    [Symbol.asyncIterator]() {
        let i = 0;
        return {
            next() {
                console.log('next');
                i += 1;
                if (i < 4)
                    return Promise.resolve({done: false, value: i});
                else
                    return Promise.reject('Error!');
            },
            return() {
                console.log('return');
                return { done: true };
            }
        };
    }
};

(async () => {
    for await (let value of itr) {
        console.log(value);
    }
})();
