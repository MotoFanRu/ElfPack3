import subprocess

from pathlib import Path

from .logger import D
from .filesystem import check_files

def invoke_run(command: list[str | Path], file_output: Path | None = None) -> bool:
	D('Will execute external system command:')
	D(f"{' '.join(map(str, command))}")

	result = subprocess.run(
		command,
		stdout=subprocess.PIPE if file_output else None,
		text=True,
		errors="replace"
	)

	if file_output and result.stdout:
		file_output.write_text(result.stdout)

	D(f'Result: {result.returncode}')
	return result.returncode == 0

def invoke_run_input(p_in: list[Path], command: list[str | Path], file_output: Path | None = None) -> bool:
	if check_files(p_in, warn=True):
		return invoke_run(command, file_output)
	return False
