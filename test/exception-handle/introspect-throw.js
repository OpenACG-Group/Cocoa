// introspect.scheduleScriptEvaluate('throw Error("Test error");');
introspect.scheduleModuleUrlEvaluate('throw.js', (value) => {}, (except) => {
    throw Error("Throw from exception handler");
});

