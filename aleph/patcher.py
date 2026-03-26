from pathlib import Path

from .logger import E
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
