from collections import Counter

from .aleph_logger import AlephLogger
from .aleph_checker import AlephChecker, CheckerMode

class AlephStringer:
	log: AlephLogger
	chk: AlephChecker

	def __init__(self, logger: AlephLogger, checker: AlephChecker) -> None:
		self.log = logger
		self.chk = checker

	def is_equal_strings(self, lines_a: list[str], lines_b: list[str], start_end=tuple[int, int]) -> bool:
		self.chk.is_ok_list(lines_a, CheckerMode.EXCEPTION)
		self.chk.is_ok_list(lines_b, CheckerMode.EXCEPTION)

		start, end = start_end
		if start < 0 or end > len(lines_a) or end > len(lines_b):
			return False

		for i in range(start, end):
			if lines_a[i].strip() != lines_b[i].strip():
				return False

		return True

	def find_line(self, lines: list[str], starts_with: str) -> str | None:
		self.chk.is_ok_list(lines, CheckerMode.EXCEPTION)
		self.chk.is_ok_str(starts_with, CheckerMode.EXCEPTION)

		for line in lines:
			if line.strip().startswith(starts_with.strip()):
				return line

		return None

	def find_duplicates[T](self, items: list[T]) -> list[T] | None:
		self.chk.is_ok_list(items, CheckerMode.EXCEPTION)

		counts = Counter(items)
		return [item for item, count in counts.items() if count > 1]
