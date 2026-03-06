from pathlib import Path

from .logger import D
from .logger import I
from .logger import E
from .hexer import format_32bit_addr
from .filesystem import read_text_file
from .filesystem import write_text_file

def patch_text(markers: list[str], patches: list[str], str_in: str) -> str | None:
	if len(markers) != len(patches):
		E('The size of the patches and markers does not match!')
		return None

	missing = next((m for m in markers if m not in str_in), None)
	if missing:
		E(f'The marker is not in the text: "{missing}"')
		return None

	text = str_in
	for marker, patch in zip(markers, patches):
		text = text.replace(marker, patch)

	return text

def patch_text_file(markers: list[str], patches: list[str], p_in: Path, p_out: Path) -> bool:
	original_text = read_text_file(p_in)
	if not original_text:
		return False

	return write_text_file(p_out, patch_text(markers, patches, original_text))

def patch_linker_script_with_addr_and_entry(p_in: Path, p_out: Path, addr: int, entry: str) -> bool:
	I(f'Patching linker script: "{p_in.name}" => "{p_out.name}"')
	D(f'"{p_in}" => "{p_out}"')

	markers = ['%ADDR_ENTRY%',          '%ENTRY_POINT%']
	patches = [format_32bit_addr(addr), entry]
	return patch_text_file(markers, patches, p_in, p_out)
