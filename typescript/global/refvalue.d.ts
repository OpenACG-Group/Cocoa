// #[[IScriptURL:internal://context/refvalue,context.refvalue.xml]]
// #[[IScriptScope:SysExecute]]

/**
 * RefValue wraps a primitive-typed value as an object.
 * When you pass a primitive-typed variable (LValue) in a function call,
 * its value will be copied and if you change its value in callee, its
 * value in caller will not be changed.
 * If you do need to change a primitive-typed variable's value in callee,
 * or other similar cases, RefValue can be helpful.
 */
declare class RefValue {
    private value_: any;
    private locked_: boolean;

    constructor(value?: any);

    /**
     * Gets the value which is held.
     * @returns The value which is held.
     */
    get(): any;

    /**
     * Saves the value.
     * @param value - The value which will be held.
     * @throws Error if this RefValue instance is locked.
     */
    set(value: any): void;

    /**
     * Makes this RefValue instance locked.
     */
    lock(): void;
    
    /**
     * Makes this RefValue instance unlocked.
     */
    unlock(): void;

    /**
     * Checks whether this RefValue instance is locked.
     * @returns true if locked, otherwise false.
     */
    isLocked(): boolean;

    /**
     * Constructs a completely new RefValue instance which has the same value
     * and lock state.
     * @returns Cloned object.
     */
    clone(): RefValue;

    // #[[CCProtocol:HasInstance]]
    static __has_instance__(value: any): boolean;
}
