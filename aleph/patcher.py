from pathlib import Path

from .logger import E
from .stringer import is_blank
from .filesystem import read_text_file
from .filesystem import write_text_file

def patch_text(markers: list[str], patches: list[str], str_in: str) -> str | None:
	if is_blank(str_in):
		E('The text is blank!')
		return None

	if len(markers) != len(patches):
		E('The size of the patches and markers does not match!')
		return None

	if not any(marker in str_in for marker in markers):
		E('No markers found in text.')
		return None

	text = str_in
	for marker, patch in zip(markers, patches):
		text = text.replace(marker, patch)

	return text

def patch_text_file(markers: list[str], patches: list[str], p_in: Path, p_out: Path) -> bool:
	original_text = read_text_file(p_in)
	if not original_text:
		return False

	text = patch_text(markers, patches, original_text)
	if not text:
		return False

	if not write_text_file(p_out, text):
		return False

	return True
