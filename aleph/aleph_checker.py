from enum import auto, StrEnum
from pathlib import Path

from .aleph_logger import AlephLogger
from .aleph_exception import AlephCheckerException

class CheckerMode(StrEnum):
	SILENT = auto()
	WARN = auto()
	EXCEPTION = auto()

class AlephChecker:
	log: AlephLogger

	def __init__(self, logger: AlephLogger) -> None:
		self.log = logger

	def check_mode(self, mode: CheckerMode, str_message: str) -> None:
		if mode == CheckerMode.WARN:
			self.log.W(str_message)
		if mode == CheckerMode.EXCEPTION:
			raise AlephCheckerException(str_message)

	def is_ok_str(self, arg_str: str | None, mode: CheckerMode = CheckerMode.SILENT) -> bool:
		if arg_str is None:
			self.check_mode(mode, 'String is None! String cannot be None.')
			return False

		arg_str_stripped = arg_str.strip()

		if len(arg_str_stripped) < 1:
			self.check_mode(mode, 'String is Empty! String cannot be Empty.')
			return False

		return True

	def is_ok_list[T](self, arg_list: list[T] | None, mode: CheckerMode = CheckerMode.SILENT) -> bool:
		if arg_list is None:
			self.check_mode(mode, 'List is None! List cannot be None.')
			return False

		if len(arg_list) < 1:
			self.check_mode(mode, 'List is Empty! List cannot be Empty.')
			return False

		for obj in arg_list:
			if obj is None:
				self.check_mode(mode, 'Some Object of List is None! Some Object of List cannot be None.')
				return False

		return True

	def is_ok_dict[K, V](self, arg_dict: dict[K, V] | None, mode: CheckerMode = CheckerMode.SILENT) -> bool:
		if arg_dict is None:
			self.check_mode(mode, 'Dictionary is None! Dictionary cannot be None.')
			return False

		if len(arg_dict) < 1:
			self.check_mode(mode, 'Dictionary is Empty! Dictionary cannot be Empty.')
			return False

		for k, v in arg_dict.items():
			if k is None:
				self.check_mode(mode, 'Some Key of Dictionary is None! Some Key of Dictionary cannot be None.')
				return False
			if v is None:
				self.check_mode(mode, 'Some Value of Dictionary is None! Some Value of Dictionary cannot be None.')
				return False

		return True

	def is_key_present[K, V](self, key: K, arg_dict: dict[K, V] | None, mode: CheckerMode = CheckerMode.SILENT) -> bool:
		if not self.is_ok_dict(arg_dict, mode):
			return False

		if arg_dict and key in arg_dict:
			return True

		self.check_mode(mode, f'Key not found in Dictionary: {key}')
		return False

	def check_files(self, p_is: list[Path], mode: CheckerMode = CheckerMode.SILENT) -> bool:
		if not self.is_ok_list(p_is, mode):
			return False

		for path in p_is:
			if not path.is_file():
				self.check_mode(mode, f'File does not exist or is the wrong type:\n{path}')
				return False

		return True

	def check_dirs(self, p_ds: list[Path], mode: CheckerMode = CheckerMode.SILENT) -> bool:
		if not self.is_ok_list(p_ds, mode):
			return False

		for path in p_ds:
			if not path.is_dir():
				self.check_mode(mode, f'Directory does not exist or is the wrong type:\n{path}')
				return False

		return True

	def check_ext(self, p_i: Path, extensions: list[str], mode: CheckerMode = CheckerMode.SILENT) -> bool:
		if not self.is_ok_list(extensions, mode):
			return False

		if p_i.suffix not in extensions:
			self.check_mode(mode, f'Extension {p_i} not in: [{", ".join(extensions)}]')
			return False

		return True
