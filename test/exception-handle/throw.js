function foo() {
	throw Error("Test error");
}

function bar() {
	foo();
}

function toplevel() {
	bar();
}

class T {
	constructor() {
		toplevel();
	}
}

let s = new T();

