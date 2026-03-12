import shutil
from pathlib import Path
from typing import Callable

from .logger import D
from .logger import I
from .logger import W
from .logger import E
from .stringer import is_blank

def check_paths_aux(paths: Path | list[Path], check_method: Callable[[Path], bool], warn: bool = False) -> bool:
	if isinstance(paths, Path):
		paths = [paths]

	for path in paths:
		if path is None or not check_method(path):
			if warn:
				entity = 'Directory' if path.is_dir() else 'File'
				W(f'{entity} "{path}" does not exist or is the wrong type.')
			return False

	return True

def check_dirs(paths: Path | list[Path], warn: bool = False) -> bool:
	return check_paths_aux(paths, lambda p: p.is_dir(), warn)

def check_files(paths: Path | list[Path], warn: bool = False) -> bool:
	return check_paths_aux(paths, lambda p: p.is_file(), warn)

def create_clean_dir(dir_path: Path, delete_files: bool = True) -> bool:
	if check_dirs(dir_path):
		for item in dir_path.iterdir():
			if item.is_file() or item.is_symlink():
				if delete_files:
					D(f'Deleting "{item.name}" file.')
					try:
						item.unlink()
					except Exception as e:
						E(f'Failed to delete file {item}: {e}')
						return False
	else:
		dir_path.mkdir(parents=True, exist_ok=True)

	return True

def read_text_file(file_path_in: Path) -> str | None:
	if not check_files(file_path_in, True):
		return None
	try:
		text = file_path_in.read_text(encoding="utf-8").strip()
		if not text or is_blank(text):
			E(f'Text file is empty: "{file_path_in}"')
			return None
		return text
	except Exception as e:
		E(f'Failed to read input file "{file_path_in}": "{e}"')
		return None

def write_text_file(file_path_out: Path, text: str) -> bool:
	try:
		if is_blank(text):
			E(f'Cannot write empty content to file: "{file_path_out}"')
			return False
		file_path_out.write_text(text, encoding="utf-8")
		return True
	except Exception as e:
		E(f'Failed to write output file "{file_path_out}": "{e}"')
	return False

def copy_file(path_in: Path, path_out: Path, overwrite: bool = True, log: bool = False) -> bool:
	if log:
		I(f'  Copying "{path_in.name}" => "{path_out.name}"')
	D(f'"{path_in}" => "{path_out}"')
	if not check_files([path_in], True):
		return False

	if not overwrite and path_out.exists():
		E(f'File is exist: "{path_in}"')
		return False

	try:
		path_out.parent.mkdir(parents=True, exist_ok=True)
	except Exception as e:
		E(f'Cannot create directories of "{path_out}" path, error: "{e}"')
		return False

	try:
		shutil.copy2(path_in, path_out)
	except Exception as e:
		E(f'Cannot copy file "{path_in}" => "{path_out}", error: "{e}"')
		return False

	return True
