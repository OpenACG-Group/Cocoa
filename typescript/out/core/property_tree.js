import * as core from 'core';
import { assert } from "./assert";
class PropertyError extends Error {
    constructor(spec, what) {
        super(`Property[${spec}]: ${what}`);
        this.propertyPath = spec;
    }
}
export class Property {
    constructor(path) {
        if (!core.hasProperty(path)) {
            throw new PropertyError(path, "Invalid or inaccessible property");
        }
        this.path = path;
        this.primitive = core.getProperty(path);
        if (this.primitive.type != "data") {
            this.children = core.enumeratePropertyNode(path);
        }
        else {
            this.children = null;
        }
    }
    static Get(ctor, path) {
        return new ctor(path);
    }
}
export class PropertyData extends Property {
    constructor(path) {
        super(path);
        if (this.primitive.type != "data") {
            throw new PropertyError(path, "Property is not a data node");
        }
        assert(Reflect.has(this.primitive, "value"));
        assert(this.primitive.value != undefined);
    }
}
