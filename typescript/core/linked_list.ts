export class LinkedListNode<T> {
    constructor(public list: LinkedList<T>,
                public value: T,
                public head: boolean = false,
                public next: LinkedListNode<T> = null,
                public prev: LinkedListNode<T> = null) {}
}

export class LinkedList<T> {
    readonly #head: LinkedListNode<T>;

    constructor() {
        this.#head = new LinkedListNode<T>(this, undefined, true, null, null);
        this.#head.next = this.#head;
        this.#head.prev = this.#head;
    }

    protected _checkNodeOwnership(node: LinkedListNode<T>): void {
        if (!node.list) {
            throw Error('Wild list node is not allowed');
        }
        if (node.list != this) {
            throw Error('Provided list node belongs to another list');
        }
    }

    protected _checkEmptyList(): void {
        if (this.#head.next == this.#head) {
            throw Error('Empty list');
        }
    }

    public isEmpty(): boolean {
        return (this.#head.next == this.#head);
    }

    public forEach(callback: (value: T, index: number) => void): void {
        let current = this.#head.next;
        let index = 0;
        while (!current.head) {
            callback(current.value, index);
            current = current.next;
            index++;
        }
    }

    public insertAfter(node: LinkedListNode<T>, value: T): void {
        this._checkNodeOwnership(node);
        const insert = new LinkedListNode<T>(this, value);
        insert.prev = node;
        insert.next = node.next;
        node.next.prev = insert;
        node.next = insert;
    }

    public insertBefore(node: LinkedListNode<T>, value: T): void {
        this._checkNodeOwnership(node);
        const insert = new LinkedListNode<T>(this, value);
        insert.next = node;
        insert.prev = node.prev;
        node.prev.next = insert;
        node.prev = insert;
    }

    public push(value: T): void {
        this.insertBefore(this.#head, value);
    }

    public pop(): void {
        this._checkEmptyList();
        this.removeNode(this.#head.prev);
    }

    public first(): LinkedListNode<T> {
        this._checkEmptyList();
        return this.#head.next;
    }

    public back(): LinkedListNode<T> {
        this._checkEmptyList();
        return this.#head.prev;
    }

    public removeNode(node: LinkedListNode<T>): void {
        this._checkNodeOwnership(node);
        if (node.head) {
            throw Error('Head node cannot be removed');
        }
        node.prev.next = node.next;
        node.next.prev = node.prev;
        node.prev = null;
        node.next = null;
        node.list = null;
        node.value = null;
    }

    public removeIf(pred: (value: T, index: number) => boolean): boolean {
        let current = this.#head.next;
        let index = 0;
        let removed = false;
        while (!current.head) {
            let next = current.next;
            if (pred(current.value, index)) {
                this.removeNode(current);
                removed = true;
            }
            current = next;
            index++;
        }
        return removed;
    }
}
