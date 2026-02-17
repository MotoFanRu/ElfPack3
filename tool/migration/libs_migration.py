#!/usr/bin/env python3

"""
EP1/EP2/EM1/EM2 => EP3 Library Migration Tool

Parse Symbol and Assembler files recursively from a directory.
Extract function names and types, ensure uniqueness, and output combined file.
"""

import sys
import argparse

from pathlib import Path

def format_output_line(num, func_type, func_name, show_nums=False, show_types=False):
	if show_nums and show_types:
		return f'{num:04d} {func_type} {func_name}'
	if show_nums:
		return f'{num:04d} {func_name}'
	if show_types:
		return f'{func_type} {func_name}'
	return f'{func_name}'

def parse_line(line: str, suffix: str) -> tuple[str, str] | None:
	if suffix == '.sym':
		return parse_sym_line(line)
	elif suffix == '.asm' or suffix == '.S' or suffix == '.s':
		return parse_asm_line(line)
	return None

def parse_sym_line(line: str) -> tuple[str, str] | None:
	line = line.strip()
	if not line or line.startswith(';') or line.startswith('#'):
		return None
	parts = line.split(maxsplit=2)
	if len(parts) >= 3:
		func_type = parts[1]
		func_name = parts[2]
		if len(func_type) == 1 and func_type.isalpha():
			return func_type, func_name
	return None

def parse_asm_line(line: str) -> tuple[str, str] | None:
	line = line.strip()
	if not line or line.startswith(';') or line.startswith('#'):
		return None
	elif line.startswith('.equ '):
		rest = line[5:].split(';', 1)[0].strip()
		if ',' in rest:
			func_name = rest.split(',', 1)[0].strip()
			func_type = 'T' if rest.split(',', 1)[1].strip().endswith('| 1') else 'A'
			if func_name:
				return func_type, func_name
	return None

def main() -> None:
	parser = argparse.ArgumentParser(description='EP1/EP2/EM1/EM2 => EP3 Library Migration Tool')
	parser.add_argument('directory', type=Path, help='Directory to recursively walk through')
	parser.add_argument('-o', '--output', type=Path, default=None, help='Output file')
	parser.add_argument('-n', '--enumerate', action='store_true', help='Enumerate symbols')
	parser.add_argument('-t', '--show-types', action='store_true', help='Show types of symbols')
	args = parser.parse_args()

	if not args.directory.is_dir():
		print(f'Error: "{args.directory}" is not a directory!', file=sys.stderr)
		sys.exit(1)

	symbols: dict[str, str] = {}

	for suffix in ('.sym', '.asm'):
		for file_path in args.directory.rglob(f'*{suffix}'):
			try:
				with open(file_path, 'r', encoding='utf-8', errors='replace') as f:
					for line in f:
						res = parse_line(line, suffix)
						if res:
							func_type, func_name = res
							symbols[func_name] = func_type
			except Exception as e:
				print(f'Warning: could not read {file_path}: {e}', file=sys.stderr)

	sorted_symbols = sorted(symbols.items(), key=lambda item: item[0].lower())

	if args.output:
		try:
			with open(args.output, 'w', encoding='utf-8', newline='\n') as f:
				for count, (func_name, func_typ) in enumerate(sorted_symbols, 1):
					f.write(format_output_line(count, func_typ, func_name, args.enumerate, args.show_types) + '\n')
			print(f'Output written to {args.output}', file=sys.stderr)
		except Exception as e:
			print(f'Error writing to {args.output}: {e}', file=sys.stderr)
			sys.exit(1)
	else:
		for count, (func_name, func_typ) in enumerate(sorted_symbols, 1):
			print(format_output_line(count, func_typ, func_name, args.enumerate, args.show_types))

if __name__ == '__main__':
	main()
