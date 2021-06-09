export var PropertyTreeNodeType;
(function (PropertyTreeNodeType) {
    PropertyTreeNodeType[PropertyTreeNodeType["NODE_DIRECTORY"] = 0] = "NODE_DIRECTORY";
    PropertyTreeNodeType[PropertyTreeNodeType["NODE_ARRAY"] = 1] = "NODE_ARRAY";
    PropertyTreeNodeType[PropertyTreeNodeType["NODE_DATA"] = 2] = "NODE_DATA";
    PropertyTreeNodeType[PropertyTreeNodeType["NODE_UNKNOWN"] = 3] = "NODE_UNKNOWN";
})(PropertyTreeNodeType || (PropertyTreeNodeType = {}));
function stringToPropertyTypeEnum(str) {
    if (str === 'dir')
        return PropertyTreeNodeType.NODE_DIRECTORY;
    else if (str === 'array')
        return PropertyTreeNodeType.NODE_ARRAY;
    else if (str === 'data')
        return PropertyTreeNodeType.NODE_DATA;
    else
        return PropertyTreeNodeType.NODE_UNKNOWN;
}
export class PropertyTreeNode {
    constructor(prop) {
        if (prop == null)
            return;
        this.__native = __native_fetch_property_node(prop);
        if (this.__native == undefined)
            throw Error("Failed to read property: " + prop);
        this.name = this.__native.name;
        this.type = stringToPropertyTypeEnum(this.__native.type);
    }
    static makeFromNative(node) {
        let result = new PropertyTreeNode(null);
        result.__native = node;
        result.name = node.name;
        result.type = stringToPropertyTypeEnum(node.type);
        return result;
    }
    parent() {
        return PropertyTreeNode.makeFromNative(this.__native.parent);
    }
    childrenCount() {
        return this.__native.children.length;
    }
    child(index) {
        if (index < 0 || index >= this.childrenCount())
            throw RangeError("Invalid index " + index);
        return PropertyTreeNode.makeFromNative(this.__native.children[index]);
    }
    createChild(name, type) {
        let typeString;
        switch (type) {
            case PropertyTreeNodeType.NODE_DIRECTORY:
                typeString = 'dir';
                break;
            case PropertyTreeNodeType.NODE_ARRAY:
                typeString = 'array';
                break;
            case PropertyTreeNodeType.NODE_DATA:
                typeString = 'data';
                break;
            default:
                throw Error("Invalid enumeration for 'type' argument");
        }
        let node = __native_new_property_node(this.name + '/' + name, typeString);
        return PropertyTreeNode.makeFromNative(node);
    }
}
