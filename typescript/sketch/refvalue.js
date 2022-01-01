class RefValue {
    constructor(value) {
        this.value_ = value;
        this.locked_ = false;
    }

    get() {
        return this.value_;
    }
    set(value) {
        if (this.locked_)
            throw Error("Operation not permitted");
        this.value_ = value;
    }
    lock() {
        this.locked_ = true;
    }
    unlock() {
        this.locked_ = false;
    }
    isLocked() {
        return this.locked_;
    }
    clone() {
        let ref = new RefValue(this.value_);
        ref.locked_ = this.locked_;
        return ref;
    }
}


let ref = new RefValue(123);
console.log(ref.get());
