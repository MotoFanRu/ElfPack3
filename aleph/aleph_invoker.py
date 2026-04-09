import subprocess
from pathlib import Path

from .aleph_logger import AlephLogger
from .aleph_checker import AlephChecker, CheckerMode
from .aleph_filesystem import AlephFileSystem

class AlephInvoker:
	log: AlephLogger
	chk: AlephChecker
	fs: AlephFileSystem

	def __init__(self, logger: AlephLogger, checker: AlephChecker, filesystem: AlephFileSystem) -> None:
		self.log = logger
		self.chk = checker
		self.fs = filesystem

	def run_cmd(self, command: list[str | Path], p_o: Path | None = None) -> bool:
		self.log.D('Will execute external system command:')
		self.log.D(f"{' '.join(map(str, command))}")

		result = subprocess.run(
			command,
			stdout=subprocess.PIPE if p_o else None,
			text=True,
			errors="replace"
		)

		if p_o and self.chk.is_ok_str(result.stdout):
			if not self.fs.write_text_file(p_o, result.stdout):
				self.log.E(f'Cannot write output to: {p_o}')

		self.log.D(f'Result: {result.returncode}')
		return result.returncode == 0

	def run_cmd_input(self, p_is: list[Path], command: list[str | Path], p_o: Path | None = None) -> bool:
		if self.chk.check_files(p_is, CheckerMode.WARN):
			return self.run_cmd(command, p_o)
		return False
