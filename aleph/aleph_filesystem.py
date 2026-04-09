import shutil
from pathlib import Path

from .aleph_logger import AlephLogger
from .aleph_checker import AlephChecker, CheckerMode
from .aleph_exception import AlephFileException

class AlephFileSystem:
	log: AlephLogger
	chk: AlephChecker

	def __init__(self, logger: AlephLogger, checker: AlephChecker) -> None:
		self.log = logger
		self.chk = checker

	def read_text_file(self, p_i: Path, encoding: str = 'utf-8') -> str:
		self.chk.check_files([p_i], CheckerMode.EXCEPTION)

		text = p_i.read_text(encoding)
		self.chk.is_ok_str(text, CheckerMode.WARN)

		return text

	def read_text_lines(self, p_i: Path, encoding: str = 'utf-8') -> list[str]:
		content = self.read_text_file(p_i, encoding)
		lines = []
		for line in content.splitlines():
			if self.chk.is_ok_str(line):
				lines.append(line)
		self.chk.is_ok_list(lines, CheckerMode.EXCEPTION)
		return lines

	def write_text_file(self, p_o: Path, text: str, encoding: str = 'utf-8') -> bool:
		try:
			self.chk.is_ok_str(text, CheckerMode.EXCEPTION)
			p_o.write_text(text, encoding)
		except Exception as e:
			self.log.E(f'Cannot write file: {p_o.name}, {e}')
			return False
		return True

	def create_dir(self, p_d: Path, delete_files: bool = False) -> None:
		if self.chk.check_dirs([p_d]):
			for item in p_d.iterdir():
				if item.is_file() or item.is_symlink():
					if delete_files:
						self.log.D(f'Deleting file: {item}')
						item.unlink()
		else:
			p_d.mkdir(parents=True, exist_ok=True)

	def copy_file(self, p_i: Path, p_o: Path, overwrite: bool = False, log_it: bool = False) -> None:
		if log_it:
			self.log.I(f'  Copy {p_i.name} => {p_o.name}')
		self.log.D(f'{p_i} => {p_o}')

		self.chk.check_files([p_i], CheckerMode.EXCEPTION)

		if not overwrite and p_o.exists():
			raise AlephFileException(f'File is exist: {p_i}')

		p_o.parent.mkdir(parents=True, exist_ok=True)
		shutil.copy2(p_i, p_o)
