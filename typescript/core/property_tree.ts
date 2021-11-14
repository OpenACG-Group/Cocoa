import * as core from 'core';
import {assert} from "./assert";

class PropertyError extends Error {
    readonly propertyPath: string;
    constructor(spec: string, what: string) {
        super(`Property[${spec}]: ${what}`);
        this.propertyPath = spec;
    }
}

export class Property {
    readonly path: string;
    protected primitive: core.PropertyPrimitive;
    protected children: core.PropertyChild[];

    constructor(path: string) {
        if (!core.hasProperty(path)) {
            throw new PropertyError(path, "Invalid or inaccessible property");
        }
        this.path = path;
        this.primitive = core.getProperty(path);
        if (this.primitive.type != "data") {
            this.children = core.enumeratePropertyNode(path);
        } else {
            this.children = null;
        }
    }

    static Get<T extends Property>(ctor: new(path: string) => T, path: string): T {
        return new ctor(path);
    }
}

export class PropertyData extends Property {
    constructor(path: string) {
        super(path);
        if (this.primitive.type != "data") {
            throw new PropertyError(path, "Property is not a data node");
        }
        assert(Reflect.has(this.primitive, "value"));
        assert(this.primitive.value != undefined);
    }
}
