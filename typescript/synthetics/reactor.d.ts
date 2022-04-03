export class Value {
}

export class VecT extends Value {
}

export class FloatT extends Value {
}

export class StringStore extends Value {
}

export class StringRef extends Value {
}

export class GShaderBuilder {
    newVec2f(v: VecT): VecT;
    newVec2f(x: number, y: number): VecT;
    newVec2f(): VecT;

    newVec3f(v: VecT): VecT;
    newVec3f(x: number, y: number, z: number): VecT;
    newVec3f(): VecT;

    newVec4f(v: VecT): VecT;
    newVec4f(x: number, y: number, z: number, w: number): VecT;
    newVec4f(): VecT;

    newFloatT(value: FloatT): FloatT;
    newFloatT(x: number): FloatT;
    newFloatT(): FloatT;

    newStringStore(x: string): StringStore;
    newStringRef(x: StringStore | StringRef): StringRef;

    newGlobalUniformFloat(location: number): FloatT;
    newGlobalUniformVec2f(location: number): FloatT;
    newGlobalUniformVec3f(location: number): FloatT;
    newGlobalUniformVec4f(location: number): FloatT;
    newGlobalUniformStringRef(location: number): StringRef;
}
