declare class __native_property_node
{
    name: string;
    type: string;
    parent: __native_property_node;
    children: __native_property_node[];
}
declare function __native_fetch_property_node(prop: string): __native_property_node;
declare function __native_new_property_node(prop: string, type: string): __native_property_node;

export enum PropertyTreeNodeType
{
    NODE_DIRECTORY,
    NODE_ARRAY,
    NODE_DATA,
    NODE_UNKNOWN
}

function stringToPropertyTypeEnum(str: string): PropertyTreeNodeType
{
    if (str === 'dir')
        return PropertyTreeNodeType.NODE_DIRECTORY;
    else if (str === 'array')
        return PropertyTreeNodeType.NODE_ARRAY;
    else if (str === 'data')
        return PropertyTreeNodeType.NODE_DATA;
    else
        return PropertyTreeNodeType.NODE_UNKNOWN;
}

export class PropertyTreeNode
{
    public name: string;
    public type: PropertyTreeNodeType;
    private __native__: __native_property_node;

    constructor(prop: string)
    {
        if (prop == null)
            return;
        this.__native__ = __native_fetch_property_node(prop);
        if (this.__native__ == undefined)
            throw Error("Failed to read property: " + prop);
        this.name = this.__native__.name;
        this.type = stringToPropertyTypeEnum(this.__native__.type);
    }

    private static makeFromNative(node: __native_property_node): PropertyTreeNode
    {
        let result: PropertyTreeNode = new PropertyTreeNode(null);
        result.__native__ = node;
        result.name = node.name;
        result.type = stringToPropertyTypeEnum(node.type);
        return result;
    }

    public parent(): PropertyTreeNode
    {
        return PropertyTreeNode.makeFromNative(this.__native__.parent);
    }

    public childrenCount(): number
    {
        return this.__native__.children.length;
    }

    public child(index: number): PropertyTreeNode
    {
        if (index < 0 || index >= this.childrenCount())
            throw RangeError("Invalid index " + index);
        return PropertyTreeNode.makeFromNative(this.__native__.children[index]);
    }

    public createChild(name: string, type: PropertyTreeNodeType): PropertyTreeNode
    {
        let typeString: string;
        switch (type)
        {
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
