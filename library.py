#!/usr/bin/env python3

import sys

from pathlib import Path

from datetime import datetime

from argparse import Namespace
from argparse import RawDescriptionHelpFormatter

from aleph.definitions import write_definitions
from aleph.filesystem import check_dirs

sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent))

from aleph import *

def convert_helper(p_in: Path, p_out: Path, mcore_asm: bool = False) -> bool:
	if p_in.suffix == '.sym' and p_out.suffix == '.def':
		return write_definitions(p_out, read_definitions(p_in))
	elif p_in.suffix == '.def' and p_out.suffix == '.sym':
		return write_definitions(p_out, read_definitions(p_in))
	elif p_in.suffix in ['.asm', '.s', '.S'] and p_out.suffix == '.def':
		return convert_asm_to_def(p_in, p_out, mcore_asm)
	elif p_in.suffix == '.def' and p_out.suffix in ['.asm', '.s', '.S']:
		pass
	else:
		E(f'Unknown conversion scheme: "{p_in.suffix}" => "{p_out.suffix}"')
		return False

def recursive_conversion(p_dir: Path) -> bool:
	if not check_dirs([p_dir], True):
		return False

	for suffix in '.sym':
		for file_path in p_dir.rglob(f'*{suffix}'):
			output_path = file_path.with_name('ep3.def')
			I(f'Converting "{file_path.name}" => "{output_path.name}"')
			if not convert_helper(file_path, output_path):
				return False
	return True

def do_work(args: Namespace) -> bool:
	if args.compare:
		return compare_definitions(args.input, args.compare, args.skip_addr, args.skip_type, args.swap)
	elif args.api_compare:
		pass
	elif args.convert:
		return convert_helper(args.input, args.convert, args.mcore)
	elif args.recursive_convert:
		return recursive_conversion(args.recursive_convert)
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
	epl += f'  python {Path(__file__).name} -i ep3_1.def -c ep3_2.def -A\n'
	epl += f'  python {Path(__file__).name} -i ep3_1.def -c ep3_2.def -A -T\n'
	epl += f'  python {Path(__file__).name} -i ep3_1.def -c ep3_2.def -A -T -S\n\n'
	epl += f'  # Compare definitions with API list file\n'
	epl += f'  python {Path(__file__).name} -i ep3_1.def -a list.api\n'
	hlp = {
		'D': 'EP3 Library Generation Tool for Motorola phones on P2K platform, 01-Mar-2026',
		'i': 'input file',
		't': 'convert input file to output file',
		'M': 'M-CORE assembler',
		'l': 'recursive sym => def conversion in directory',
		'c': 'compare file with input file',
		'a': 'compare file with API list file',
		'A': 'skip addresses during comparison',
		'T': 'skip types during comparison',
		'S': 'swap input and compared files',
		'v': 'enable verbose output'
	}

	## TODO:
	# def => asm, compile, so
	# def => recursive compile, bump date
	# def => convert to asm
	# def => compare api = chunk (found)
	# def => compare api = reverse chunk (not found)
	# so, elf => def or api listing

	parser = Args(description=hlp['D'], epilog=epl, formatter_class=RawDescriptionHelpFormatter)
	parser.add_argument('-i', '--input', type=Path, default=None, help=hlp['i'])
	parser.add_argument('-t', '--convert', type=Path, default=None, help=hlp['t'])
	parser.add_argument('-M', '--mcore', action='store_true', help=hlp['M'])
	parser.add_argument('-l', '--recursive-convert', type=Path, default=None, help=hlp['l'])
	parser.add_argument('-c', '--compare', type=Path, default=None, help=hlp['c'])
	parser.add_argument('-a', '--api-compare', type=Path, default=None, help=hlp['a'])
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
