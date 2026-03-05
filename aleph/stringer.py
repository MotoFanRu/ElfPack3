from .logger import D

from collections import Counter

def is_blank(text: str) -> bool:
	return not (text and text.strip())

def is_blank_list[T](lines: list[T]) -> bool:
	return not (lines and len(lines) > 0)

def is_equal_lines(lines_a: list[str], lines_b: list[str], start_end=tuple[int, int]) -> bool:
	start, end = start_end
	if start < 0 or end > len(lines_a) or end > len(lines_b):
		return False

	for i in range(start, end):
		# D(f'{lines_a[i]} | {lines_b[i]}')
		if lines_a[i].strip() != lines_b[i].strip():
			return False

	return True

def find_line_starts_with(lines: list[str], starts_with: str) -> str | None:
	if is_blank_list(lines):
		return None

	for line in lines:
		if line.strip().startswith(starts_with.strip()):
			return line

	return None

def find_duplicates[T](items: list[T]) -> list[T] | None:
	counts = Counter(items)
	return [s for s, count in counts.items() if count > 1]
