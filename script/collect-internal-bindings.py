#!/usr/bin/env python3
import os.path
import sys
import modulec


if __name__ == '__main__':
    if len(sys.argv) != 3:
        print(f'Usage: {sys.argv[0]} [ModuleLists.txt file] [output directory]')
        exit(1)
    lists_file = sys.argv[1]
    lists_file_dir = os.path.dirname(lists_file)
    output_dir = sys.argv[2]

    module_names: list[str] = []

    with open(lists_file, 'r') as fp:
        for line in fp:
            line = line.strip('\n')
            split = line.split(' ')

            module_dir = lists_file_dir + '/' + split[1]
            print(f'=> Generating for binding module \'{split[0]}\' in {module_dir}')

            header_file = f'{output_dir}/module_{split[0]}.h'
            source_file = f'{output_dir}/module_{split[0]}.cc'

            modulec.main(module_dir + '/Module.xml', header_file, source_file)
            module_names.append(split[0])

    print(f'=> Generating module collection {output_dir}/internal_bindings.cc')
    with open(output_dir + '/internal_bindings.cc', 'w+') as fp:
        fp.truncate(0)
        for mod in module_names:
            fp.write('#include <vector>\n')
            fp.write(f'#include \"module_{mod}.h\"\n')
        fp.write(f'''
namespace {modulec.bindings_namespace} {{
std::vector<{modulec.bindings_namespace}::BindingBase*> on_register_internal_bindings() {{
    std::vector<{modulec.bindings_namespace}::BindingBase*> list{{\n''')

        for mod in module_names:
            fp.write(f'        on_register_module_{mod}(),\n')

        fp.write(f'''
    }};
    return list;
}}
}} // namespace {modulec.bindings_namespace}
''')
