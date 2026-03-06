#!/usr/bin/env python3

import sys
from pathlib import Path
from datetime import datetime
from argparse import Namespace
from argparse import RawDescriptionHelpFormatter

from aleph import *

def convertor_helper(args: Namespace) -> bool:
	ext_a, ext_b = args.input.suffix, args.convert.suffix

	if {ext_a, ext_b} == {'.sym', '.def'}:
		return write_definitions(args.convert, read_definitions(args.input))
	if ext_a in  {'.asm', '.s', '.S'} and ext_b == '.def':
		return convert_asm_to_def(args.input, args.convert, args.mcore)
	if ext_a == '.def' and ext_b in {'.asm', '.s', '.S'}:
		return True

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
	if args.compare:
		return comparator_helper(args)
	elif args.convert:
		return convertor_helper(args)
	elif args.recursive_convert:
		return recursive_conversion(args)
	return True

def args_and_help() -> Namespace:
	class Args(Arguments):
		def check_args(self, args: Namespace) -> None:
			pass

	epl = 'examples:\n\n'
	epl += f'  # Convert (conversion scheme is determined by file extensions)\n'
	epl += f'  python {Path(__file__).name} -i ep3_1.sym -t ep3_2.def\n'
	epl += f'  python {Path(__file__).name} -i ep3_1.def -t ep3_2.sym\n'
	epl += f'  python {Path(__file__).name} -i ep3_1.asm -t ep3_2.def\n'
	epl += f'  python {Path(__file__).name} -i ep3_1.asm -t ep3_2.def -M\n'
	epl += f'  python {Path(__file__).name} -i ep3_1.def -t ep3_2.asm\n\n'
	epl += f'  # Recursive sym => def conversion in given directory\n'
	epl += f'  python {Path(__file__).name} -l sym_directory\n\n'
	epl += f'  # Compare (skip addresses, skip types, swap input/compared)\n'
	epl += f'  python {Path(__file__).name} -i ep3_1.def -c ep3_2.def\n'
	epl += f'  python {Path(__file__).name} -i ep3_1.def -c ep3_2.api\n'
	epl += f'  python {Path(__file__).name} -i ep3_1.def -c ep3_2.api -S\n'
	epl += f'  python {Path(__file__).name} -i ep3_1.def -c ep3_2.def -A\n'
	epl += f'  python {Path(__file__).name} -i ep3_1.def -c ep3_2.def -A -T\n'
	epl += f'  python {Path(__file__).name} -i ep3_1.def -c ep3_2.def -A -T -S\n\n'
	hlp = {
		'D': 'EP3 Library Generation Tool for Motorola phones on P2K platform, 01-Mar-2026',
		'i': 'input file',
		't': 'convert input file to output file',
		'M': 'M-CORE assembler',
		'l': 'recursive sym => def conversion in directory',
		'c': 'compare file with input file',
		'A': 'skip addresses during comparison',
		'T': 'skip types during comparison',
		'S': 'swap input and compared files',
		'v': 'enable verbose output'
	}

	## TODO:
	# def => asm, compile a, so
	# def => recursive compile, bump date
	# def => convert to asm
	# so, elf => def or api listing

	parser = Args(description=hlp['D'], epilog=epl, formatter_class=RawDescriptionHelpFormatter)
	parser.add_argument('-i', '--input', type=Path, default=None, help=hlp['i'])
	parser.add_argument('-t', '--convert', type=Path, default=None, help=hlp['t'])
	parser.add_argument('-M', '--mcore', action='store_true', help=hlp['M'])
	parser.add_argument('-l', '--recursive-convert', type=Path, default=None, help=hlp['l'])
	parser.add_argument('-c', '--compare', type=Path, default=None, help=hlp['c'])
	parser.add_argument('-A', '--skip_addr', action='store_true', help=hlp['A'])
	parser.add_argument('-T', '--skip_type', action='store_true', help=hlp['T'])
	parser.add_argument('-S', '--swap', action='store_true', help=hlp['S'])
	parser.add_argument('-v', '--verbose', action='store_true', help=hlp['v'])

	if len(sys.argv) == 1:
		parser.print_help(sys.stderr)
		sys.exit(1)

	return parser.parse_and_check_args()

def main() -> None:
	args = args_and_help()
	set_logger(args.verbose)

	start_time = datetime.now()

	do_work(args)

	I(f'Time elapsed: "{elapsed_format(datetime.now() - start_time)}".')

if __name__ == '__main__':
	main()
