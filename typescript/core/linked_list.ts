import {details} from 'traint_utils';

class LinkedListNode<T> {
    constructor(public value: T, public next?: LinkedListNode<T>,
                public prev?: LinkedListNode<T>) {
    }
}

export class LinkedList<T> implements Iterable<T> {
    static readonly INVALID_INDEX = -1;

    private count: number = 0;
    private head: LinkedListNode<T> = null;
    private rear: LinkedListNode<T> = null;

    constructor(protected comparator: details.BinaryEqComparator<T> = details.defaultBinaryEqComparator) {
    }

    private atIndexUnsafe(index: number): LinkedListNode<T> {
        let cur: LinkedListNode<T> = null;
        if (index <= (this.count >>> 1)) {
            cur = this.head;
            while (index > 0 && cur != null) {
                cur = cur.next;
                index--;
            }
        } else {
            cur = this.rear;
            let p = this.count - 1;
            while (p > index && cur != null) {
                cur = cur.prev;
                p--;
            }
        }
        return cur;
    }

    private atIndexNodeSafe(index: number): LinkedListNode<T> {
        if (!Number.isInteger(index) || index < 0 || index >= this.count) {
            throw RangeError(`Invalid index value ${index} (size=${this.count})`);
        }
        return this.atIndexUnsafe(index);
    }

    private removeNode(node: LinkedListNode<T>): LinkedListNode<T> {
        if (node.prev != null) {
            node.prev.next = node.next;
        }
        if (node.next != null) {
            node.next.prev = node.prev;
        }
        let nextNode = node.next;

        if (node == this.rear) {
            this.rear = node.prev;
        }
        if (node == this.head) {
            this.head = nextNode;
        }

        node.prev = null;
        node.next = null;
        this.count--;
        return nextNode;
    }

    *[Symbol.iterator](): IterableIterator<T> {
        let current = this.head;
        while (current != null) {
            yield current.value;
            current = current.next;
        }
    }

    public size(): number {
        return this.count;
    }

    public push(value: T): void {
        let node = new LinkedListNode<T>(value);
        if (this.head == null) {
            this.head = node;
            this.rear = node;
        } else {
            this.rear.next = node;
            node.prev = this.rear;
            this.rear = node;
        }

        this.count++;
    }

    public pop(): void {
        if (this.rear == null) {
            throw Error('Pop an empty LinkedList');
        }

        if (this.rear == this.head) {
            this.head = null;
        }

        let newRear = this.rear.prev;
        if (newRear != null) {
            newRear.next = null;
        }
        this.rear.prev = null;
        this.rear = newRear;
        this.count--;
    }

    public atIndex(index: number): T {
        return this.atIndexNodeSafe(index).value;
    }

    public insertAfter(index: number, value: T): void {
        if (index == this.count - 1) {
            this.push(value);
            return;
        }
        let insertNode = new LinkedListNode<T>(value);
        let prevNode = this.atIndexNodeSafe(index);
        insertNode.prev = prevNode;
        insertNode.next = prevNode.next;
        if (prevNode.next != null) {
            prevNode.next.prev = insertNode;
        }
        prevNode.next = insertNode;
        this.count++;
    }

    public insertBefore(index: number, value: T): void {
        let insertNode = new LinkedListNode<T>(value);
        let nextNode = this.atIndexNodeSafe(index);
        insertNode.prev = nextNode.prev;
        insertNode.next = nextNode;
        if (nextNode.prev != null) {
            nextNode.prev.next = insertNode;
        } else {
            this.head = insertNode;
        }
        nextNode.prev = insertNode;
        this.count++;
    }

    public indexOf(value: T): number {
        let current = this.head;
        let i = 0;
        while (current != null) {
            if (this.comparator(current.value, value)) {
                break;
            }
            current = current.next;
            i++;
        }
        return (current != null) ? i : LinkedList.INVALID_INDEX;
    }

    public removeAt(index: number): T {
        let node = this.atIndexNodeSafe(index);
        this.removeNode(node);
        return node.value;
    }

    public remove(value: T): number {
        let counter = 0;
        let current = this.head;
        while (current != null) {
            if (this.comparator(current.value, value)) {
                current = this.removeNode(current);
                counter++;
            } else {
                current = current.next;
            }
        }
        return counter;
    }

    public removeIf(func: (value: T) => boolean): void {
        let current = this.head;
        while (current != null) {
            if (func(current.value) == true) {
                current = this.removeNode(current);
            } else {
                current = current.next;
            }
        }
    }

    public hasElement(value: T): boolean {
        let cur = this.head;
        while (cur != null) {
            if (this.comparator(cur.value, value)) {
                return true;
            }
            cur = cur.next;
        }
        return false;
    }

    public toString(): string {
        let result = '[';
        let current = this.head;
        while (current != null) {
            if (current != this.head) {
                result = result.concat(', ');
            }
            result = result.concat(current.value.toString());
            current = current.next;
        }
        return result.concat(']');
    }
}
