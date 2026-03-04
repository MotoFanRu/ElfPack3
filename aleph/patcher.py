from pathlib import Path

from .logger import E
from .filesystem import check_files

def patch_text_file(markers: list[str], patches: list[str], p_in: Path, p_out: Path) -> bool:
	if len(markers) != len(patches):
		E('The size of the patches and markers does not match!')
		return False

	if not check_files(p_in, True):
		return False

	try:
		original_text = p_in.read_text(encoding="utf-8")
	except Exception as e:
		E(f'Failed to read input file {p_in}: {e}')
		return False

	if not any(marker in original_text for marker in markers):
		E(f'No markers found in file: {p_in}')
		return False

	text = original_text
	for marker, patch in zip(markers, patches):
		text = text.replace(marker, patch)

	try:
		p_out.write_text(text, encoding="utf-8")
		return True
	except Exception as e:
		E(f'Failed to write output file {p_out}: {e}')
		return False
