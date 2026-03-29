#!/usr/bin/env python3

"""
Aleph Application Skeleton Template

A general template for creating Python tools using Aleph library.
"""

import sys
from pathlib import Path
from datetime import datetime
from argparse import Namespace
from argparse import RawDescriptionHelpFormatter

sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent))

from aleph import *

def do_work(args: Namespace) -> bool:
	if args.verbose:
		I('Verbose mode enabled {check}.')
	return True

def args_and_help() -> Namespace:
	class Args(Arguments):
		def check_args(self, args: Namespace) -> None:
			pass

	epl = 'examples:\n'
	epl += f'  python {Path(__file__).name} -v\n'
	hlp = {
		'D': 'Aleph Application Skeleton Template, 01-Mar-2026',
		'v': 'Enable verbose output'
	}

	parser = Args(description=hlp['D'], epilog=epl, formatter_class=RawDescriptionHelpFormatter)
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
