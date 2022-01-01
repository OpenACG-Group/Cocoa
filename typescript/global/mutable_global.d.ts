interface MutableGlobal {
    /**
     * This field and the __global__ variable declared globally
     * refer to the same Global object, which means this field
     * in Global object refers to the object itself.
     */
    __global__: MutableGlobal;
    RefValue: RefValue;
    introspect: Introspect;
}

declare let __global__: MutableGlobal;
