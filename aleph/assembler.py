from pathlib import Path

from .skeleton import Definitions
from .stringer import is_blank
from .patcher import patch_text
from .hexer import format_32bit_addr
from .hexer import is_valid_32bit_addr
from .filesystem import read_text_file
from .filesystem import write_text_file

def generate_assembler_definition(max_len: int, def_type: str, def_name: str, def_val: str | None) -> str | None:
	if is_blank(def_type) or is_blank(def_name) or is_blank(def_val):
		return None

	if def_type == 'C':
		def_type = 'D'

	return f'def_{def_type} {def_name + ",":<{max_len}} {def_val}'

def generate_assembler_listing(p_assembler_template: Path, definitions: Definitions | None) -> str | None:
	if not definitions:
		return None

	asm_template = read_text_file(p_assembler_template)
	if not asm_template:
		return None

	phone, firmware = definitions.head.pfw.split('_', 1)
	version = definitions.head.ver

	max_name_len = max((len(d.name) for d in definitions.defs), default=0) + len('EP3_firmware') + 4

	listing = [
		generate_assembler_definition(max_name_len, 'S', 'EP3_phone', f'"{phone}"'),
		generate_assembler_definition(max_name_len, 'S', 'EP3_firmware', f'"{firmware}"'),
		generate_assembler_definition(max_name_len, 'S', 'EP3_version', f'"{version}"')
	]

	for d in definitions.defs:
		listing.append(generate_assembler_definition(max_name_len, d.type, d.name, format_32bit_addr(d.addr)))

	markers = ['%ENTITY_LISTING%']
	patches = [''.join(map(lambda s: f'\t{s}\n', listing))]
	patch = patch_text(markers, patches, asm_template)
	if not patch:
		return None

	return patch + '\n'

def write_assembler_listing(p_asm_template: Path, p_asm_listing: Path, definitions: Definitions | None) -> bool:
	if not definitions:
		return False

	from .definitions import validate_addresses_and_names

	if not validate_addresses_and_names(definitions.defs, True):
		return False

	return write_text_file(p_asm_listing, generate_assembler_listing(p_asm_template, definitions))

def parse_equ_assembler_line(line: str, mcore_asm: bool = False) -> tuple[str, str, str] | None:
	line = line.strip()
	if not line or line.startswith(';') or line.startswith('#'):
		return None
	elif line.startswith('.equ ') or line.startswith('.set '):
		rest = line[5:].split(';', 1)[0].strip()
		if ',' in rest:
			asm_flavor = 'M' if mcore_asm else 'A'
			func_name = rest.split(',', 1)[0].strip()
			func_addr = rest.split(',', 1)[1].strip().split()[0]
			func_type = 'T' if rest.split(',', 1)[1].strip().endswith('| 1') else asm_flavor
			if func_name:
				return func_addr, func_type, func_name
	return None

def read_equ_assembler_listing(p_in: Path, mcore_asm: bool = False) -> list[Definitions.Definition] | None:
	content = read_text_file(p_in)
	if not content:
		return None
	lines_content = [line.strip() for line in content.splitlines()]

	definitions = []
	for line in lines_content:
		equ_line = parse_equ_assembler_line(line, mcore_asm)
		if equ_line:
			equ_addr, equ_type, equ_name = equ_line
			if is_valid_32bit_addr(equ_addr):
				definitions.append(Definitions.Definition(
					addr=int(equ_addr, 0),
					type=equ_type,
					name=equ_name,
				))
	return definitions
