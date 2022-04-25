export var details;
(function (details) {
    function defaultBinaryEqComparator(a, b) {
        return (a == b);
    }
    details.defaultBinaryEqComparator = defaultBinaryEqComparator;
})(details || (details = {}));
