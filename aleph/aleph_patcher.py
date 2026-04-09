from pathlib import Path

from .aleph_logger import AlephLogger
from .aleph_checker import AlephChecker, CheckerMode
from .aleph_filesystem import AlephFileSystem
from .aleph_exception import AlephPatcherException

class AlephPatcher:
	log: AlephLogger
	chk: AlephChecker
	fs: AlephFileSystem

	def __init__(self, logger: AlephLogger, checker: AlephChecker, filesystem: AlephFileSystem) -> None:
		self.log = logger
		self.chk = checker
		self.fs = filesystem

	def patch_text(self, markers: list[str], patches: list[str], in_text: str) -> str:
		self.chk.is_ok_str(in_text, CheckerMode.EXCEPTION)
		self.chk.is_ok_list(markers, CheckerMode.EXCEPTION)
		self.chk.is_ok_list(patches, CheckerMode.EXCEPTION)

		if len(markers) != len(patches):
			raise AlephPatcherException(f'Sizes patches/markers does not match! {len(markers)} != {len(patches)}')

		missing = next((m for m in markers if m not in in_text), None)
		if missing:
			raise AlephPatcherException(f'The marker is not in the text: {missing}')

		patched_text = in_text
		for marker, patch in zip(markers, patches):
			patched_text = patched_text.replace(marker, patch)

		return patched_text

	def patch_text_file(self, markers: list[str], patches: list[str], p_i: Path, p_o: Path) -> bool:
		orig = self.fs.read_text_file(p_i)
		self.chk.is_ok_str(orig, CheckerMode.EXCEPTION)

		return self.fs.write_text_file(p_o, self.patch_text(markers, patches, orig))
