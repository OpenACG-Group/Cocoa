#!/usr/bin/env python3

import re
import sys

generated_file_header_comment = f'''
/**
 * This file was generated by Renpy Code Generating Scripts and built by building system
 * automatically.
 * Never modify this file manually or compile it standalone.
 * Renpy generator script: {__file__}
 * Source file: {sys.argv[2]}
 */
'''

codegen_entries = []
enum_map_entries = []

type_name_ts_convert_dict = {
    # [typescript typename, writer method name, size in bytes]
    'u8': ['number', 'Uint8', 1],
    'i8': ['number', 'Int8', 1],
    'u16': ['number', 'Uint16', 2],
    'i16': ['number', 'Int16', 2],
    'u32': ['number', 'Uint32', 4],
    'i32': ['number', 'Int32', 4],
    'u64': ['bigint', 'Uint64', 8],
    'i64': ['bigint', 'Int64', 8],
    'f32': ['number', 'Float32', 4],
    'f64': ['number', 'Float64', 8]
}

type_name_cpp_convert_dict = {
    'u8': 'uint8_t',
    'i8': 'int8_t',
    'u16': 'uint16_t',
    'i16': 'int16_t',
    'u32': 'uint32_t',
    'i32': 'int32_t',
    'u64': 'uint64_t',
    'i64': 'int64_t',
    'f32': 'float',
    'f64': 'double',

    # Memory operations
    'string': 'SkString',
    'u32array': 'MoeHeap::U32Array',
    'f32array': 'MoeHeap::F32Array',
    'vec2': 'SkV2',
    'vec3': 'SkV3',
    'vec4': 'SkV4',
    'mat3x3': 'SkMatrix',
    'mat4x4': 'SkM44',
    'rect': 'SkRect',
    'rrect': 'SkRRect',
    'region': 'SkRegion',
    'path': 'SkPath',
    'paint': 'SkPaint',
    'samplingoptions': 'SkSamplingOptions',
    'shader': 'sk_sp<SkShader>',
    'blender': 'sk_sp<SkBlender>',
    'colorfilter': 'sk_sp<SkColorFilter>',
    'imagefilter': 'sk_sp<SkImageFilter>',
    'maskfilter': 'sk_sp<SkMaskFilter>',
    'patheffect': 'sk_sp<SkPathEffect>',

    'bitmap': 'std::shared_ptr<SkBitmap>',
    'image': 'sk_sp<SkImage>',
    'picture': 'sk_sp<SkPicture>'
}

cpp_disassembler_type_id_dict = {
    'u8': 1,
    'i8': 2,
    'u16': 3,
    'i16': 4,
    'u32': 5,
    'i32': 6,
    'u64': 7,
    'i64': 8,
    'f32': 9,
    'f64': 10,

    # Memory operations
    'string': 0,
    'u32array': 0,
    'f32array': 0,
    'vec2': 0,
    'vec3': 0,
    'vec4': 0,
    'mat3x3': 0,
    'mat4x4': 0,
    'rect': 0,
    'rrect': 0,
    'region': 0,
    'path': 0,
    'paint': 0,
    'samplingoptions': 0,
    'shader': 0,
    'blender': 0,
    'colorfilter': 0,
    'imagefilter': 0,
    'maskfilter': 0,
    'patheffect': 0,

    'bitmap': 0,
    'image': 0,
    'picture': 0,

    'any': 0
}


def parse_entries(file: str):
    with open(file, 'r') as fp:
        opcode = 1
        impl_mode = False
        enum_map_mode = False
        enum_map_entry = None
        last_entry = None
        for line in fp:
            if line[0] == '\n':
                continue

            if impl_mode:
                if line == '%}\n':
                    impl_mode = False
                    continue
                if last_entry is None:
                    raise Exception('Invalid implementation code without an instruction declaration')
                if last_entry['impl'] is None:
                    last_entry['impl'] = line
                else:
                    last_entry['impl'] += line
                continue

            if enum_map_mode:
                if line == '%end_enum_map\n':
                    enum_map_entries.append(enum_map_entry)
                    enum_map_mode = False
                    continue
                result = re.match(r'^([\w_][\w\d_]*)\s+([\w\d:_]+)$', line)
                if result is None:
                    raise Exception('Invalid enumeration map declaration')
                members = enum_map_entry['members']
                members.append({'ir_value': len(members), 'name': result.group(1),
                                'cpp_value': result.group(2)})
                continue

            if line == '%{\n':
                impl_mode = True
                continue

            result = re.match(r'^%enum_map\s+([_\w][\w\d_]*)\s+([\w\d:_]+)\s+([\w\d:_]+)$', line)
            if result is not None:
                enum_map_mode = True
                enum_map_entry = {'type_name': result.group(1), 'underlying_type_name': result.group(2),
                                  'cpp_type_name': result.group(3), 'members': []}
                continue

            result = re.match(r'([\w.][\w\d]*)\s+(.*)?', line)
            if result is None:
                raise Exception('invalid opcode name')

            op_name = result.group(1)
            params = []

            params_remain = result.group(2)
            while params_remain is not None:
                result = re.match(r'(\s*,\s*)?([\w\d]+[?]?)\s+([%$][\w\d]+)(.*)', params_remain)
                if result is None:
                    break
                type_name = result.group(2)
                value = result.group(3)
                params_remain = result.group(4)

                is_mem_operand = value[0] == '%'
                is_nullable = type_name[-1] == '?'
                if is_nullable:
                    type_name = type_name[:-1]
                params.append({'type_name': type_name, 'value': value[1:], 'memop': is_mem_operand,
                               'nullable': is_nullable})

            if op_name[0] == '.':
                is_annotation = True
                op_name = op_name[1:]
            else:
                is_annotation = False

            last_entry = {'name': op_name, 'annotation': is_annotation,
                          'params': params, 'desc': line[:-1], 'opcode': opcode, 'impl': None}
            codegen_entries.append(last_entry)
            opcode += 1


def try_replace_type_in_enum_map(typename):
    for entry in enum_map_entries:
        if entry['type_name'] == typename:
            return entry['underlying_type_name']
    return typename


def find_type_in_enum_map(typename):
    for entry in enum_map_entries:
        if entry['type_name'] == typename:
            return entry
    return None


def codegen_typescript():
    print('import { IProtoBufferWriter } from \'../canvaskit_iface\';')
    print('export namespace Opcode {')
    for entry in codegen_entries:
        desc = entry['desc']
        name = entry['name']
        opcode = entry['opcode']
        print(f'// Description: {desc}')
        print(f'export const k{name} = {hex(opcode)};\n')

    print('} // namespace Opcode')

    print('export namespace Constants {')
    for entry in enum_map_entries:
        for member in entry['members']:
            print(f"export const {entry['type_name'].upper()}_{member['name']} = {hex(member['ir_value'])};")
    print('} // namespace Constants')

    print('''
export type MemOp = number;
export class ProtoCodeEmitter {
  private writer: IProtoBufferWriter;
  constructor(writer: IProtoBufferWriter) {
    this.writer = writer;
  }
''')

    for entry in codegen_entries:
        # Generate function signature
        ts_func_params = ''
        # opcode is uint16, which is 2 bytes
        required_size = 2

        entry_params = entry['params']
        for operand in entry_params:
            typename = try_replace_type_in_enum_map(operand['type_name'])

            if operand['memop'] is True:
                # MemOp is uint32, which is 4 bytes
                required_size += 4
                ts_type = 'MemOp'
            else:
                convert = type_name_ts_convert_dict[typename]
                ts_type = convert[0]
                required_size += convert[2]

            name = operand['value']
            if len(ts_func_params) > 0:
                ts_func_params += ', '
            ts_func_params += f'{name}: {ts_type}'
        name = entry['name']
        print(f'  public emit{name}({ts_func_params}): void {{')

        # Generate function body
        op_name = entry['name']
        print(f'    this.writer.performPossibleBufferSwitching({str(required_size)});')
        print(f'    this.writer.writeUint16Unsafe(Opcode.k{op_name} | {hex(len(entry_params) << 8)});')
        for operand in entry['params']:
            if operand['memop']:
                writer_type = 'Uint32'
            else:
                typename = try_replace_type_in_enum_map(operand['type_name'])
                writer_type = type_name_ts_convert_dict[typename][1]

            name = operand['value']
            print(f'    this.writer.write{writer_type}Unsafe({name});')

        # End
        print('  }')

    print('}')


def codegen_cpp_impl():
    print('namespace opcode {')

    # Generate enum-mapping translators
    for entry in enum_map_entries:
        underlying_type = type_name_cpp_convert_dict[entry['underlying_type_name']]
        cpp_type_name = entry['cpp_type_name']
        print(f"inline {cpp_type_name} ir_enum_translator_{entry['type_name']}({underlying_type} x) {{")
        print(f'  static {cpp_type_name} __tbl[] = {{')
        for member in entry['members']:
            print(f"    {member['cpp_value']},")
        print('  };')
        print(f"  if (x < 0 || x >= {len(entry['members'])})")
        print(f"    throw_error(\"Invalid enumeration value for type {entry['type_name']}\");")
        print('  return __tbl[x];')
        print('}')

    # Generate instruction executors
    for entry in codegen_entries:
        name = entry['name']
        params = entry['params']
        cpp_executor_args = ''
        for operand in params:
            typename = operand['type_name']
            if typename != 'any':
                maybe_enum_map_entry = find_type_in_enum_map(typename)
                if maybe_enum_map_entry is None:
                    executor_arg = type_name_cpp_convert_dict[typename] + ' '
                else:
                    executor_arg = maybe_enum_map_entry['cpp_type_name'] + ' '
            else:
                executor_arg = 'MemOp '

            if operand['memop'] is True and operand['nullable'] is True:
                # For nullable memory operands, we pass a nullable-pointer directly instead of reference
                executor_arg += '* '
            elif operand['memop'] is True and typename != 'any':
                executor_arg += '& '

            executor_arg += operand['value']
            cpp_executor_args += ', ' + executor_arg

        if entry['annotation'] is True:
            common_args = 'ExecutionContext& context'
        else:
            common_args = 'MoeHeap& heap, SkCanvas *canvas'

        if entry['impl'] is None:
            print(f'void __op_inst_{name}({common_args}{cpp_executor_args});')
        else:
            print(f"void __op_inst_{name}({common_args}{cpp_executor_args}) {{\n{entry['impl']}\n}}")

    # Generate query table
    print('const OpcodeVectorTbl g_opcode_vector_tbl[] = {')
    for entry in codegen_entries:
        opcode_enum_name = entry['name']
        name = opcode_enum_name
        if entry['annotation'] is True:
            name = '.' + opcode_enum_name

        operands_field = ''
        # Size of opcode (uint16)
        fixed_size = 2
        for operand in entry['params']:
            if len(operands_field) > 0:
                operands_field += ', '
            type_name = try_replace_type_in_enum_map(operand['type_name'])
            operands_field += f"{{\"{operand['type_name']}\", {cpp_disassembler_type_id_dict[type_name]}}}"
            if type_name in type_name_ts_convert_dict:
                fixed_size += type_name_ts_convert_dict[type_name][2]
            else:
                # Size of memory operand (uint32)
                fixed_size += 4

        if len(operands_field) > 0:
            operands_field += ', '
        operands_field += '{nullptr, -1}'

        print(f'  {{ k{opcode_enum_name},  "{name}", {fixed_size}, {{ {operands_field} }} }},')

    print('}; // g_opcode_vector_tbl')
    print(f'const size_t g_opcode_vector_tbl_count = {len(codegen_entries)};')

    # Generate instruction dispatcher
    print('void Dispatch(MoeByteStreamReader& reader, ExecutionContext& context) {')
    print('''
  MoeHeap& heap = context.heap;
  SkCanvas *canvas = context.canvas;
  while (true) {
    uint16_t verb = reader.ExtractNext<uint16_t>();
    if (VERB_OPCODE(verb) == kCommandPoolEnd)
      break;
    if (VERB_OPCODE(verb) == kSwitchNextBuffer) {
      reader.MoveToNextBuffer();
      continue;
    }
    switch (VERB_OPCODE(verb)) {''')
    for entry in codegen_entries:
        name = entry['name']
        print(f'      case k{name}: {{')
        param_count = 0
        for operand in entry['params']:
            typename = operand['type_name']
            if operand['memop'] is True:
                if typename != 'any':
                    cpp_type = type_name_cpp_convert_dict[typename]
                    if operand['nullable'] is True:
                        print(f"        auto __memop_x{param_count} = reader.ExtractNext<MemOp>();")
                        print(f"        {cpp_type} *x{param_count} = (__memop_x{param_count} > 0) ? &heap.Extract<{cpp_type}>(__memop_x{param_count}) : nullptr;")
                    else:
                        print(f"        auto& x{param_count} = heap.Extract<{cpp_type}>(reader.ExtractNext<MemOp>());")
                else:
                    print(f"        auto x{param_count} = reader.ExtractNext<MemOp>();")
            else:
                maybe_enum_map_entry = find_type_in_enum_map(typename)
                if maybe_enum_map_entry is None:
                    cpp_type = type_name_cpp_convert_dict[typename]
                    print(f"        auto x{param_count} = reader.ExtractNext<{cpp_type}>();")
                else:
                    enum_name = maybe_enum_map_entry['type_name']
                    underlying_type = type_name_cpp_convert_dict[maybe_enum_map_entry['underlying_type_name']]
                    print(f"        auto x{param_count} = ir_enum_translator_{enum_name}(reader.ExtractNext<{underlying_type}>());")
            param_count += 1

        if entry['annotation'] is True:
            executor_call_args = 'context'
        else:
            executor_call_args = 'heap, canvas'
        for index in range(param_count):
            executor_call_args += f', x{index}'

        print(f'        __op_inst_{name}({executor_call_args});')
        print('        break;\n      }')

    print('    }\n  }\n}')

    print('} // namespace opcode')


def codegen_cpp_header():
    print('namespace opcode {')

    # Generate opcode enumerations
    print('enum {')
    for entry in codegen_entries:
        desc = entry['desc']
        name = entry['name']
        opcode = entry['opcode']
        print(f'  // {desc}')
        print(f'  k{name} = {opcode},\n')
    print('}; // enum <anonymous>\n')

    print('enum class OperandTypes : uint8_t {')
    print('  memop = 0,')
    for pair in cpp_disassembler_type_id_dict:
        if cpp_disassembler_type_id_dict[pair] != 0:
            print(f'  {pair} = {cpp_disassembler_type_id_dict[pair]},')
    print('}; // enum class OperandTypes')

    print('using MemOp = uint32_t;\n')

    print('''
struct OpcodeVectorTbl {
  uint32_t opcode;
  const char *name;
  int32_t fixed_size;
  struct {
    const char *type_name;
    int type_id;
  } operands[16];
};''')

    print('extern const OpcodeVectorTbl g_opcode_vector_tbl[];')
    print('extern const size_t g_opcode_vector_tbl_count;')
    print('} // namespace opcode')


if len(sys.argv) != 3:
    print(f'Usage: {sys.argv[0]} <target> <codegen file>')
    print('Available targets: cpp-header, cpp-impl, typescript')
    exit(1)

parse_entries(sys.argv[2])


print(generated_file_header_comment)
if sys.argv[1] == 'cpp-impl':
    codegen_cpp_impl()
elif sys.argv[1] == 'cpp-header':
    codegen_cpp_header()
elif sys.argv[1] == 'typescript':
    codegen_typescript()
