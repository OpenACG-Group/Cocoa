export namespace details {
    export type BinaryEqComparator<T> = (a: T, b: T) => boolean;

    export function defaultBinaryEqComparator<T>(a: T, b: T): boolean {
        return (a == b);
    }
}
