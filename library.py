#!/usr/bin/env python3

import sys
from pathlib import Path
from datetime import datetime
from argparse import Namespace
from argparse import RawDescriptionHelpFormatter

from config import *

def assembler_helper(p_in: Path, p_out: Path) -> bool:
	d = read_definitions(p_in)
	if not d:
		return False
	if not d.head.pfw in RECIPES.keys():
		E(f'Cannot find recipe for "{d.head.pfw}" in config!')
		return False
	return write_assembler_listing(RECIPES[d.head.pfw].soc.asm, p_out, d)

def convertor_helper(args: Namespace) -> bool:
	ext_a, ext_b = args.input.suffix, args.convert.suffix

	if {ext_a, ext_b} == {'.sym', '.def'}:
		return write_definitions(args.convert, read_definitions(args.input))
	if ext_a in  {'.asm', '.s', '.S'} and ext_b == '.def':
		return convert_asm_to_def(args.input, args.convert, args.mcore)
	if ext_a == '.def' and ext_b in {'.asm', '.s', '.S'}:
		return assembler_helper(args.input, args.convert)

	E(f'Unknown conversion scheme: "{ext_a}" => "{ext_b}"')
	return False

def comparator_helper(args: Namespace) -> bool:
	ext_a, ext_b = args.input.suffix, args.compare.suffix

	if {ext_a, ext_b} == {'.sym', '.def'}:
		return compare_definitions(args.input, args.compare, args.skip_addr, args.skip_type, args.swap)
	if ext_a == '.def' and ext_b == '.api':
		return compare_definitions_with_api(args.input, args.compare, args.swap)

	E(f'Unknown comparing scheme: "{ext_a}" => "{ext_b}"')
	return False

def regenerator(args: Namespace) -> bool:
	for suffix in ['.def']:
		for file_path in P2K_SDK_RES.rglob(f'*{suffix}'):
			if args.format_only:
				I(f'Formatting "{file_path}" file.')
				if not format_definitions(file_path, file_path, args.warn_duplicates, args.sort_address):
					return False
			if args.delete_name:
				delete_definition(file_path, args.delete_name)
			# TODO: Compile here?
	return True

def recursive_conversion(args: Namespace) -> bool:
	if not check_dirs([args.recursive_convert], True):
		return False

	for suffix in '.sym':
		for file_path in args.recursive_convert.rglob(f'*{suffix}'):
			output_path = file_path.with_name('ep3.def')
			I(f'Converting "{file_path}" => "{output_path.name}"')
			if not write_definitions(output_path, read_definitions(file_path)):
				return False
	return True

def do_work(args: Namespace) -> bool:
	if args.regenerate:
		return regenerator(args)
	elif args.compare:
		return comparator_helper(args)
	elif args.convert:
		return convertor_helper(args)
	elif args.recursive_convert:
		return recursive_conversion(args)
	# Should be the last branch.
	elif args.input:
		return format_definitions(args.input, args.input, args.warn_duplicates, args.sort_address)
	return True

def args_and_help() -> Namespace:
	class Args(Arguments):
		def check_args(self, args: Namespace) -> None:
			if len(sys.argv) == 1:
				self.print_help(sys.stderr)
				sys.exit(1)

	epl = 'examples:\n\n'
	epl += f'  # Regenerate libraries (or only format def files)\n'
	epl += f'  python {Path(__file__).name} -r\n'
	epl += f'  python {Path(__file__).name} -r -f -w\n'
	epl += f'  python {Path(__file__).name} -r -d some_function_name\n\n'
	epl += f'  # Format, validate, and sort definitions file\n'
	epl += f'  python {Path(__file__).name} -i ep3.sym\n'
	epl += f'  python {Path(__file__).name} -i ep3.sym -a -w\n\n'
	epl += f'  # Convert (conversion scheme is determined by file extensions)\n'
	epl += f'  python {Path(__file__).name} -i ep3.sym -t ep3.def\n'
	epl += f'  python {Path(__file__).name} -i ep3.def -t ep3.sym\n'
	epl += f'  python {Path(__file__).name} -i ep3.asm -t ep3.def -M\n'
	epl += f'  python {Path(__file__).name} -i ep3.def -t ep3.S\n\n'
	epl += f'  # Recursive sym => def conversion in given directory\n'
	epl += f'  python {Path(__file__).name} -l directory\n\n'
	epl += f'  # Compare (skip addresses, skip types, swap input/compared)\n'
	epl += f'  python {Path(__file__).name} -i ep3.def -c ep3.api -S\n'
	epl += f'  python {Path(__file__).name} -i ep3_a.def -c ep3_b.def\n'
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

	parser = Args(description=hlp['D'], epilog=epl, formatter_class=RawDescriptionHelpFormatter)
	parser.add_argument('-r', '--regenerate', action='store_true', help=hlp['r'])
	parser.add_argument('-f', '--format-only', action='store_true', help=hlp['f'])
	parser.add_argument('-i', '--input', type=Path, default=None, help=hlp['i'])
	parser.add_argument('-a', '--sort-address', action='store_true', help=hlp['a'])
	parser.add_argument('-w', '--warn-duplicates', action='store_true', help=hlp['w'])
	parser.add_argument('-t', '--convert', type=Path, default=None, help=hlp['t'])
	parser.add_argument('-M', '--mcore', action='store_true', help=hlp['M'])
	parser.add_argument('-l', '--recursive-convert', type=Path, default=None, help=hlp['l'])
	parser.add_argument('-c', '--compare', type=Path, default=None, help=hlp['c'])
	parser.add_argument('-A', '--skip_addr', action='store_true', help=hlp['A'])
	parser.add_argument('-T', '--skip_type', action='store_true', help=hlp['T'])
	parser.add_argument('-S', '--swap', action='store_true', help=hlp['S'])
	parser.add_argument('-d', '--delete-name', type=str, default=None, help=hlp['d'])
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
