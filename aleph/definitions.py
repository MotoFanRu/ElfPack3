from pathlib import Path
from datetime import datetime

from .logger import D
from .logger import I
from .logger import W
from .logger import E
from .soc import CPU_ARCH
from .hexer import format_32bit_addr
from .hexer import is_valid_32bit_addr
from .patcher import patch_text
from .stringer import is_blank, is_blank_list
from .stringer import is_equal_lines
from .stringer import find_duplicates
from .stringer import find_line_starts_with
from .skeleton import Definitions
from .filesystem import read_text_file
from .filesystem import write_text_file
from .timedate import date_format
from .assembler import read_equ_assembler_listing

SYM_HEADER_TEMPLATE = """#<SYMDEFS>#symdef-file
# SYMDEFS ADS HEADER

"""

DEF_HEADER_TEMPLATE = """# Aleph-generated definitions
# https://github.com/MotoFanRu/P2K-ELF-SDK
#
# PFW %pfw%
# VER %ver%
# CPU %cpu%

"""

DEF_TYPES = ['A', 'T', 'D', 'C', 'M']

def get_definitions_by_addr(defs: list[Definitions.Definition], addr: int) -> list[Definitions.Definition]:
	return [d for d in defs if d.addr == addr]

def get_definitions_by_types(defs: list[Definitions.Definition], types: list[str]) -> list[Definitions.Definition]:
	return [d for d in defs if d.type in types]

def get_definitions_by_name(defs: list[Definitions.Definition], name: str) -> list[Definitions.Definition]:
	return [d for d in defs if d.name == name]

def format_definition_string(definition: Definitions.Definition) -> str:
	return f'{format_32bit_addr(definition.addr)} {definition.type} {definition.name}'

def validate_header_info(lines: tuple[str, str, str]) -> bool:
	if not lines:
		return False

	pfw, ver, cpu = lines

	if len(pfw.split('_', 1)) != 2:
		E(f'PFW (phone and firmware) header string is wrong: "{pfw}"')
		E('Should be in "C650_R365_G_0B.D3.08R" format.')
		return False

	if (len(ver.split('_', 1)) != 2) and (not ver.startswith('EP3_')):
		E(f'VER (version) header string is wrong: "{ver}"')
		E('Should be in "EP3_01-Feb-2025" format.')
		return False

	if cpu not in CPU_ARCH:
		E(f'CPU (architecture) header string is wrong: "{cpu}"')
		E(f'Should be in [{", ".join(CPU_ARCH)}] scope.')
		return False

	return True

def find_header_lines(lines_c, required) -> tuple[str, str, str] | None:
	results = []
	for prefix, err_msg in required:
		line = find_line_starts_with(lines_c, prefix)
		if not line:
			E(err_msg)
			return None
		results.append(line.replace(prefix, ''))
	return tuple(results)

def parse_validate_symbols_header(content: str) -> tuple[str, str, str] | None:
	lines_t = [line.strip() for line in SYM_HEADER_TEMPLATE.splitlines()]
	lines_c = [line.strip() for line in content.splitlines()]

	if not is_equal_lines(lines_t, lines_c, (0, 3)):
		D('No proper header comment.')

	required = [
		('# Phone: ', 'Cannot find phone line in the header.'),
		('# Firmware: ', 'Cannot find firmware line in the header.'),
	]
	lines = find_header_lines(lines_c, required)
	if len(lines) != 2:
		E(f'Cannot find phone firmware in header: "{lines}"')
		return None

	# Use some default values.
	return '_'.join(lines), 'EP3_' + date_format(datetime.now()), 'ARM7'

def parse_validate_definitions_header(content: str) -> tuple[str, str, str] | None:
	lines_t = [line.strip() for line in DEF_HEADER_TEMPLATE.splitlines()]
	lines_c = [line.strip() for line in content.splitlines()]

	if not is_equal_lines(lines_t, lines_c, (0, 3)):
		D('No proper header comment.')

	if not is_equal_lines(lines_t, lines_c, (6, 6)):
		D('No blank line separator of header and body found.')

	required = [
		('# PFW ', 'Cannot find PFW line (phone and firmware) in the header.'),
		('# VER ', 'Cannot find VER line (version) in the header.'),
		('# CPU ', 'Cannot find CPU line (architecture) in the header.')
	]
	lines = find_header_lines(lines_c, required)
	if not validate_header_info(lines):
		return None

	return lines

def parse_validate_definition_line(line: str) -> tuple[int, str, str] | None:
	parts = line.split()

	if len(parts) != 3:
		E(f'Cannot parse line: "{line}"')
		return None

	def_addr, def_type, def_name = parts

	if not is_valid_32bit_addr(def_addr):
		E(f'Invalid 32-bit address: {def_addr} in the line: {line}')
		return None

	if def_type not in DEF_TYPES:
		E(f'Unknown type "{def_type}" in the line: {line}')
		return None

	if len(def_name) > 80:
		E(f'Too long name "{def_name}" > 80 in the line: {line}')
		return None

	return int(def_addr, 0), def_type, def_name

def parse_validate_definitions_body(content: str) -> list[Definitions.Definition] | None:
	lines_content = [line.strip() for line in content.splitlines()]
	definitions = []
	for line in lines_content:
		# Skip blank lines and comments.
		if (not is_blank(line)) and (not line.startswith('#')):
			valid_line = parse_validate_definition_line(line)
			if not valid_line:
				return None
			def_addr, def_type, def_name = valid_line
			definitions.append(Definitions.Definition(
				addr=def_addr,
				type=def_type,
				name=def_name
			))
	return definitions

def validate_addresses_and_names(defs: list[Definitions.Definition]) -> bool:
	addrs = []
	names = []
	for d in defs:
		# It is better to check all types.
		# if d.type not in ['D', 'C']: # Skip constants and data addresses.
		addrs.append(d.addr)
		names.append(d.name)

	addrs_d = find_duplicates(addrs)
	if len(addrs_d) > 0:
		W('Duplicate addresses found:')
		for addr in addrs_d:
			W('')
			for d in get_definitions_by_addr(defs, addr):
				W(f'{format_definition_string(d)}')

	names_d = find_duplicates(names)
	if len(names_d) > 0:
		E('Duplicate names found:')
		for name in names_d:
			E('')
			for d in get_definitions_by_name(defs, name):
				E(f'{format_definition_string(d)}')
		return False

	return True

def sort_definitions(defs: list[Definitions.Definition], by_addresses: bool) -> list[Definitions.Definition] | None:
	if len(defs) == 0:
		return None

	consts = get_definitions_by_types(defs, ['C'])
	addrs = get_definitions_by_types(defs, ['D'])
	funcs = get_definitions_by_types(defs, ['A', 'T', 'M'])

	sort_key = (lambda e: e.addr) if by_addresses else (lambda e: e.name.lower())
	for target_list in [consts, addrs, funcs]:
		target_list.sort(key=sort_key)

	return consts + addrs + funcs

def parse_definitions_symbols_content(content: str, sym_format: bool = False) -> Definitions | None:
	if not content or is_blank(content):
		return None
	content = content.strip()

	header = parse_validate_symbols_header(content) if sym_format else parse_validate_definitions_header(content)
	if not header:
		return None
	pfw_line, ver_line, cpu_line = header

	definitions = parse_validate_definitions_body(content)
	if (not definitions) or (len(definitions) == 0):
		return None

	if not validate_addresses_and_names(definitions):
		return None

	definitions = sort_definitions(definitions, False)

	return Definitions(
		head=Definitions.Header(
			pfw=pfw_line,
			ver=ver_line,
			cpu=cpu_line,
		),
		defs=definitions,
	)

def read_definitions(p_in: Path) -> Definitions | None:
	return parse_definitions_symbols_content(read_text_file(p_in), (p_in.suffix == '.sym'))

def generate_definitions_content(definitions: Definitions, sym_format: bool = False) -> str | None:
	if not definitions:
		return None

	header = ''
	if sym_format:
		header += SYM_HEADER_TEMPLATE

	markers = ['%pfw%',              '%ver%',              '%cpu%']
	patches = [definitions.head.pfw, definitions.head.ver, definitions.head.cpu]
	header += patch_text(markers, patches, DEF_HEADER_TEMPLATE)

	content = '\n'.join(format_definition_string(d) for d in definitions.defs)

	return header + content + '\n'

def convert_def_to_sym(defs: list[Definitions.Definition]) -> list[Definitions.Definition]:
	if defs or len(defs) > 0:
		# C => D
		# M => A
		for d in defs:
			if d.type == 'C':
				d.type = 'D'
			elif d.type == 'M':
				d.type = 'A'
	return defs

def write_definitions(p_out: Path, definitions: Definitions) -> bool:
	sym_format = p_out.suffix == '.sym'
	if sym_format:
		definitions.defs = convert_def_to_sym(definitions.defs)
	return write_text_file(p_out, generate_definitions_content(definitions, sym_format))

def pack_def_string(d: Definitions.Definition, skip_addr: bool = False, skip_type: bool = False) -> str | None:
	if not skip_addr and not skip_type:
		return format_definition_string(d)
	if skip_addr and skip_type:
		return d.name
	if skip_type:
		return f'{d.addr} {d.name}'
	if skip_addr:
		return f'{d.type} {d.name}'
	return None

def unpack_name_from_def_string(def_string: str, skip_addr: bool = False, skip_type: bool = False) -> str | None:
	if not skip_addr and not skip_type:
		def_addr, def_type, def_name = def_string.split()
		return def_name
	if skip_addr and skip_type:
		return def_string
	if skip_type:
		def_addr, def_name = def_string.split()
		return def_name
	if skip_addr:
		def_type, def_name = def_string.split()
		return def_name
	return None

def comparator_report(
	defs_a: Definitions, defs_b: Definitions, skip_addr: bool = False, skip_type: bool = False
) -> list[Definitions.Definition] | None:
	if not defs_a.head.pfw == defs_b.head.pfw:
		I('Header phone firmware mismatch:')
		I('')
		I(f'{defs_a.head.pfw} {defs_b.head.pfw}')
	if not defs_a.head.ver == defs_b.head.ver:
		I('Header version mismatch:')
		I('')
		I(f'{defs_a.head.ver} {defs_b.head.ver}')
	if not defs_a.head.cpu == defs_b.head.cpu:
		I('Header CPU architecture mismatch:')
		I('')
		I(f'{defs_a.head.cpu} {defs_b.head.cpu}')

	ds_a = [pack_def_string(d, skip_addr=skip_addr, skip_type=skip_type) for d in defs_a.defs]
	ds_b = [pack_def_string(d, skip_addr=skip_addr, skip_type=skip_type) for d in defs_b.defs]

	set_a = set(ds_a)
	unique_to_b = [s for s in ds_b if s not in set_a]

	different = []
	for line in unique_to_b:
		for d in get_definitions_by_name(defs_b.defs, unpack_name_from_def_string(line, skip_addr, skip_type)):
			different.append(d)

	if len(different) > 0:
		I('Definitions mismatch:')
		I('')
		for d in different:
			I(format_definition_string(d))
		return different

	I('No mismatch detected.')
	return None

def compare_definitions(
	p_in_a: Path, p_in_b: Path,
	skip_addr: bool = False, skip_type: bool = False,
	swap: bool = False
) -> bool:
	defs_a = read_definitions(p_in_b) if swap else read_definitions(p_in_a)
	defs_b = read_definitions(p_in_a) if swap else read_definitions(p_in_b)
	if not defs_a or not defs_b:
		return False

	diffs = comparator_report(defs_a, defs_b, skip_addr, skip_type)
	if not diffs:
		return False

	return True

def convert_asm_to_def(p_in: Path, p_out: Path, mcore_asm: bool = False) -> bool:
	if not p_in.suffix in ['.S', '.s', '.asm']:
		E(f'Wrong asm file suffix format: "{p_in.suffix}", should be .S, .s, or .asm')
		return False

	defs = read_equ_assembler_listing(p_in, mcore_asm)
	if not defs or is_blank_list(defs):
		E('No proper equ strings found in assembler listing.')
		return False

	d = Definitions(
		head=Definitions.Header(
			pfw='UNK_Unknown',
			ver='UNK_' + date_format(datetime.now()),
			cpu='UNK',
		),
		defs=defs,
	)
	return write_definitions(p_out, d)
