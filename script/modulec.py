#!/usr/bin/env python3
import datetime
import random
import sys
import xml.dom.minidom
from typing import TextIO


file_header_comment = f'''
/**
 * This file was generated by modulec.py from Cocoa Module Description XML file.
 * Never modify it manually. To regenerate this file, update XML file and rerun
 * modulec.py script.
 * Timestamp: {datetime.datetime.now()}
 */
'''

gallium_namespace = 'cocoa::gallium'
bindings_namespace = gallium_namespace + '::bindings'


class ToplevelExportItem:
    # function or declaration
    type: str = ''
    name: str = ''
    value: str = ''


class ClassMethodExportItem:
    name: str = ''
    static: bool = False
    value: str = ''


class ClassPropertyExportItem:
    name: str = ''
    static: bool = False
    value: str = ''
    getter: str = ''
    setter: str = ''


class ClassConstructorExportItem:
    prototype: str = ''


class ClassExportItem:
    name: str = ''
    wrapper: str = ''
    inherit_wrapper_name: str = ''
    invisible: bool = False
    constructor_export: ClassConstructorExportItem = None
    method_exports: list[ClassMethodExportItem] = None
    property_exports: list[ClassPropertyExportItem] = None


class Module:
    name: str = ''
    description: str = ''
    namespace: str = ''
    classname: str = ''
    include_srcs: list[str] = None
    instantiate_hooks: list[str] = None
    toplevel_exports: list[ToplevelExportItem] = None
    class_exports: list[ClassExportItem] = None


module: Module = None


def error_exit(msg):
    print(msg)
    exit(1)


def filter_children_by_tag_name(node: xml.dom.minidom.Element, name: str):
    result = []
    for child in node.childNodes:
        if child.nodeType == child.ELEMENT_NODE and child.tagName == name:
            result.append(child)
    return result


def visit_document(document: xml.dom.minidom.Element):
    if document.nodeName != 'module':
        error_exit('Root document name was not \'module\'')
    info_node = document.getElementsByTagName('info')
    metadata_node = document.getElementsByTagName('metadata')
    if len(info_node) == 0 or len(metadata_node) == 0:
        error_exit('Invalid document: missing module.info and module.metadata element')
    visit_info_element(info_node[0])
    visit_metadata_element(metadata_node[0])
    exports_nodes = document.getElementsByTagName('exports')
    if len(exports_nodes) == 0:
        error_exit('Invalid document: missing module.exports element')
    if len(exports_nodes) > 1:
        error_exit('Invalid document: too many module.exports elements. Only one is required')
    visit_exports_element(exports_nodes[0])
    for include_node in filter_children_by_tag_name(document, 'include'):
        if not include_node.hasAttribute('src'):
            error_exit('Invalid document: missing attribute \'src\' in module.include element')
        module.include_srcs.append(include_node.getAttribute('src'))
    for hook_node in filter_children_by_tag_name(document, 'hook'):
        if not hook_node.hasAttribute('on') or not hook_node.hasAttribute('call'):
            error_exit('Invalid document: missing \'on\' or \'call\' attributes in module.hook element')
        hook_on = hook_node.getAttribute('on')
        hook_call = hook_node.getAttribute('call')
        if hook_on == 'instantiate':
            module.instantiate_hooks.append(hook_call)
        else:
            error_exit(f'Invalid document: unknown hook name: {hook_on}')


def visit_info_element(node: xml.dom.minidom.Element):
    if not node.hasAttribute('name'):
        error_exit('Invalid document: missing attribute module.info.name')
    module.name = node.getAttribute('name')
    if not node.hasAttribute('description'):
        error_exit('Invalid document: missing attribute module.info.description')
    module.description = node.getAttribute('description')


def visit_metadata_element(node: xml.dom.minidom.Element):
    if not node.hasAttribute('namespace'):
        error_exit('Invalid document: missing attribute module.metadata.namespace')
    module.namespace = node.getAttribute('namespace')
    if not node.hasAttribute('class'):
        error_exit('Invalid document: missing attribute module.metadata.class')
    module.classname = node.getAttribute('class')


def visit_exports_element(node: xml.dom.minidom.Element):
    for toplevel_node in filter_children_by_tag_name(node, 'toplevel'):
        rc = ToplevelExportItem()
        visit_toplevel_element(toplevel_node, rc)
        module.toplevel_exports.append(rc)
    for class_node in filter_children_by_tag_name(node, 'class'):
        rc = ClassExportItem()
        visit_class_element(class_node, rc)
        module.class_exports.append(rc)


def visit_toplevel_element(node: xml.dom.minidom.Element, rc: ToplevelExportItem):
    if not node.hasAttribute('type'):
        error_exit('Invalid document: corrupted module.exports.toplevel element. '
                   'Attribute \'type\' is required')
    if not node.hasAttribute('name'):
        error_exit('Invalid document: corrupted module.exports.toplevel element. '
                   'Attribute \'name\' attribute is required')
    rc.type = node.getAttribute('type')
    rc.name = node.getAttribute('name')
    if node.hasAttribute('value'):
        rc.value = node.getAttribute('value')

    if rc.type != 'function' and rc.type != 'declaration':
        error_exit('Invalid document: corrupted module.exports.toplevel element. '
                   'Unrecognized value of attribute \'type\'')
    if rc.type != 'declaration' and len(rc.value) == 0:
        error_exit('Invalid document: corrupted module.exports.toplevel element. '
                   'Requires valid attribute \'value\' if \'type\' is not \'declaration\'')


def visit_class_element(node: xml.dom.minidom.Element, rc: ClassExportItem):
    if not node.hasAttribute('name') or not node.hasAttribute('wrapper'):
        error_exit('Invalid document: missing \'name\' or \'wrapper\' attributes in module.exports.class')
    rc.name = node.getAttribute('name')
    rc.wrapper = node.getAttribute('wrapper')

    if node.hasAttribute('inherit'):
        rc.inherit_wrapper_name = node.getAttribute('inherit')

    if node.hasAttribute('invisible'):
        value = node.getAttribute('invisible')
        if value == 'true':
            rc.invisible = True
        elif value == 'false':
            rc.invisible = False
        else:
            error_exit('Invalid document: invalid \'invisible\' attribute in module.exports.class element')

    ctor_node = filter_children_by_tag_name(node, 'constructor')
    if len(ctor_node) > 1:
        error_exit('Invalid document: more than one constructor is provided in module.exports.class element')
    elif len(ctor_node) > 0:
        rc.constructor_export = ClassConstructorExportItem()
        visit_class_constructor_element(ctor_node[0], rc.wrapper, rc.constructor_export)

    rc.method_exports = []
    rc.property_exports = []
    rc.symbol_exports = []
    for method_node in filter_children_by_tag_name(node, 'method'):
        sub_rc = ClassMethodExportItem()
        visit_class_method_element(method_node, rc.wrapper, sub_rc)
        rc.method_exports.append(sub_rc)
    for property_node in filter_children_by_tag_name(node, 'property'):
        sub_rc = ClassPropertyExportItem()
        visit_class_property_element(property_node, rc.wrapper, sub_rc)
        rc.property_exports.append(sub_rc)


def visit_class_method_element(node: xml.dom.minidom.Element, wrapper: str, rc: ClassMethodExportItem):
    if not node.hasAttribute('name') or not node.hasAttribute('value'):
        error_exit('Invalid document: missing \'name\' or \'value\' attributes in module.exports.class.method')
    rc.name = node.getAttribute('name')
    rc.value = node.getAttribute('value').replace('@', wrapper + '::')
    if node.hasAttribute('static'):
        static = node.getAttribute('static')
        if static == 'true':
            rc.static = True
        elif static == 'false':
            rc.static = False
        else:
            error_exit('Invalid value of attribute \'static\' in module.exports.class.method')


def visit_class_property_element(node: xml.dom.minidom.Element, wrapper: str, rc: ClassPropertyExportItem):
    if not node.hasAttribute('name'):
        error_exit('Invalid document: missing \'name\' attribute in module.exports.class.property')
    rc.name = node.getAttribute('name')
    if node.hasAttribute('static'):
        static = node.getAttribute('static')
        if static == 'true':
            rc.static = True
        elif static == 'false':
            rc.static = False
        else:
            error_exit('Invalid value of attribute \'static\' in module.exports.class.property')
    if node.hasAttribute('value'):
        rc.value = node.getAttribute('value').replace('@', wrapper + '::')
    if node.hasAttribute('getter'):
        rc.getter = node.getAttribute('getter').replace('@', wrapper + '::')
    if node.hasAttribute('setter'):
        rc.setter = node.getAttribute('setter').replace('@', wrapper + '::')


def visit_class_constructor_element(node: xml.dom.minidom.Element, wrapper: str, rc: ClassConstructorExportItem):
    if not node.hasAttribute('prototype'):
        error_exit('Invalid document: missing \'prototype\' attribute in module.exports.class.constructor')
    rc.prototype = node.getAttribute('prototype').replace('@', wrapper + '::')


def parse_module_file(file_path: str):
    dom = xml.dom.minidom.parse(file_path)
    visit_document(dom.documentElement)


def calc_unique_id():
    hex_transition = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                      'a', 'b', 'c', 'd', 'e', 'f']
    src_bytes = random.randbytes(8)
    result = ''
    for byte in src_bytes:
        result += hex_transition[byte >> 4]
        result += hex_transition[byte & 0x0f]
    return result


def modifier_replace(content: str) -> str:
    if len(content) == 0:
        return content
    if content[0] == '#':
        return f'v8::Symbol::Get{content[1:]}(isolate)'
    return f'\"{content}\"'


def run_class_fields_modifier_replace_pass():
    for class_ in module.class_exports:
        for method in class_.method_exports:
            method.name = modifier_replace(method.name)
        for prop in class_.property_exports:
            prop.name = modifier_replace(prop.name)


def generate_cpp_source(file: TextIO):
    def out(content: str):
        file.write(content + '\n')

    out(file_header_comment)

    out(f'''
#include "include/v8.h"
#include "Gallium/bindings/Base.h"
#include "Gallium/binder/Convert.h"
#include "Gallium/binder/Class.h"
''')

    for src in module.include_srcs:
        out(f'#include \"{src}\"')

    out(f'''
#define V_CAST_U32(x) static_cast<uint32_t>(x)
    
namespace {module.namespace} {{
class {module.classname} : public {bindings_namespace}::BindingBase
{{
    GALLIUM_BINDING_OBJECT
public:
    {module.classname}();
    ~{module.classname}() override;
    void onSetInstanceProperties(v8::Local<v8::Object> instance) override;
    void onRegisterClasses(v8::Isolate *isolate) override;
    
''')
    for cl in module.class_exports:
        out(f'    ClassExport<{cl.wrapper}> class_{cl.name}_;')
    out(f'''
}};''')

    out('namespace {')
    out(f'const char *g_unique_id = \"{calc_unique_id()}\";')
    out('const char *g_toplevel_exports[] = {')
    for toplevel in module.toplevel_exports:
        out(f'    \"{toplevel.name}\",')
    for class_ in module.class_exports:
        out(f'    \"{class_.name}\",')
    out('    nullptr\n}; // g_toplevel_exports')
    out('} // namespace anonymous')

    out(f'''
{module.classname}::{module.classname}()
    : {bindings_namespace}::BindingBase(\"{module.name}\", \"{module.description}\") {{}}
{module.classname}::~{module.classname}() {{}}''')

    out(f'''
const char *{module.classname}::onGetUniqueId() {{ return g_unique_id; }}
const char **{module.classname}::onGetExports() {{ return g_toplevel_exports; }}

void {module.classname}::onGetModule(cocoa::gallium::binder::Module& mod) {{''')

    for toplevel in module.toplevel_exports:
        if toplevel.type == 'declaration':
            continue
        out(f'    mod.set(\"{toplevel.name}\", {toplevel.value});')
    for class_ in module.class_exports:
        if class_.invisible:
            continue
        out(f'    mod.set(\"{class_.name}\", *class_{class_.name}_);')

    out('}\n')

    out(f'void {module.classname}::onRegisterClasses(v8::Isolate *isolate) {{')
    for class_ in module.class_exports:
        out(f'    class_{class_.name}_ = NewClassExport<{class_.wrapper}>(isolate);')
        out(f'    (*class_{class_.name}_)')
        if len(class_.inherit_wrapper_name) > 0:
            out(f'    .inherit<{class_.inherit_wrapper_name}>()')
        if class_.constructor_export is not None:
            out(f'    .constructor<{class_.constructor_export.prototype}>()')
        for method in class_.method_exports:
            set_prefix = 'set'
            if method.static:
                set_prefix = 'set_static_func'
            out(f'    .{set_prefix}({method.name}, &{method.value})')
        for prop in class_.property_exports:
            if prop.static:
                out(f'    .set_static({prop.name}, {prop.value})')
            else:
                property_setter_and_getter = '&' + prop.getter
                if len(prop.setter) > 0:
                    property_setter_and_getter += ', ' + '&' + prop.setter
                out(f'    .set({prop.name}, cocoa::gallium::binder::Property({property_setter_and_getter}))')
        out('    ;\n')
    out('}\n')

    out(f'void {module.classname}::onSetInstanceProperties(v8::Local<v8::Object> instance) {{')
    for hook_func in module.instantiate_hooks:
        out(f'    {hook_func}(instance);')
    out('}\n')

    out(f'}} // namespace {module.namespace}')
    out(f'''
namespace {bindings_namespace} {{
BindingBase *on_register_module_{module.name}() {{
    return new {module.namespace}::{module.classname}();
}}
}} // {bindings_namespace}
''')


def generate_cpp_header(file: TextIO):
    protect_macro = f'_COCOA_GALLIUM_BINDINGS_{module.name}_H_'
    file.write(f'''{file_header_comment}

#ifndef {protect_macro} 
#define {protect_macro}

#include "Gallium/bindings/Base.h"
namespace {bindings_namespace} {{

{bindings_namespace}::BindingBase *on_register_module_{module.name}();

}} // namespace {module.namespace}

#endif // {protect_macro}
''')


def main(xml_file: str, out_header_path: str, out_source_path: str):
    global module
    module = Module()

    module.class_exports = []
    module.toplevel_exports = []
    module.instantiate_hooks = []
    module.include_srcs = []

    parse_module_file(xml_file)
    run_class_fields_modifier_replace_pass()
    with open(out_source_path, 'w+') as fp:
        fp.truncate(0)
        generate_cpp_source(fp)
    with open(out_header_path, 'w+') as fp:
        fp.truncate(0)
        generate_cpp_header(fp)

    module = None


if __name__ == '__main__':
    if len(sys.argv) != 4:
        print(f'Usage: {sys.argv[0]} [module.xml file] [output header] [output source]')
        exit(1)
    main(sys.argv[1], sys.argv[2], sys.argv[3])
