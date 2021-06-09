export var IO;
(function (IO) {
    function printCallable(first, rest, callback) {
        let buffer = JSON.stringify(first);
        for (let obj of rest)
            buffer += " " + JSON.stringify(obj);
        return callback(buffer);
    }
    /**
     * @brief Prints objects to standard output.
     * @return number A number which means how many bytes
     *         are printed.
     */
    function print(first, ...rest) {
        return printCallable(first, rest, __native_print);
    }
    IO.print = print;
    /**
     * @brief Prints objects to standard error.
     * @return number A number which means how many bytes
     *         are printed.
     */
    function printError(first, ...rest) {
        return printCallable(first, rest, __native_print_err);
    }
    IO.printError = printError;
})(IO || (IO = {}));
