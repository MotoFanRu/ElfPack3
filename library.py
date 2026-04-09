#!/usr/bin/env python3

import sys
from pathlib import Path

from aleph import Aleph, CheckerMode, AsmMode

def convertor(a: Aleph) -> bool:
	file_path_a = a.args.input
	file_path_b = a.args.convert
	ext_a, ext_b = file_path_a.suffix, file_path_b.suffix

	a.log.I(f'Convert: {file_path_a} => {file_path_b}')

	if ext_a == '.def' and ext_b == '.sym':
		return a.fsym.write_sym(file_path_b, a.fdef.read_def(file_path_a))
	elif ext_a == '.sym' and ext_b == '.def':
		return a.fdef.write_def(file_path_b, a.fsym.read_sym(file_path_a))
	elif ext_a in {'.asm', '.s', '.S'} and ext_b == '.def':
		model = a.mdef.gen_model()
		mode = AsmMode.MCORE_ASM if a.args.mcore else AsmMode.ARM_ASM
		model.syms = a.fasm.read_asm(file_path_a, mode)
		return a.fdef.write_def(file_path_b, model)
	elif ext_a == '.def' and ext_b in {'.asm', '.s', '.S'}:
		model = a.fdef.read_def(file_path_a)
		if not model.head.pfw in a.recipe.recipes.keys():
			a.log.E(f'Cannot find recipe for {model.head.pfw}')
			return False
		return a.fasm.write_asm(a.recipe.recipes[model.head.pfw].soc.asm, file_path_b, model)

	a.log.E(f'Unknown convert scheme: {ext_a} => {ext_b}')
	return False

def comparator(a: Aleph) -> bool:
	file_path_a = a.args.compare if a.args.swap else a.args.input
	file_path_b = a.args.input if a.args.swap else a.args.compare
	ext_a, ext_b = file_path_a.suffix, file_path_b.suffix

	a.log.I(f'Compare: {file_path_a} => {file_path_b}')

	readers = {
		'.def': a.fdef.read_def,
		'.sym': a.fsym.read_sym,
		'.api': a.fapi.read_api,
	}
	if ext_a not in readers or ext_b not in readers:
		a.log.E(f'Unknown compare scheme: {ext_a} => {ext_b}')
		return False
	model_a = readers[ext_a](file_path_a)
	model_b = readers[ext_b](file_path_b)

	if ext_a == '.api' or ext_b == '.api':
		return a.mdef.compare(model_a, model_b, True, True, True, True)
	return a.mdef.compare(model_a, model_b, a.args.skip_addr, a.args.skip_type)

def regenerator(a: Aleph) -> bool:
	for file_path in a.consts.P2K_SDK_RES.rglob('*.def'):
		if a.args.format_only:
			a.log.I(f'Format: {file_path}')
			if not a.fdef.format_def(file_path, file_path, a.args.warn_duplicates, a.args.sort_address):
				return False
		elif a.args.delete_name:
			a.fdef.del_def(file_path, a.args.delete_name)
		else:
			# TODO: Compile Library here?
			a.log.I(f'Not implemented, Regen: {file_path}')
	return True

def recursive_conversion(a: Aleph) -> bool:
	if not a.chk.check_dirs([a.args.recursive_convert], CheckerMode.WARN):
		return False

	for file_path in a.args.recursive_convert.rglob('*.sym'):
		p_o = file_path.with_name('ep3.def')
		a.log.I(f'Convert: {file_path} => {p_o.name}')
		if not a.fdef.write_def(p_o, a.fsym.read_sym(file_path)):
			return False

	return True

def do_library_work(a: Aleph) -> bool:
	if a.args.regenerate:
		return regenerator(a)
	elif a.args.compare:
		return comparator(a)
	elif a.args.convert:
		return convertor(a)
	elif a.args.recursive_convert:
		return recursive_conversion(a)
	# Should be the last branch.
	elif a.args.input:
		a.log.I(f'Format: {a.args.input}')
		return a.fdef.format_def(a.args.input, a.args.input, a.args.warn_duplicates, a.args.sort_address)
	return True

def fill_library_args(a: Aleph) -> None:
	epl = 'examples:\n\n'
	epl += f'  # Regenerate libraries (format only def files, warn on duplicates, delete symbol)\n'
	epl += f'  python {Path(__file__).name} -r\n'
	epl += f'  python {Path(__file__).name} -r -f -w\n'
	epl += f'  python {Path(__file__).name} -r -d some_function_name\n\n'
	epl += f'  # Format, validate, and sort definitions file\n'
	epl += f'  python {Path(__file__).name} -i ep3.def\n'
	epl += f'  python {Path(__file__).name} -i ep3.def -a -w\n\n'
	epl += f'  # Convert (conversion scheme is determined by file extensions)\n'
	epl += f'  python {Path(__file__).name} -i ep3.sym -t ep3.def\n'
	epl += f'  python {Path(__file__).name} -i ep3.def -t ep3.sym\n'
	epl += f'  python {Path(__file__).name} -i ep3.asm -t ep3.def -M\n'
	epl += f'  python {Path(__file__).name} -i ep3.def -t ep3.S\n\n'
	epl += f'  # Recursive sym => def conversion in given directory\n'
	epl += f'  python {Path(__file__).name} -l directory\n\n'
	epl += f'  # Compare (skip addresses, skip types, swap input/compared)\n'
	epl += f'  python {Path(__file__).name} -i ep3.def -c ep3.api\n'
	epl += f'  python {Path(__file__).name} -i ep3_a.def -c ep3_b.def\n'
	epl += f'  python {Path(__file__).name} -i ep3_a.def -c ep3_b.def -S\n'
	epl += f'  python {Path(__file__).name} -i ep3_a.def -c ep3_b.def -A -T -S\n\n'
	hlp = {
		'D': 'EP3 Library Generation Tool for Motorola phones on P2K platform, 01-Mar-2026',
		'r': 'recursive libraries regenerator',
		'f': 'format only during regeneration',
		'i': 'input file',
		'a': 'sort by addresses',
		'w': 'warn on duplicates',
		't': 'convert input file to output file',
		'M': 'M-CORE assembler',
		'l': 'recursive sym => def conversion in directory',
		'c': 'compare file with input file',
		'A': 'skip addresses during comparison',
		'T': 'skip types during comparison',
		'S': 'swap input and compared files',
		'd': 'delete entity by name (use with -r)',
		'v': 'enable verbose output'
	}

	## TODO:
	# def => asm then compile a, so libraries
	# def => recursive compile libraries, bump date
	# so, elf => def or api listing

	a.args_parser.add_epilog_description(epl, hlp['D'])
	a.args_parser.add_argument('-r', '--regenerate', action='store_true', help=hlp['r'])
	a.args_parser.add_argument('-f', '--format-only', action='store_true', help=hlp['f'])
	a.args_parser.add_argument('-i', '--input', type=Path, default=None, help=hlp['i'])
	a.args_parser.add_argument('-a', '--sort-address', action='store_true', help=hlp['a'])
	a.args_parser.add_argument('-w', '--warn-duplicates', action='store_true', help=hlp['w'])
	a.args_parser.add_argument('-t', '--convert', type=Path, default=None, help=hlp['t'])
	a.args_parser.add_argument('-M', '--mcore', action='store_true', help=hlp['M'])
	a.args_parser.add_argument('-l', '--recursive-convert', type=Path, default=None, help=hlp['l'])
	a.args_parser.add_argument('-c', '--compare', type=Path, default=None, help=hlp['c'])
	a.args_parser.add_argument('-A', '--skip_addr', action='store_true', help=hlp['A'])
	a.args_parser.add_argument('-T', '--skip_type', action='store_true', help=hlp['T'])
	a.args_parser.add_argument('-S', '--swap', action='store_true', help=hlp['S'])
	a.args_parser.add_argument('-d', '--delete-name', type=str, default=None, help=hlp['d'])
	a.args_parser.add_argument('-v', '--verbose', action='store_true', help=hlp['v'])

def check_library_args(a: Aleph) -> None:
	if len(sys.argv) == 1:
		a.args_parser.error()

if __name__ == '__main__':
	aleph = Aleph()
	aleph.set_args(fill_library_args, check_library_args)
	aleph.main(do_library_work)
