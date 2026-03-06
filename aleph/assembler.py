from pathlib import Path

from .skeleton import Definitions
from .filesystem import read_text_file
from .hexer import is_valid_32bit_addr

def generate_arm_assembler_listing(definitions: Definitions) -> str | None:
	pass

def generate_mcore_assembler_listing(definitions: Definitions) -> str | None:
	pass

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
