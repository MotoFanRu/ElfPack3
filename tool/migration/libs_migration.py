#!/usr/bin/env python3

"""
EP1/EP2/EM1/EM2 => EP3 Library Migration Tool

Parse Symbol and Assembler files recursively from a directory.
Extract function names and types, ensure uniqueness, and output combined file.
"""

import sys
from pathlib import Path
from datetime import datetime
from argparse import Namespace
from argparse import RawDescriptionHelpFormatter

sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent))

from aleph import *

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

def do_work(args: Namespace) -> bool:
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
				W(f'Warning: could not read {file_path}: {e}')

	sorted_symbols = sorted(symbols.items(), key=lambda item: item[0].lower())

	if args.output:
		try:
			with open(args.output, 'w', encoding='utf-8', newline='\n') as f:
				for count, (func_name, func_typ) in enumerate(sorted_symbols, 1):
					f.write(format_output_line(count, func_typ, func_name, args.enumerate, args.show_types) + '\n')
			I(f'Output written to {args.output}')
		except Exception as e:
			E(f'Error writing to {args.output}: {e}')
			return False
	else:
		for count, (func_name, func_typ) in enumerate(sorted_symbols, 1):
			print(format_output_line(count, func_typ, func_name, args.enumerate, args.show_types))

	return True

def args_and_help() -> Namespace:
	class Args(Arguments):
		def check_args(self, args: Namespace) -> None:
			if not args.directory.is_dir():
				self.error(f'Error: "{args.directory}" is not a directory!')

	epl = 'examples:\n'
	epl += f'  python {Path(__file__).name} symbols_directory\n'
	epl += f'  python {Path(__file__).name} symbols_directory -n -t\n'
	epl += f'  python {Path(__file__).name} symbols_directory -o result.sym\n'
	epl += f'  python {Path(__file__).name} symbols_directory -n -t -o result.sym\n'
	hlp = {
		'D': 'EP1/EP2/EM1/EM2 => EP3 Library Migration Tool, 01-Mar-2026',
		'P': 'directory to recursively walk through',
		'o': 'output file',
		'n': 'enumerate symbols',
		't': 'show types of symbols',
		'v': 'enable verbose output'
	}

	parser = Args(description=hlp['D'], epilog=epl, formatter_class=RawDescriptionHelpFormatter)
	parser.add_argument('directory', type=Path, help=hlp['P'])
	parser.add_argument('-o', '--output', type=Path, default=None, help=hlp['o'])
	parser.add_argument('-n', '--enumerate', action='store_true', help=hlp['n'])
	parser.add_argument('-t', '--show-types', action='store_true', help=hlp['t'])
	parser.add_argument('-v', '--verbose', action='store_true', help=hlp['v'])
	return parser.parse_and_check_args()

def main() -> None:
	args = args_and_help()
	set_logger(args.verbose)

	start_time = datetime.now()

	do_work(args)

	I(f'Time elapsed: "{elapsed_format(datetime.now() - start_time)}".')

if __name__ == '__main__':
	main()
