#!/usr/bin/env python3

"""
EP3 Build Tool

A general tool for building ELF Loader, BIN Loader, and libraries for a selected platform.
"""

import sys

from datetime import datetime

from argparse import Namespace
from argparse import ArgumentParser
from argparse import RawDescriptionHelpFormatter

from aleph import D
from aleph import I
from aleph import E
from aleph import Recipe
from aleph import compile_c
from aleph import set_logger
from aleph import P2K_SDK_SRC
from aleph import P2K_SDK_BUILD
from aleph import elapsed_format
from aleph import create_clean_dir

from config import RECIPES

def build_bin_ldr(recipe: Recipe) -> bool:
	build_files = [
		(P2K_SDK_SRC / 'P2K_EP3_BIN_Loader.c', P2K_SDK_BUILD / 'P2K_EP3_BIN_Loader.o'),
	]

	for c_src, c_out in build_files:
		if not compile_c(recipe, c_src, c_out):
			return False

	return True

def build_elf_ldr(recipe: Recipe) -> bool:
	pass

def build_so_lib(recipe: Recipe) -> bool:
	pass

def build_patches(recipe: Recipe) -> bool:
	pass

def cook_recipe(recipe_name: str, recipe: Recipe) -> bool:
	I(f'Cooking {recipe_name} recipe.')

	build_steps = [
		(create_clean_dir, [P2K_SDK_BUILD], 'Cannot create build directory'),
		(build_bin_ldr, [recipe], 'Cannot build BIN Loader'),
		(build_elf_ldr, [recipe], 'Cannot build ELF Loader'),
		(build_so_lib, [recipe], 'Cannot build SO Library'),
		(build_patches, [recipe], 'Cannot build Final Patches'),
	]

	for func, args, error_msg in build_steps:
		if not func(*args):
			E(f'{error_msg} for "{recipe_name}".')
			return False

	return True

def do_work(args: Namespace) -> bool:
	if args.recipe == 'all':
		I('Cooking all recipes.')
		for recipe in RECIPES.keys():
			if not cook_recipe(args.recipe, RECIPES[recipe]):
				return False
		return True
	return cook_recipe(args.recipe, RECIPES[args.recipe])

def args_and_help() -> Namespace:
	class Args(ArgumentParser):
		def error(self, message: str) -> None:
			self.print_help(sys.stderr)
			self.exit(2, f'\n{self.prog}: error: {message}\n')
		def parse_and_check_args(self) -> Namespace:
			args = self.parse_args()
			if (args.recipe not in RECIPES) and (args.recipe != 'all'):
				self.error(f'Recipe "{args.recipe}" not supported!')
			return args

	epl = 'examples:\n'
	epl += '  python build.py all\n'
	for recipe in RECIPES.keys():
		epl += f'  python build.py {recipe}\n'
	hlp = {
		'D': 'EP3 Build Tool for Motorola phones on P2K platform, 01-Mar-2026',
		'R': 'Phone and firmware recipe to build',
		'v': 'Enable verbose output'
	}

	parser = Args(description=hlp['D'], epilog=epl, formatter_class=RawDescriptionHelpFormatter)
	parser.add_argument('recipe', type=str, help=hlp['R'])
	parser.add_argument('-v', '--verbose', action='store_true', help=hlp['v'])
	return parser.parse_and_check_args()

def main() -> None:
	args = args_and_help()
	set_logger(args.verbose)

	start_time = datetime.now()
	D(f'Start building on "{start_time}".')

	do_work(args)

	D(f'End building on "{datetime.now()}".')
	I(f'Time elapsed: "{elapsed_format(datetime.now() - start_time)}".')

if __name__ == '__main__':
	main()
