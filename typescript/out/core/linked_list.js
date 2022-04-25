import { details } from 'traint_utils';
class LinkedListNode {
    constructor(value, next, prev) {
        this.value = value;
        this.next = next;
        this.prev = prev;
    }
}
export class LinkedList {
    constructor(comparator = details.defaultBinaryEqComparator) {
        this.comparator = comparator;
        this.count = 0;
        this.head = null;
        this.rear = null;
    }
    atIndexUnsafe(index) {
        let cur = null;
        if (index <= (this.count >>> 1)) {
            cur = this.head;
            while (index > 0 && cur != null) {
                cur = cur.next;
                index--;
            }
        }
        else {
            cur = this.rear;
            let p = this.count - 1;
            while (p > index && cur != null) {
                cur = cur.prev;
                p--;
            }
        }
        return cur;
    }
    atIndexNodeSafe(index) {
        if (!Number.isInteger(index) || index < 0 || index >= this.count) {
            throw RangeError(`Invalid index value ${index} (size=${this.count})`);
        }
        return this.atIndexUnsafe(index);
    }
    removeNode(node) {
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
    *[Symbol.iterator]() {
        let current = this.head;
        while (current != null) {
            yield current.value;
            current = current.next;
        }
    }
    size() {
        return this.count;
    }
    push(value) {
        let node = new LinkedListNode(value);
        if (this.head == null) {
            this.head = node;
            this.rear = node;
        }
        else {
            this.rear.next = node;
            node.prev = this.rear;
            this.rear = node;
        }
        this.count++;
    }
    pop() {
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
    atIndex(index) {
        return this.atIndexNodeSafe(index).value;
    }
    insertAfter(index, value) {
        if (index == this.count - 1) {
            this.push(value);
            return;
        }
        let insertNode = new LinkedListNode(value);
        let prevNode = this.atIndexNodeSafe(index);
        insertNode.prev = prevNode;
        insertNode.next = prevNode.next;
        if (prevNode.next != null) {
            prevNode.next.prev = insertNode;
        }
        prevNode.next = insertNode;
        this.count++;
    }
    insertBefore(index, value) {
        let insertNode = new LinkedListNode(value);
        let nextNode = this.atIndexNodeSafe(index);
        insertNode.prev = nextNode.prev;
        insertNode.next = nextNode;
        if (nextNode.prev != null) {
            nextNode.prev.next = insertNode;
        }
        else {
            this.head = insertNode;
        }
        nextNode.prev = insertNode;
        this.count++;
    }
    indexOf(value) {
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
    removeAt(index) {
        let node = this.atIndexNodeSafe(index);
        this.removeNode(node);
        return node.value;
    }
    remove(value) {
        let counter = 0;
        let current = this.head;
        while (current != null) {
            if (this.comparator(current.value, value)) {
                current = this.removeNode(current);
                counter++;
            }
            else {
                current = current.next;
            }
        }
        return counter;
    }
    removeIf(func) {
        let current = this.head;
        while (current != null) {
            if (func(current.value) == true) {
                current = this.removeNode(current);
            }
            else {
                current = current.next;
            }
        }
    }
    hasElement(value) {
        let cur = this.head;
        while (cur != null) {
            if (this.comparator(cur.value, value)) {
                return true;
            }
            cur = cur.next;
        }
        return false;
    }
    toString() {
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
LinkedList.INVALID_INDEX = -1;
