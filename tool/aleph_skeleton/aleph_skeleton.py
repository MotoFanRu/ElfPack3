#!/usr/bin/env python3

"""
Aleph Application Skeleton Template

A general template for creating Python tools using Aleph framework library.
"""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent))

from aleph import Aleph

def do_skeleton_work(a: Aleph) -> bool:
	a.log.I(f'Working... ')
	return True

def full_skeleton_args(a: Aleph) -> None:
	epl = 'examples:\n'
	epl += f'  python {Path(__file__).name} -v\n'
	hlp = {
		'D': 'Aleph Application Skeleton Template, 01-Mar-2026',
		'v': 'Enable verbose output'
	}

	a.args_parser.add_epilog_description(epl, hlp['D'])
	a.args_parser.add_argument('-v', '--verbose', action='store_true', help=hlp['v'])

def check_skeleton_args(a: Aleph) -> None:
	if a.args.verbose:
		a.log.I(f'Verbose mode: {a.args.verbose}')

if __name__ == '__main__':
	aleph = Aleph()
	aleph.set_args(full_skeleton_args, check_skeleton_args)
	aleph.main(do_skeleton_work)
