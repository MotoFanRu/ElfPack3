#!/usr/bin/env python3

"""
EP1/EP2/EM1/EM2 => EP3 Library Migration Tool

Parse Symbol and Assembler files recursively from a directory.
Extract function names and types, ensure uniqueness, and output combined file.
"""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent))

from aleph import Aleph
from aleph import AsmMode
from aleph import CheckerMode

def format_output_line(num, func_type, func_name, show_nums=False, show_types=False):
	if show_nums and show_types:
		return f'{num:04d} {func_type} {func_name}'
	if show_nums:
		return f'{num:04d} {func_name}'
	if show_types:
		return f'{func_type} {func_name}'
	return f'{func_name}'

def do_migration_libs_work(a: Aleph) -> bool:
	symbols: dict[str, str] = { }

	for suffix in ['.sym', '.asm']:
		for file_path in a.args.directory.rglob(f'*{suffix}'):
			a.log.I(f'Gathering: {file_path}')

			syms = [ ]
			if suffix == '.sym':
				syms = a.fsym.read_sym(file_path).syms
			elif suffix == '.asm':
				syms = a.fasm.read_asm(file_path, AsmMode.MCORE_ASM)

			for sym in syms:
				symbols[sym.name] = sym.type

	a.chk.is_ok_dict(symbols, CheckerMode.EXCEPTION)
	sorted_symbols = sorted(symbols.items(), key=lambda item: item[0].lower())

	content = ''
	for count, (func_name, func_typ) in enumerate(sorted_symbols, 1):
		content += format_output_line(count, func_typ, func_name, a.args.enumerate, a.args.show_types) + '\n'

	if a.args.output:
		a.chk.check_ext(a.args.output, ['.lst'], CheckerMode.EXCEPTION)
		return a.fs.write_text_file(a.args.output, content)
	else:
		print('\nResult:\n')
		print(content)
		print(f'Total: {len(sorted_symbols)}')

	return True

def fill_migration_libs_args(a: Aleph) -> None:
	epl = 'examples:\n'
	epl += f'  python {Path(__file__).name} symbols_directory\n'
	epl += f'  python {Path(__file__).name} symbols_directory -n -t\n'
	epl += f'  python {Path(__file__).name} symbols_directory -o result.lst\n'
	epl += f'  python {Path(__file__).name} symbols_directory -n -t -o result.lst\n'

	hlp = {
		'D': 'EP1/EP2/EM1/EM2 => EP3 Library Migration Tool, 01-Mar-2026',
		'P': 'directory to recursively walk through',
		'o': 'output file',
		'n': 'enumerate symbols',
		't': 'show types of symbols',
		'v': 'enable verbose output'
	}

	a.args_parser.add_epilog_description(epl, hlp['D'])
	a.args_parser.add_argument('directory', type=Path, help=hlp['P'])
	a.args_parser.add_argument('-o', '--output', type=Path, default=None, help=hlp['o'])
	a.args_parser.add_argument('-n', '--enumerate', action='store_true', help=hlp['n'])
	a.args_parser.add_argument('-t', '--show-types', action='store_true', help=hlp['t'])
	a.args_parser.add_argument('-v', '--verbose', action='store_true', help=hlp['v'])

def check_migration_libs_args(a: Aleph) -> None:
	if not a.chk.check_dirs([a.args.directory]):
		a.args_parser.error(f'Error: "{a.args.directory}" is not a directory!')

if __name__ == '__main__':
	aleph = Aleph()
	aleph.set_args(fill_migration_libs_args, check_migration_libs_args)
	aleph.main(do_migration_libs_work)
