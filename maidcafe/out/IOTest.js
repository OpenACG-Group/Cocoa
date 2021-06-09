import { PropertyTreeNode } from "./PropertyTree";
import { IO } from "./StandardIO";
let node = new PropertyTreeNode("/runtime/command/argc");
IO.print(node.name, node.type);
